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
#include "test_device.h"
#include "device.h"

#include <QTest>

using namespace ceam;

constexpr int XW = 22;
constexpr int XH = 20;

void TestDevice::create()
{
    Device dev;
    QVERIFY(dev.id() != DEV_NULL_ID);
    QCOMPARE(dev.deviceData()->inputs().count(), 4);
    QCOMPARE(dev.deviceData()->outputs().count(), 2);

    QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, 2 * XH + 24));

    QCOMPARE(dev.inletPos(0), QPointF(0 * XW + 11, 24));
    QCOMPARE(dev.inletPos(1), QPointF(1 * XW + 11, 24));
    QCOMPARE(dev.inletPos(2), QPointF(2 * XW + 11, 24));
    QCOMPARE(dev.inletPos(3), QPointF(3 * XW + 11, 24));
    QCOMPARE(dev.inletPos(4), QPointF());

    QCOMPARE(dev.outletPos(0), QPointF(0 * XW + 11, 2 * XH + 24));
    QCOMPARE(dev.outletPos(1), QPointF(1 * XW + 11, 2 * XH + 24));
}
