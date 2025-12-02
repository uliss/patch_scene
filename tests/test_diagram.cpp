/*****************************************************************************
 * Copyright 2024 Serge Poltavski. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also find the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#include "test_diagram.h"
#include "diagram.h"

#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

using namespace ceam;

namespace {

#ifdef Q_OS_LINUX
constexpr int DEF_TXT_HT = 26;
#else
constexpr int DEF_TXT_HT = 24;
#endif

SharedItemData make_dev(SceneItemId id, const QPointF& pos)
{
    auto data = new ItemData(id);
    data->setTitle("abc");
    data->setPos(pos);
    return SharedItemData(data);
}

SharedItemData data1(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setShowTitle(false);
    return SharedItemData { data };
}

} // namespace

void TestDiagram::addItem()
{
    Diagram dia(100, 100);
    QSignalSpy sig_spy(&dia, SIGNAL(sceneChanged()));
    QVERIFY(sig_spy.isValid());
    QCOMPARE(sig_spy.count(), 0);

    auto json0 = dia.toJson();

    dia.cmdCreateDevice({ 50, 100 });
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(sig_spy.count(), 1);
    dia.cmdCreateDevice({ 50, 200 });
    QCOMPARE(dia.itemScene().count(), 2);
    QCOMPARE(sig_spy.count(), 2);

    auto json1 = dia.toJson();

    dia.undo();
    QCOMPARE(dia.itemScene().count(), 1);
    dia.undo();
    QCOMPARE(dia.itemScene().count(), 0);

    auto json2 = dia.toJson();
    QCOMPARE(json0, json2);

    dia.redo();
    QCOMPARE(dia.itemScene().count(), 1);
    dia.redo();
    QCOMPARE(dia.itemScene().count(), 2);

    auto json3 = dia.toJson();
    QCOMPARE(json1, json3);
}

void TestDiagram::removeItem()
{
    Diagram dia(100, 100);

    dia.itemScene().add(make_dev(100, { 50, 100 }));
    QCOMPARE(dia.itemScene().count(), 1);
    dia.itemScene().add(make_dev(101, { 100, 200 }));
    QCOMPARE(dia.itemScene().count(), 2);

    dia.cmdRemoveItem(dia.itemScene().findData(100));
    QCOMPARE(dia.itemScene().count(), 1);
    dia.cmdRemoveItem(dia.itemScene().findData(101));
    QCOMPARE(dia.itemScene().count(), 0);

    dia.undo();
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(100, 200));
    dia.undo();
    QCOMPARE(dia.itemScene().count(), 2);
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(100, 200));

    dia.redo();
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(100, 200));

    dia.redo();
    QCOMPARE(dia.itemScene().count(), 0);
}

void TestDiagram::moveSelectedBy()
{
    Diagram dia(100, 100);
    dia.itemScene().add(make_dev(100, { 50, 100 }));
    dia.itemScene().setSelected({ 100 }, true);

    dia.cmdMoveSelectedItemsBy(10, 20);
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(60, 120));

    dia.undo();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

    dia.redo();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(60, 120));
}

void TestDiagram::moveSelectedFrom()
{
    Diagram dia(100, 100);
    dia.itemScene().add(make_dev(100, { 50, 100 }));
    dia.itemScene().setSelected({ 100 }, true);

    dia.cmdMoveSelectedItemsFrom({ 0, 0 }, { 100, 50 });
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(150, 150));

    dia.undo();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

    dia.redo();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(150, 150));
}

void TestDiagram::cmdPlaceInRowSelected()
{
    Diagram dia(100, 100);

    // 1 devices
    dia.itemScene().add(make_dev(100, { 50, 100 }));
    QCOMPARE(dia.itemScene().setSelected({ 100 }, true), 1);
    dia.cmdPlaceInRowSelected();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

    // 2 devices
    dia.itemScene().add(make_dev(101, { 1050, 500 }));
    QCOMPARE(dia.itemScene().setSelected({ 100, 101 }, true), 2);
    dia.cmdPlaceInRowSelected();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(130, 100));

    // 3 devices
    dia.itemScene().add(make_dev(102, { 10, 25 }));
    QCOMPARE(dia.itemScene().setSelected({ 100, 101, 102 }, true), 3);
    dia.cmdPlaceInRowSelected();

    QCOMPARE(dia.itemScene().findData(102)->pos(), QPointF(10, 25));
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(90, 25));
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(170, 25));
}

void TestDiagram::cmdPlaceInColumnSelected()
{
    Diagram dia(100, 100);

    // 1 devices
    dia.itemScene().add(make_dev(100, { 50, 100 }));
    QCOMPARE(dia.itemScene().setSelected({ 100 }, true), 1);
    dia.cmdPlaceInColumnSelected();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

    // 2 devices
    dia.itemScene().add(make_dev(101, { 1050, 500 }));
    QCOMPARE(dia.itemScene().setSelected({ 100, 101 }, true), 2);
    dia.cmdPlaceInColumnSelected();
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(50, 100 + DEF_TXT_HT));

    // 3 devices
    dia.itemScene().add(make_dev(102, { 10, 25 }));
    QCOMPARE(dia.itemScene().setSelected({ 100, 101, 102 }, true), 3);
    dia.cmdPlaceInColumnSelected();

    QCOMPARE(dia.itemScene().findData(102)->pos(), QPointF(10, 25));
    QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(10, 25 + DEF_TXT_HT));
    QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(10, 25 + 2 * DEF_TXT_HT));
}

void TestDiagram::duplicateSelected()
{
    Diagram dia(100, 100);
    auto& devs = dia.itemScene();
    QCOMPARE(devs.selectedCount(), 0);
    size_t num = 0;

    num = dia.duplicateSelected({ true, true }).count();
    QCOMPARE(devs.selectedCount(), 0);

    auto dev1 = dia.addItem(data1(100));
    QCOMPARE(devs.count(), 1);
    QCOMPARE(devs.selectedCount(), 0);
    devs.setSelected({ dev1->id() }, true);
    QCOMPARE(devs.selectedCount(), 1);

    dia.duplicateSelected({ true, true });
    QCOMPARE(devs.count(), 2);
    QCOMPARE(devs.selectedCount(), 1);

    dia.duplicateSelected({ true, true });
    QCOMPARE(devs.count(), 3);
    QCOMPARE(devs.selectedCount(), 1);

    num = dia.duplicateSelected({ true, false }).count();
    QCOMPARE(num, 1);
    QCOMPARE(devs.count(), 4);
    QCOMPARE(devs.selectedCount(), 2);
}

void TestDiagram::mouseMove()
{
    Diagram dia(100, 100);
    dia.show();

    dia.cmdCreateDevice({ 10, 10 });
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(dia.itemScene().selectedCount(), 0);

    // +200, +150
    QPoint pt { 212, 162 };

    // click select item
    QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, pt);
    QCOMPARE(dia.state(), DiagramState::SelectItem);
    QCOMPARE(dia.itemScene().selectedCount(), 1);
    QVERIFY(!dia.isSelectionRectVisible());

    // move selected item
    QTest::mouseMove(dia.viewport(), pt + QPoint { 20, 20 });
    QCOMPARE(dia.state(), DiagramState::MoveItem);
    QCOMPARE(dia.itemScene().selectedCount(), 1);

    // finish moving
    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, pt + QPoint { 20, 20 });
    QCOMPARE(dia.state(), DiagramState::Init);

    // dia.selected

    auto selected = dia.itemScene().selectedIdList();
    QCOMPARE(selected.count(), 1);
    auto item = dia.itemScene().find(selected.front());
    QVERIFY(item);
    QCOMPARE(item->pos(), QPointF(30, 30));

    dia.undo();
    QCOMPARE(item->pos(), QPointF(10, 10));
}

void TestDiagram::mousePress()
{
    Diagram dia(100, 100);
    QCOMPARE(dia.state(), DiagramState::Init);
    dia.show();

    QTest::mousePress(dia.viewport(), Qt::LeftButton);
    QCOMPARE(dia.state(), DiagramState::SelectionRect);

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton);
    QCOMPARE(dia.state(), DiagramState::Init);

    QTest::mousePress(dia.viewport(), Qt::LeftButton);
    QCOMPARE(dia.state(), DiagramState::SelectionRect);

    QTest::mouseMove(dia.viewport(), { 10, 10 });
    QCOMPARE(dia.state(), DiagramState::SelectionRect);

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 10, 10 });
    QCOMPARE(dia.state(), DiagramState::Init);

    dia.cmdCreateDevice({ 10, 10 });
    QCOMPARE(dia.itemScene().count(), 1);
    QCOMPARE(dia.itemScene().selectedCount(), 0);

    // +200, +150
    QPoint pt { 212, 162 };

    // click select item
    QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, pt);
    QCOMPARE(dia.state(), DiagramState::SelectItem);
    QCOMPARE(dia.itemScene().selectedCount(), 1);

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, pt);
    QCOMPARE(dia.state(), DiagramState::Init);
    QCOMPARE(dia.itemScene().selectedCount(), 1);

    // click+ctl toggle selected item
    QTest::mousePress(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
    QCOMPARE(dia.state(), DiagramState::Init);
    QCOMPARE(dia.itemScene().selectedCount(), 0);

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
    QCOMPARE(dia.state(), DiagramState::Init);
    QCOMPARE(dia.itemScene().selectedCount(), 0);

    // click+ctl toggle selected item
    QTest::mousePress(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
    QCOMPARE(dia.state(), DiagramState::SelectItem);
    QCOMPARE(dia.itemScene().selectedCount(), 1);

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
    QCOMPARE(dia.state(), DiagramState::Init);
    QCOMPARE(dia.itemScene().selectedCount(), 1);
}

void TestDiagram::mouseSelect()
{
    Diagram dia(100, 100);
    dia.show();

    dia.cmdCreateDevice({ 10, 10 });
    dia.cmdCreateDevice({ 20, 20 });

    // +200, +150
    // select single
    QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, { 200, 150 });
    QCOMPARE(dia.state(), DiagramState::SelectionRect);
    QVERIFY(dia.isSelectionRectVisible());

    QTest::mouseMove(dia.viewport(), { 215, 165 });
    QCOMPARE(dia.state(), DiagramState::SelectionRect);
    QVERIFY(dia.isSelectionRectVisible());

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 215, 215 });
    QCOMPARE(dia.state(), DiagramState::Init);
    QVERIFY(!dia.isSelectionRectVisible());

    QCOMPARE(dia.itemScene().selectedCount(), 1);

    // select all
    QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, { 200, 150 });
    QCOMPARE(dia.state(), DiagramState::SelectionRect);
    QVERIFY(dia.isSelectionRectVisible());

    QTest::mouseMove(dia.viewport(), { 225, 175 });
    QCOMPARE(dia.state(), DiagramState::SelectionRect);
    QVERIFY(dia.isSelectionRectVisible());

    QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 225, 175 });
    QCOMPARE(dia.state(), DiagramState::Init);
    QVERIFY(!dia.isSelectionRectVisible());

    QCOMPARE(dia.itemScene().selectedCount(), 2);

    dia.undo();
    QCOMPARE(dia.itemScene().selectedCount(), 0);
}
