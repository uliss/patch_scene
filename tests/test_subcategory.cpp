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
#include "test_subcategory.h"
#include "device_common.h"

#include <QTest>

using namespace ceam;

void TestSubCategory::toJson()
{
    SubCategory sub;
    QVERIFY(!sub.isValid());
    QCOMPARE(sub.toJson(), QJsonValue {});

    sub |= DeviceCategory::Amplifier;
    QVERIFY(sub.isValid());
    QVERIFY(sub & DeviceCategory::Amplifier);
    QCOMPARE(sub.toJson(), QJsonValue { "amp" });

    sub |= DeviceCategory::Midi;
    QVERIFY(sub & DeviceCategory::Amplifier);
    QVERIFY(sub & DeviceCategory::Midi);

    auto arr = QJsonArray({ "amp", "midi" });
    QCOMPARE(sub.toJson(), arr);

    sub |= InstrumentCategory::Keyboard;
    QVERIFY(sub & DeviceCategory::Amplifier);
    QVERIFY(sub & DeviceCategory::Midi);
    QVERIFY(!(sub & InstrumentCategory::Keyboard));

    arr = QJsonArray({ "amp", "midi" });
    QCOMPARE(sub.toJson(), arr);
}

void TestSubCategory::fromJson()
{
    auto j = SubCategory::fromJson("midi");
    SubCategory sub;
    sub |= DeviceCategory::Midi;
    QVERIFY(j);
    QCOMPARE(j, sub);

    j = SubCategory::fromJson(QJsonArray({ "midi", "amp" }));
    sub |= DeviceCategory::Amplifier;
    QVERIFY(j);
    QCOMPARE(j, sub);
}

void TestSubCategory::separate()
{
    QCOMPARE(SubCategory().separate().count(), 0);

    auto j = SubCategory::fromJson(QJsonArray({ "midi", "amp" }));
    auto sep = j->separate();
    QCOMPARE(sep.count(), 2);
    QVERIFY(sep[0] & DeviceCategory::Amplifier);
    QVERIFY(sep[1] & DeviceCategory::Midi);
}
