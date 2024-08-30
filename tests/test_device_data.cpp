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
#include "test_device_data.h"
#include "device_common.h"

#include <QTest>

using namespace ceam;

void TestDeviceData::construct()
{
    DeviceData data(DEV_NULL_ID);
    QVERIFY(data.isNull());
    QCOMPARE(data.id(), DEV_NULL_ID);
    QVERIFY(data.showInDeviceCategory());
    QVERIFY(data.showTitle());
    QCOMPARE(data.batteryCount(), 0);
    QCOMPARE(data.batteryType(), BatteryType::None);
    QCOMPARE(data.pos(), QPointF());
    QCOMPARE(data.zoom(), 1);
    QCOMPARE(data.category(), ItemCategory::Device);
    QVERIFY(!data.hasVisInputs());
    QVERIFY(!data.hasVisOutputs());
}

void TestDeviceData::hasVisInputs()
{
    DeviceData data(1);
    QVERIFY(!data.isNull());
    QCOMPARE(data.id(), 1);

    data.appendInput(XletData());
    QVERIFY(data.hasVisInputs());

    data.inputs().clear();
    QVERIFY(!data.hasVisInputs());

    data.appendInput(XletData());
    data.inputs().last().setVisible(false);
    QVERIFY(!data.hasVisInputs());

    data.appendInput(XletData());
    QVERIFY(data.hasVisInputs());
}

void TestDeviceData::hasVisOutputs()
{
    DeviceData data(1);
    QVERIFY(!data.isNull());
    QCOMPARE(data.id(), 1);

    data.appendOutput(XletData());
    QVERIFY(data.hasVisOutputs());

    data.outputs().clear();
    QVERIFY(!data.hasVisOutputs());

    data.appendOutput(XletData());
    data.outputs().last().setVisible(false);
    QVERIFY(!data.hasVisOutputs());

    data.appendOutput(XletData());
    QVERIFY(data.hasVisOutputs());
}

void TestDeviceData::visInputAt()
{
    DeviceData data(1);
    QVERIFY(!data.visInputAt(0));
    QVERIFY(!data.visInputAt(1));

    data.appendInput(XletData());
    QVERIFY(data.visInputAt(0));
    QVERIFY(!data.visInputAt(1));

    data.appendInput(XletData());
    data.inputs().last().setVisible(false);

    QVERIFY(data.visInputAt(0));
    QVERIFY(!data.visInputAt(1));

    data.inputs().last().setVisible(true);
    QVERIFY(data.visInputAt(1));
}

void TestDeviceData::visOutputAt()
{
    DeviceData data(1);
    QVERIFY(!data.visOutputAt(0));
    QVERIFY(!data.visOutputAt(1));

    data.appendOutput(XletData());
    QVERIFY(data.visOutputAt(0));
    QVERIFY(!data.visOutputAt(1));

    data.appendOutput(XletData());
    data.outputs().last().setVisible(false);

    QVERIFY(data.visOutputAt(0));
    QVERIFY(!data.visOutputAt(1));

    data.outputs().last().setVisible(true);
    QVERIFY(data.visOutputAt(1));
}
