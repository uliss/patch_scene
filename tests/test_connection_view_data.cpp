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
#include "test_connection_view_data.h"
#include "connection_data.h"

#include <QJsonObject>
#include <QTest>

using namespace ceam;

void TestConnectionViewData::testJson()
{
    ConnectionViewData cv;
    auto obj = cv.toJson();
    QVERIFY(!obj.isEmpty());
    QCOMPARE(obj["color"], "#000000");
    QCOMPARE(obj["cord"], "bezier");
    QCOMPARE(obj["width"], 1.5);
}

void TestConnectionViewData::testJsonColor()
{
    QCOMPARE(ConnectionViewData::colorFromString({}), Qt::black);
    QCOMPARE(ConnectionViewData::colorFromString("#"), Qt::black);
    QCOMPARE(ConnectionViewData::colorFromString("#123"), QColor(0x11, 0x22, 0x33));
    QCOMPARE(ConnectionViewData::colorFromString("#ABC"), QColor(0xAA, 0xBB, 0xCC));
    QCOMPARE(ConnectionViewData::colorFromString("#abc"), QColor(0xAA, 0xBB, 0xCC));
    QCOMPARE(ConnectionViewData::colorFromString("#123A"), QColor(0x11, 0x22, 0x33));
    QCOMPARE(ConnectionViewData::colorFromString("#DEADBE"), QColor(0xDE, 0xAD, 0xBE));
    QCOMPARE(ConnectionViewData::colorFromString("#abcdef"), QColor(0xAB, 0xCD, 0xEF));
    QCOMPARE(ConnectionViewData::colorFromString("#abcdef00"), QColor(0xAB, 0xCD, 0xEF));
    QCOMPARE(ConnectionViewData::colorFromString("#123456"), QColor(0x12, 0x34, 0x56));
}
