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

namespace {

constexpr int XW = 22;
constexpr int XH = 20;

SharedDeviceData data_no_title(DeviceId id, int numIn = 0, int numOut = 0)
{
    auto data = new DeviceData(id);
    data->setShowTitle(false);
    for (auto i = 0; i < numIn; i++)
        data->appendInput({});

    for (auto i = 0; i < numOut; i++)
        data->appendOutput({});

    return SharedDeviceData { data };
}
}

void TestDevice::createDefault()
{
    Device dev;
    QVERIFY(dev.id() != DEV_NULL_ID);
    QCOMPARE(dev.deviceData()->inputs().count(), 4);
    QCOMPARE(dev.deviceData()->outputs().count(), 2);

    QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, 2 * XH + 24));

    QCOMPARE(dev.inConnectionPoint(0), QPointF(0 * XW + 11, 24));
    QCOMPARE(dev.inConnectionPoint(1), QPointF(1 * XW + 11, 24));
    QCOMPARE(dev.inConnectionPoint(2), QPointF(2 * XW + 11, 24));
    QCOMPARE(dev.inConnectionPoint(3), QPointF(3 * XW + 11, 24));
    QCOMPARE(dev.inConnectionPoint(4), QPointF());

    QCOMPARE(dev.outConnectionPoint(0), QPointF(0 * XW + 11, 2 * XH + 24));
    QCOMPARE(dev.outConnectionPoint(1), QPointF(1 * XW + 11, 2 * XH + 24));
    QCOMPARE(dev.outConnectionPoint(2), QPointF());
}

void TestDevice::createNoTitle()
{
    {
        Device dev(data_no_title(100, 2, 0));
        QCOMPARE(dev.id(), 100);

        QCOMPARE(dev.deviceData()->inputs().count(), 2);
        QCOMPARE(dev.deviceData()->outputs().count(), 0);

        QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, XH));
        QCOMPARE(dev.inConnectionPoint(0), QPointF(0 * XW + 11, 0));
        QCOMPARE(dev.inConnectionPoint(1), QPointF(1 * XW + 11, 0));
        QCOMPARE(dev.outConnectionPoint(0), QPointF());
    }

    {
        Device dev(data_no_title(100, 0, 1));
        QCOMPARE(dev.id(), 100);

        QCOMPARE(dev.deviceData()->inputs().count(), 0);
        QCOMPARE(dev.deviceData()->outputs().count(), 1);

        QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, XH));
        QCOMPARE(dev.inConnectionPoint(0), QPointF());
        QCOMPARE(dev.inConnectionPoint(1), QPointF());
        QCOMPARE(dev.outConnectionPoint(0), QPointF(0 * XW + 11, XH));
    }

    {
        Device dev(data_no_title(100, 2, 1));
        QCOMPARE(dev.id(), 100);

        QCOMPARE(dev.deviceData()->inputs().count(), 2);
        QCOMPARE(dev.deviceData()->outputs().count(), 1);

        QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, 2 * XH));
        QCOMPARE(dev.inConnectionPoint(0), QPointF(0 * XW + 11, 0));
        QCOMPARE(dev.inConnectionPoint(1), QPointF(1 * XW + 11, 0));
        QCOMPARE(dev.outConnectionPoint(0), QPointF(0 * XW + 11, 2 * XH));
    }
}
