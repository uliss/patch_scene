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
#include "comment.h"
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

SharedDeviceData make_dev(SceneItemId id, const QPointF& pos)
{
    auto data = new DeviceData(id);
    data->setTitle("abc");
    data->setPos(pos);
    return SharedDeviceData(data);
}

SharedDeviceData data1(SceneItemId id)
{
    auto data = new DeviceData(id);
    data->setShowTitle(false);
    return SharedDeviceData { data };
}

} // namespace

void TestDiagram::addDevice()
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

void TestDiagram::removeDevice()
{
    Diagram dia(100, 100);

    dia.itemScene().add(make_dev(100, { 50, 100 }));
    QCOMPARE(dia.itemScene().count(), 1);
    dia.itemScene().add(make_dev(101, { 100, 200 }));
    QCOMPARE(dia.itemScene().count(), 2);

    dia.cmdRemoveDevice(dia.itemScene().findData(100));
    QCOMPARE(dia.itemScene().count(), 1);
    dia.cmdRemoveDevice(dia.itemScene().findData(101));
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

    dia.cmdMoveSelectedDevicesBy(10, 20);
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

    dia.cmdMoveSelectedDevicesFrom({ 0, 0 }, { 100, 50 });
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

void TestDiagram::addComment()
{
    Diagram dia(100, 100);
    QSignalSpy sig_spy(&dia, SIGNAL(sceneChanged()));
    QVERIFY(sig_spy.isValid());
    QCOMPARE(sig_spy.count(), 0);

    auto json0 = dia.toJson();

    auto comm = dia.addComment();
    QVERIFY(comm != nullptr);
    QCOMPARE(comm->deviceData()->category(), ceam::ItemCategory::Comment);
    QCOMPARE(sig_spy.count(), 1);
}

void TestDiagram::duplicateSelected()
{
    Diagram dia(100, 100);
    auto& devs = dia.itemScene();
    QCOMPARE(devs.selectedCount(), 0);
    size_t num = 0;

    num = dia.duplicateSelected({ true, true }).count();
    QCOMPARE(devs.selectedCount(), 0);

    auto dev1 = dia.addDevice(data1(100));
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
