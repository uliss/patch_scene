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
#include <QTest>

using namespace ceam;

namespace {
SharedDeviceData make_dev(DeviceId id, const QPointF& pos)
{
    auto data = new DeviceData(id);
    data->setPos(pos);
    return SharedDeviceData(data);
}

}

void TestDiagram::addDevice()
{
    Diagram dia(100, 100);
    auto json0 = dia.toJson();

    dia.cmdCreateDevice({ 50, 100 });
    QCOMPARE(dia.devices().count(), 1);
    dia.cmdCreateDevice({ 50, 200 });
    QCOMPARE(dia.devices().count(), 2);

    auto json1 = dia.toJson();

    dia.undo();
    QCOMPARE(dia.devices().count(), 1);
    dia.undo();
    QCOMPARE(dia.devices().count(), 0);

    auto json2 = dia.toJson();
    QCOMPARE(json0, json2);

    dia.redo();
    QCOMPARE(dia.devices().count(), 1);
    dia.redo();
    QCOMPARE(dia.devices().count(), 2);

    auto json3 = dia.toJson();
    QCOMPARE(json1, json3);
}

void TestDiagram::removeDevice()
{
    Diagram dia(100, 100);

    dia.devices().add(make_dev(100, { 50, 100 }));
    QCOMPARE(dia.devices().count(), 1);
    dia.devices().add(make_dev(101, { 100, 200 }));
    QCOMPARE(dia.devices().count(), 2);

    dia.cmdRemoveDevice(dia.devices().findData(100));
    QCOMPARE(dia.devices().count(), 1);
    dia.cmdRemoveDevice(dia.devices().findData(101));
    QCOMPARE(dia.devices().count(), 0);

    dia.undo();
    QCOMPARE(dia.devices().count(), 1);
    QCOMPARE(dia.devices().findData(101)->pos(), QPointF(100, 200));
    dia.undo();
    QCOMPARE(dia.devices().count(), 2);
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(50, 100));
    QCOMPARE(dia.devices().findData(101)->pos(), QPointF(100, 200));

    dia.redo();
    QCOMPARE(dia.devices().count(), 1);
    QCOMPARE(dia.devices().count(), 1);
    QCOMPARE(dia.devices().findData(101)->pos(), QPointF(100, 200));

    dia.redo();
    QCOMPARE(dia.devices().count(), 0);
}

void TestDiagram::moveSelectedBy()
{
    Diagram dia(100, 100);
    dia.devices().add(make_dev(100, { 50, 100 }));
    dia.devices().setSelected({ 100 }, true);

    dia.cmdMoveSelectedDevicesBy(10, 20);
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(60, 120));

    dia.undo();
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(50, 100));

    dia.redo();
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(60, 120));
}

void TestDiagram::moveSelectedFrom()
{
    Diagram dia(100, 100);
    dia.devices().add(make_dev(100, { 50, 100 }));
    dia.devices().setSelected({ 100 }, true);

    dia.cmdMoveSelectedDevicesFrom({ 0, 0 }, { 100, 50 });
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(150, 150));

    dia.undo();
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(50, 100));

    dia.redo();
    QCOMPARE(dia.devices().findData(100)->pos(), QPointF(150, 150));
}
