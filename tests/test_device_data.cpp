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
    QVERIFY(!data.isLocked());
    QCOMPARE(data.batteryCount(), 0);
    // for Qt6.2 using verify instead of compare
    QVERIFY(data.batteryType() == BatteryType::None);
    QCOMPARE(data.pos(), QPointF());
    QCOMPARE(data.zoom(), 1);
    // for Qt6.2 using verify instead of compare
    QVERIFY(data.category() == ItemCategory::Device);

    QVERIFY(data.info().isEmpty());
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
    QVERIFY(j.contains("locked"));
    QVERIFY(j.contains("view-logic"));
    QVERIFY(j.contains("view-user"));
    QVERIFY(!j.contains("input-columns"));
    QVERIFY(!j.contains("output-columns"));
    QVERIFY(j.contains("info"));
}

void TestDeviceData::fromJson()
{
    DeviceData data(DEV_NULL_ID);
    QVERIFY(!data.setJson(QJsonValue {}));

    QJsonObject j;
    j["id"] = 1000;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.id(), 1000);

    j["x"] = -10;
    j["y"] = 505.5;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.pos(), QPointF(-10, 505.5));

    j["title"] = "Test";
    QVERIFY(data.setJson(j));
    QCOMPARE(data.title(), "Test");

    j["vendor"] = "Vendor";
    QVERIFY(data.setJson(j));
    QCOMPARE(data.vendor(), "Vendor");

    j["model"] = "ModelX";
    QVERIFY(data.setJson(j));
    QCOMPARE(data.model(), "ModelX");

    j["zoom"] = 3.25;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.zoom(), 3.25);

    j["input-columns"] = 3;
    j["output-columns"] = 4;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.logicViewData().maxInputColumnCount(), 3);
    QCOMPARE(data.logicViewData().maxOutputColumnCount(), 4);

    j["locked"] = true;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.isLocked(), true);

    j["locked"] = false;
    QVERIFY(data.setJson(j));
    QCOMPARE(data.isLocked(), false);
}

void TestDeviceData::testJson()
{
    DeviceData d0(100), d1(100);
    QVERIFY(d0 == d1);

    d0.setId(101);
    QVERIFY(d0 == d1);
    d0.setTitle("title");
    QVERIFY(d0 != d1);

    auto j = d0.toJson();
    QVERIFY(d1.setJson(j));
    QVERIFY(d0 == d1);

    d0.setBatteryCount(10);
    QVERIFY(d0 != d1);
    j = d0.toJson();
    QVERIFY(d1.setJson(j));
    QVERIFY(d0 == d1);

    d0.info().append(std::pair { "test", "test" });
    QVERIFY(d0 != d1);
    j = d0.toJson();
    QVERIFY(d1.setJson(j));
    QVERIFY(d0 == d1);
}
