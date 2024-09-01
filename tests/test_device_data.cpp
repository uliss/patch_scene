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

#include <QJsonObject>
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
}

void TestDeviceData::toJson()
{
    DeviceData data(DEV_NULL_ID);
    auto j = data.toJson();
    QVERIFY(j.contains("id"));
    QVERIFY(j.contains("x"));
    QVERIFY(j.contains("y"));
    QVERIFY(j.contains("title"));
    QVERIFY(j.contains("vendor"));
    QVERIFY(j.contains("model"));
    QVERIFY(j.contains("zoom"));
    QVERIFY(j.contains("image"));
    QVERIFY(j.contains("category"));
    QVERIFY(j.contains("battery-type"));
    QVERIFY(j.contains("battery-count"));
    QVERIFY(j.contains("inputs"));
    QVERIFY(j.contains("outputs"));
    QVERIFY(j.contains("show-title"));
    QVERIFY(j.contains("xlet-columns"));
}
