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
#include "test_diagram_scene.h"
#include "diagram_scene.h"
#include "scene_connections.h"
#include "scene_item.h"

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

SharedItemData make_dev_inout(SceneItemId id, int ins = 0, int outs = 0)
{
    auto data = new ItemData(id);
    data->setTitle("abc");
    while (ins-- > 0)
        data->appendInput({});

    while (outs-- > 0)
        data->appendOutput({});

    return SharedItemData(data);
}

SceneItemId toId(const QSignalSpy& spy, qsizetype n, int x = 0)
{
    if (n >= spy.count())
        return SCENE_ITEM_NULL_ID;

    if (x >= spy.at(n).count())
        return SCENE_ITEM_NULL_ID;

    auto sig_arg = spy.at(n).at(x).value<SharedItemData>();
    if (!sig_arg)
        return SCENE_ITEM_NULL_ID;
    else
        return sig_arg->id();
}

SharedItemData toData(const QSignalSpy& spy, qsizetype n, int x = 0)
{
    if (n >= spy.count())
        return {};

    if (x >= spy.at(n).count())
        return {};

    return spy.at(n).at(x).value<SharedItemData>();
}

SharedItemData clone(const SharedItemData& data)
{
    return SharedItemData(new ItemData(*data.data()));
}

qreal zValue(const DiagramScene& ds, const SharedItemData& data)
{
    return ds.itemScene().find(data->id())->zValue();
}

} // namespace

void TestDiagramScene::cmdCreateDevice()
{
    DiagramScene ds(100, 100);
    QCOMPARE(ds.itemCount(), 0);

    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);
    QSignalSpy sig_dev_added(&ds, &DiagramScene::deviceAdded);
    QSignalSpy sig_dev_removed(&ds, &DiagramScene::deviceRemoved);

    QVERIFY(sig_scene_changed.isValid());
    QCOMPARE(sig_scene_changed.count(), 0);

    ds.cmdCreateDevice({ 50, 100 });
    QCOMPARE(ds.itemCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(sig_dev_added.count(), 1);
    QCOMPARE(sig_dev_removed.count(), 0);

    ds.undo();
    QCOMPARE(ds.itemCount(), 0);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(sig_dev_added.count(), 1);
    QCOMPARE(sig_dev_removed.count(), 1);
    ds.redo();
    QCOMPARE(ds.itemCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 3);
    QCOMPARE(sig_dev_added.count(), 2);
    QCOMPARE(sig_dev_removed.count(), 1);

    ds.cmdCreateDevice({ 50, 200 });
    QCOMPARE(ds.itemCount(), 2);
    QCOMPARE(sig_scene_changed.count(), 4);
    QCOMPARE(sig_dev_added.count(), 3);
    QCOMPARE(sig_dev_removed.count(), 1);
}

void TestDiagramScene::cmdConnectDevices()
{
    DiagramScene ds(100, 100);
    QCOMPARE(ds.itemCount(), 0);
    QCOMPARE(ds.connectionCount(), 0);

    ds.addSceneItem(SharedItemData(new ItemData(100)));
    ds.addSceneItem(SharedItemData(new ItemData(101)));

    QCOMPARE(ds.itemCount(), 2);

    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);
    QSignalSpy sig_conn_added(&ds, &DiagramScene::connectionAdded);
    QSignalSpy sig_conn_removed(&ds, &DiagramScene::connectionRemoved);

    // invalid device id's
    ds.cmdConnectDevices({ 1, 0, 2, 0 });
    QCOMPARE(ds.connectionCount(), 0);
    QCOMPARE(sig_scene_changed.count(), 0);
    QCOMPARE(sig_conn_added.count(), 0);
    QCOMPARE(sig_conn_removed.count(), 0);

    // invalid source/destination
    ds.cmdConnectDevices({ 100, 0, 101, 0 });
    QCOMPARE(ds.connectionCount(), 0);
    QCOMPARE(sig_scene_changed.count(), 0);
    QCOMPARE(sig_conn_added.count(), 0);
    QCOMPARE(sig_conn_removed.count(), 0);

    // ok
    ds.addSceneItem(make_dev_inout(102, 0, 1));
    ds.addSceneItem(make_dev_inout(103, 1, 0));

    ds.cmdConnectDevices({ 102, 0, 103, 0 });
    QCOMPARE(ds.connectionCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 3);
    QCOMPARE(sig_conn_added.count(), 1);
    QCOMPARE(sig_conn_removed.count(), 0);

    ds.undo();
    QCOMPARE(ds.connectionCount(), 0);
    QCOMPARE(sig_scene_changed.count(), 4);
    QCOMPARE(sig_conn_added.count(), 1);
    QCOMPARE(sig_conn_removed.count(), 1);
}

void TestDiagramScene::cmdCreateComment()
{
    DiagramScene ds(100, 100);
    QCOMPARE(ds.itemCount(), 0);

    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);
    QSignalSpy sig_dev_added(&ds, &DiagramScene::deviceAdded);
    QSignalSpy sig_dev_removed(&ds, &DiagramScene::deviceRemoved);

    QVERIFY(sig_scene_changed.isValid());
    QCOMPARE(sig_scene_changed.count(), 0);

    ds.cmdCreateComment({ 50, 100 });
    QCOMPARE(ds.itemCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(sig_dev_added.count(), 0);
    QCOMPARE(sig_dev_removed.count(), 0);

    ds.undo();
    QCOMPARE(ds.itemCount(), 0);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(sig_dev_added.count(), 0);
    QCOMPARE(sig_dev_removed.count(), 1);
    ds.redo();
    QCOMPARE(ds.itemCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 3);
    QCOMPARE(sig_dev_added.count(), 0);
    QCOMPARE(sig_dev_removed.count(), 1);
}

void TestDiagramScene::cmdRemoveItem()
{
    DiagramScene ds(100, 100);

    QSignalSpy sig_added(&ds.itemScene(), &Scene::added);
    QSignalSpy sig_removed(&ds, &DiagramScene::deviceRemoved);
    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);

    ds.cmdCreateDevice({ 100, 100 });
    QCOMPARE(sig_added.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    auto d0 = toData(sig_added, 0, 0);
    QVERIFY(d0);

    ds.cmdCreateDevice({ 200, 200 });
    auto d1 = toData(sig_added, 1, 0);
    QVERIFY(d1);

    ds.cmdCreateComment({ 50, 50 });
    auto d2 = toData(sig_added, 2, 0);
    QVERIFY(d2);
    QCOMPARE(sig_scene_changed.count(), 3);

    QCOMPARE(ds.itemCount(), 3);

    ds.cmdRemoveItem(d2);
    QCOMPARE(ds.itemCount(), 2);
    QCOMPARE(sig_removed.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 4);

    ds.undo();
    QCOMPARE(ds.itemCount(), 3);
    QCOMPARE(sig_removed.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 5);

    ds.cmdConnectDevices({ d0->id(), 0, d1->id(), 0 });
    QCOMPARE(ds.connectionCount(), 1);
    QCOMPARE(sig_scene_changed.count(), 6);

    ds.cmdRemoveItem(d0);
    QCOMPARE(ds.itemCount(), 2);
    QCOMPARE(sig_removed.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 7);
    QCOMPARE(ds.connectionCount(), 0);

    ds.undo();
    QCOMPARE(ds.itemCount(), 3);
    QCOMPARE(sig_removed.count(), 2);
    QCOMPARE(ds.connectionCount(), 1);
    // on device add and connection add
    QCOMPARE(sig_scene_changed.count(), 9);
}

void TestDiagramScene::cmdDuplicateItem()
{
    DiagramScene ds(100, 100);

    QSignalSpy sig_added(&ds.itemScene(), &Scene::added);
    QSignalSpy sig_removed(&ds, &DiagramScene::deviceRemoved);
    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);

    ds.cmdCreateDevice({ 100, 100 });
    QCOMPARE(sig_added.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(ds.itemCount(), 1);
    auto d0 = toData(sig_added, 0, 0);
    QVERIFY(d0);

    ds.cmdCreateComment({ 50, 50 });
    QCOMPARE(sig_added.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(ds.itemCount(), 2);
    auto d1 = toData(sig_added, 1, 0);
    QVERIFY(d1);

    ds.cmdDuplicateItem(d0);
    QCOMPARE(sig_added.count(), 3);
    QCOMPARE(sig_scene_changed.count(), 3);
    QCOMPARE(ds.itemCount(), 3);
    auto d0_copy = toData(sig_added, 2, 0);
    QVERIFY(d0_copy);

    ds.cmdDuplicateItem(d1);
    QCOMPARE(sig_added.count(), 4);
    QCOMPARE(sig_scene_changed.count(), 4);
    QCOMPARE(ds.itemCount(), 4);
    auto d1_copy = toData(sig_added, 3, 0);
    QVERIFY(d1_copy);

    QCOMPARE(*d0.get(), *d0_copy.get());
    QCOMPARE(*d1.get(), *d1_copy.get());

    ds.undo();
    QCOMPARE(sig_added.count(), 4);
    QCOMPARE(sig_removed.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 5);
    QCOMPARE(ds.itemCount(), 3);

    ds.undo();
    QCOMPARE(sig_added.count(), 4);
    QCOMPARE(sig_removed.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 6);
    QCOMPARE(ds.itemCount(), 2);
}

void TestDiagramScene::cmdUpdateItem()
{
    DiagramScene ds(100, 100);

    QSignalSpy sig_added(&ds.itemScene(), &Scene::added);
    QSignalSpy sig_updated(&ds, &DiagramScene::deviceUpdated);
    QSignalSpy sig_title_updated(&ds, &DiagramScene::deviceTitleUpdated);
    QSignalSpy sig_bat_changed(&ds, &DiagramScene::batteryChanged);
    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);

    ds.cmdCreateDevice({ 100, 100 });
    QCOMPARE(sig_added.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(ds.itemCount(), 1);
    auto d0 = toData(sig_added, 0, 0);
    QVERIFY(d0);

    auto d1 = clone(d0);
    QCOMPARE(d0->id(), d1->id());
    d1->setVendor("TEST VENDOR");
    ds.cmdUpdateItem(d1);
    QCOMPARE(sig_updated.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(sig_bat_changed.count(), 0);
    QCOMPARE(sig_title_updated.count(), 0);

    d1->setTitle("TEST TITLE");
    ds.cmdUpdateItem(d1);
    QCOMPARE(sig_updated.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 3);
    QCOMPARE(sig_bat_changed.count(), 0);
    QCOMPARE(sig_title_updated.count(), 1);

    d1->setBatteryCount(4);
    ds.cmdUpdateItem(d1);
    QCOMPARE(sig_updated.count(), 3);
    QCOMPARE(sig_scene_changed.count(), 4);
    QCOMPARE(sig_bat_changed.count(), 1);
    QCOMPARE(sig_title_updated.count(), 1);
    auto d1c = ds.itemScene().findData(d1->id());
    QCOMPARE(d1c->batteryCount(), 4);

    ds.undo();
    QCOMPARE(sig_updated.count(), 4);
    QCOMPARE(sig_scene_changed.count(), 5);
    QCOMPARE(sig_bat_changed.count(), 2);
    QCOMPARE(sig_title_updated.count(), 1);

    d1c = ds.itemScene().findData(d1->id());
    QCOMPARE(d1c->batteryCount(), 0);
    QCOMPARE(d1c->title(), "TEST TITLE");
    QCOMPARE(d1c->vendor(), "TEST VENDOR");

    ds.undo();
    QCOMPARE(sig_updated.count(), 5);
    QCOMPARE(sig_scene_changed.count(), 6);
    QCOMPARE(sig_bat_changed.count(), 2);
    QCOMPARE(sig_title_updated.count(), 2);

    d1c = ds.itemScene().findData(d1->id());
    QCOMPARE(d1c->batteryCount(), 0);
    QCOMPARE(d1c->title(), "Device");
    QCOMPARE(d1c->vendor(), "TEST VENDOR");

    ds.undo();
    QCOMPARE(sig_updated.count(), 6);
    QCOMPARE(sig_scene_changed.count(), 7);
    QCOMPARE(sig_bat_changed.count(), 2);
    QCOMPARE(sig_title_updated.count(), 2);

    d1c = ds.itemScene().findData(d1->id());
    QCOMPARE(d1c->batteryCount(), 0);
    QCOMPARE(d1c->title(), "Device");
    QCOMPARE(d1c->vendor(), "");
}

void TestDiagramScene::cmdMoveLower()
{
    DiagramScene ds(100, 100);

    QSignalSpy sig_added(&ds.itemScene(), &Scene::added);
    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);

    ds.cmdCreateDevice({ 100, 100 });
    QCOMPARE(ds.undoStack()->count(), 1);
    QCOMPARE(sig_added.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(ds.itemCount(), 1);
    auto d0 = toData(sig_added, 0, 0);
    QVERIFY(d0);
    auto z0 = zValue(ds, d0);

    ds.cmdMoveLower(d0);
    QCOMPARE(ds.undoStack()->count(), 1);
    QCOMPARE(zValue(ds, d0), z0);

    ds.cmdCreateDevice({ 110, 110 });
    QCOMPARE(sig_added.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(ds.itemCount(), 2);
    auto d1 = toData(sig_added, 1, 0);
    QVERIFY(d1);
    auto z1 = zValue(ds, d1);

    ds.cmdMoveLower(d1);
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z0 > z1);

    ds.cmdMoveLower(d0);
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z1 > z0);

    ds.undo();
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z1 < z0);
}

void TestDiagramScene::cmdMoveUpper()
{
    DiagramScene ds(100, 100);

    QSignalSpy sig_added(&ds.itemScene(), &Scene::added);
    QSignalSpy sig_scene_changed(&ds, &DiagramScene::sceneChanged);

    ds.cmdCreateDevice({ 100, 100 });
    QCOMPARE(ds.undoStack()->count(), 1);
    QCOMPARE(sig_added.count(), 1);
    QCOMPARE(sig_scene_changed.count(), 1);
    QCOMPARE(ds.itemCount(), 1);
    auto d0 = toData(sig_added, 0, 0);
    QVERIFY(d0);
    auto z0 = zValue(ds, d0);

    ds.cmdMoveUpper(d0);
    QCOMPARE(ds.undoStack()->count(), 1);
    QCOMPARE(zValue(ds, d0), z0);

    ds.cmdCreateDevice({ 110, 110 });
    QCOMPARE(sig_added.count(), 2);
    QCOMPARE(sig_scene_changed.count(), 2);
    QCOMPARE(ds.itemCount(), 2);
    auto d1 = toData(sig_added, 1, 0);
    QVERIFY(d1);
    auto z1 = zValue(ds, d1);

    ds.cmdMoveUpper(d1);
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z0 < z1);

    ds.cmdMoveUpper(d0);
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z0 > z1);

    ds.undo();
    z0 = zValue(ds, d0);
    z1 = zValue(ds, d1);
    QVERIFY(z0 < z1);
}

// void TestDiagramScene::moveSelectedBy()
// {
//     Diagram dia(100, 100);
//     dia.itemScene().add(make_dev(100, { 50, 100 }));
//     dia.itemScene().setSelected({ 100 }, true);

//     dia.cmdMoveSelectedItemsBy(10, 20);
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(60, 120));

//     dia.undo();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

//     dia.redo();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(60, 120));
// }

// void TestDiagramScene::moveSelectedFrom()
// {
//     Diagram dia(100, 100);
//     dia.itemScene().add(make_dev(100, { 50, 100 }));
//     dia.itemScene().setSelected({ 100 }, true);

//     dia.cmdMoveSelectedItemsFrom({ 0, 0 }, { 100, 50 });
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(150, 150));

//     dia.undo();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

//     dia.redo();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(150, 150));
// }

// void TestDiagramScene::cmdPlaceInRowSelected()
// {
//     Diagram dia(100, 100);

//     // 1 devices
//     dia.itemScene().add(make_dev(100, { 50, 100 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100 }, true), 1);
//     dia.cmdPlaceInRowSelected();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

//     // 2 devices
//     dia.itemScene().add(make_dev(101, { 1050, 500 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100, 101 }, true), 2);
//     dia.cmdPlaceInRowSelected();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));
//     QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(130, 100));

//     // 3 devices
//     dia.itemScene().add(make_dev(102, { 10, 25 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100, 101, 102 }, true), 3);
//     dia.cmdPlaceInRowSelected();

//     QCOMPARE(dia.itemScene().findData(102)->pos(), QPointF(10, 25));
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(90, 25));
//     QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(170, 25));
// }

// void TestDiagramScene::cmdPlaceInColumnSelected()
// {
//     Diagram dia(100, 100);

//     // 1 devices
//     dia.itemScene().add(make_dev(100, { 50, 100 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100 }, true), 1);
//     dia.cmdPlaceInColumnSelected();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));

//     // 2 devices
//     dia.itemScene().add(make_dev(101, { 1050, 500 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100, 101 }, true), 2);
//     dia.cmdPlaceInColumnSelected();
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(50, 100));
//     QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(50, 100 + DEF_TXT_HT));

//     // 3 devices
//     dia.itemScene().add(make_dev(102, { 10, 25 }));
//     QCOMPARE(dia.itemScene().setSelected({ 100, 101, 102 }, true), 3);
//     dia.cmdPlaceInColumnSelected();

//     QCOMPARE(dia.itemScene().findData(102)->pos(), QPointF(10, 25));
//     QCOMPARE(dia.itemScene().findData(100)->pos(), QPointF(10, 25 + DEF_TXT_HT));
//     QCOMPARE(dia.itemScene().findData(101)->pos(), QPointF(10, 25 + 2 * DEF_TXT_HT));
// }

// void TestDiagramScene::duplicateSelected()
// {
//     Diagram dia(100, 100);
//     auto& devs = dia.itemScene();
//     QCOMPARE(devs.selectedCount(), 0);
//     size_t num = 0;

//     num = dia.duplicateSelected({ true, true }).count();
//     QCOMPARE(devs.selectedCount(), 0);

//     auto dev1 = dia.addItem(data1(100));
//     QCOMPARE(devs.count(), 1);
//     QCOMPARE(devs.selectedCount(), 0);
//     devs.setSelected({ dev1->id() }, true);
//     QCOMPARE(devs.selectedCount(), 1);

//     dia.duplicateSelected({ true, true });
//     QCOMPARE(devs.count(), 2);
//     QCOMPARE(devs.selectedCount(), 1);

//     dia.duplicateSelected({ true, true });
//     QCOMPARE(devs.count(), 3);
//     QCOMPARE(devs.selectedCount(), 1);

//     num = dia.duplicateSelected({ true, false }).count();
//     QCOMPARE(num, 1);
//     QCOMPARE(devs.count(), 4);
//     QCOMPARE(devs.selectedCount(), 2);
// }

// void TestDiagramScene::keyPress()
// {
//     Diagram dia(100, 100);
//     dia.show();

//     dia.cmdCreateDevice({ 10, 10 });
//     dia.cmdCreateDevice({ 20, 20 });
//     QCOMPARE(dia.itemScene().count(), 2);

//     dia.cmdSelectAll();

//     // no action
//     QTest::keyClick(&dia, Qt::Key_Backspace);
//     QCOMPARE(dia.itemScene().count(), 2);
//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     // delete selected
//     QTest::keyClick(&dia, Qt::Key_Backspace, Qt::ControlModifier);
//     QCOMPARE(dia.itemScene().count(), 0);
//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     dia.undo();
//     QCOMPARE(dia.itemScene().count(), 2);
//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     auto selected = dia.itemScene().selectedIdList();
//     QCOMPARE(selected.count(), 2);
//     auto p0 = dia.itemScene().find(selected[0])->pos();
//     auto p1 = dia.itemScene().find(selected[1])->pos();

//     QTest::keyClick(&dia, Qt::Key_Right);
//     QCOMPARE(p0 + QPoint(2, 0), dia.itemScene().find(selected[0])->pos());
//     QCOMPARE(p1 + QPoint(2, 0), dia.itemScene().find(selected[1])->pos());

//     dia.undo();
//     dia.redo();

//     QTest::keyClick(&dia, Qt::Key_Right, Qt::ShiftModifier);
//     QCOMPARE(p0 + QPointF(12, 0), dia.itemScene().find(selected[0])->pos());
//     QCOMPARE(p1 + QPointF(12, 0), dia.itemScene().find(selected[1])->pos());

//     dia.undo();
//     dia.redo();

//     QTest::keyClick(&dia, Qt::Key_Left, Qt::ControlModifier);
//     QCOMPARE(p0 - QPointF(38, 0), dia.itemScene().find(selected[0])->pos());
//     QCOMPARE(p1 - QPointF(38, 0), dia.itemScene().find(selected[1])->pos());

//     dia.undo();
//     dia.redo();

//     QTest::keyClick(&dia, Qt::Key_Down, Qt::ShiftModifier);
//     QCOMPARE(p0 + QPointF(-38, 10), dia.itemScene().find(selected[0])->pos());
//     QCOMPARE(p1 + QPointF(-38, 10), dia.itemScene().find(selected[1])->pos());

//     dia.undo();
//     dia.redo();

//     QTest::keyClick(&dia, Qt::Key_Up, Qt::ControlModifier);
//     QCOMPARE(p0 + QPointF(-38, -40), dia.itemScene().find(selected[0])->pos());
//     QCOMPARE(p1 + QPointF(-38, -40), dia.itemScene().find(selected[1])->pos());
// }

// void TestDiagramScene::mouseMoveMultiple()
// {
//     Diagram dia(100, 100);
//     dia.show();

//     dia.cmdCreateDevice({ 10, 10 });
//     dia.cmdCreateDevice({ 20, 20 });
//     QCOMPARE(dia.itemScene().count(), 2);

//     dia.cmdSelectAll();
//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     // +200, +150
//     QPoint pt { 212, 162 };

//     // click select items
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, Qt::ShiftModifier, pt);
//     QCOMPARE(dia.state(), DiagramState::SelectItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     // move selected item
//     QTest::mouseMove(dia.viewport(), pt + QPoint { 20, 20 });
//     QCOMPARE(dia.state(), DiagramState::MoveItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     // finish moving
//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, Qt::ShiftModifier, pt + QPoint { 20, 20 });
//     QCOMPARE(dia.state(), DiagramState::Init);
//     QCOMPARE(dia.itemScene().selectedCount(), 2);
// }

// void TestDiagramScene::mouseMoveSingle()
// {
//     Diagram dia(100, 100);
//     dia.show();

//     dia.cmdCreateDevice({ 10, 10 });
//     QCOMPARE(dia.itemScene().count(), 1);
//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     // +200, +150
//     QPoint pt { 212, 162 };

//     // click select item
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, pt);
//     QCOMPARE(dia.state(), DiagramState::SelectItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     // move selected item
//     QTest::mouseMove(dia.viewport(), pt + QPoint { 20, 20 });
//     QCOMPARE(dia.state(), DiagramState::MoveItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     // finish moving
//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, pt + QPoint { 20, 20 });
//     QCOMPARE(dia.state(), DiagramState::Init);

//     auto selected = dia.itemScene().selectedIdList();
//     QCOMPARE(selected.count(), 1);
//     auto item = dia.itemScene().find(selected.front());
//     QVERIFY(item);
//     QCOMPARE(item->pos(), QPointF(30, 30));

//     dia.undo();
//     QCOMPARE(item->pos(), QPointF(10, 10));
// }

// void TestDiagramScene::mousePress()
// {
//     Diagram dia(100, 100);
//     QCOMPARE(dia.state(), DiagramState::Init);
//     dia.show();

//     QTest::mousePress(dia.viewport(), Qt::LeftButton);
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton);
//     QCOMPARE(dia.state(), DiagramState::Init);

//     QTest::mousePress(dia.viewport(), Qt::LeftButton);
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);

//     QTest::mouseMove(dia.viewport(), { 10, 10 });
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 10, 10 });
//     QCOMPARE(dia.state(), DiagramState::Init);

//     dia.cmdCreateDevice({ 10, 10 });
//     QCOMPARE(dia.itemScene().count(), 1);
//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     // +200, +150
//     QPoint pt { 212, 162 };

//     // click select item
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, pt);
//     QCOMPARE(dia.state(), DiagramState::SelectItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, pt);
//     QCOMPARE(dia.state(), DiagramState::Init);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     // click+ctl toggle selected item
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
//     QCOMPARE(dia.state(), DiagramState::Init);
//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
//     QCOMPARE(dia.state(), DiagramState::Init);
//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     // click+ctl toggle selected item
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
//     QCOMPARE(dia.state(), DiagramState::SelectItem);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, Qt::ControlModifier, pt);
//     QCOMPARE(dia.state(), DiagramState::Init);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);
// }

// void TestDiagramScene::mouseRightClick()
// {
//     Diagram dia(100, 100);
//     dia.show();
// }

// void TestDiagramScene::mouseSelect()
// {
//     Diagram dia(100, 100);
//     dia.show();

//     dia.cmdCreateDevice({ 10, 10 });
//     dia.cmdCreateDevice({ 20, 20 });

//     QCOMPARE(dia.itemScene().selectedCount(), 0);

//     // +200, +150
//     // select single
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, { 200, 150 });
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);

//     QTest::mouseMove(dia.viewport(), { 215, 165 });
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 215, 215 });
//     QCOMPARE(dia.state(), DiagramState::Init);

//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     dia.undo();
//     QCOMPARE(dia.itemScene().selectedCount(), 0);
//     dia.redo();
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     // select all
//     QTest::mousePress(dia.viewport(), Qt::LeftButton, {}, { 200, 150 });
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     QTest::mouseMove(dia.viewport(), { 225, 175 });
//     QCOMPARE(dia.state(), DiagramState::SelectionRect);
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     QTest::mouseRelease(dia.viewport(), Qt::LeftButton, {}, { 225, 175 });
//     QCOMPARE(dia.state(), DiagramState::Init);

//     QCOMPARE(dia.itemScene().selectedCount(), 2);

//     dia.undo();
//     QCOMPARE(dia.itemScene().selectedCount(), 1);

//     dia.undo();
//     QCOMPARE(dia.itemScene().selectedCount(), 0);
// }
