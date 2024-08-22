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
#include "test_xlet.h"
#include "device_xlet.h"

#include <QJsonObject>
#include <QTest>

using namespace ceam;

void TestXlet::init()
{
    XletData data;
    QCOMPARE(data.name, QString {});
    QCOMPARE(data.model, ConnectorModel::UNKNOWN);
    QCOMPARE(data.phantom_power, false);
    QCOMPARE(data.power_type, PowerType::None);
    QCOMPARE(data.type, ConnectorType::Socket_Female);
    QCOMPARE(data.visible, true);
    QCOMPARE(data.isPlug(), false);
    QCOMPARE(data.isSocket(), true);
    QCOMPARE(data.typeString(), "female");
    QCOMPARE(data.modelString(), "Unknown");
}

void TestXlet::json()
{
    XletData data1;
    XletData data2;

    QCOMPARE(data1, data2);
    QCOMPARE(data1.toJson(), data2.toJson());

    QCOMPARE(data1.toJson().count(), 6);

    data1.name = "Xlet";
    QCOMPARE_NE(data1, data2);
    QCOMPARE_NE(data1.toJson(), data2.toJson());

    QVERIFY(!XletData::fromJson({}));
    auto data3 = XletData::fromJson(data1.toJson());
    QVERIFY(data3);
    QCOMPARE_EQ(data1, data3.value());
}

static TestXlet test;
