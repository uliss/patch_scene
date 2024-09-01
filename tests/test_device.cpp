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
constexpr int MIN_TXT_WD = 80;
constexpr int DEF_TXT_HT = 24;

SharedDeviceData make_data(DeviceId id, int numIn = 0, int numOut = 0, const QString& title = {})
{
    auto data = new DeviceData(id);
    data->setShowTitle(!title.isEmpty());

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

    QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, 2 * XH + DEF_TXT_HT));
    QCOMPARE(dev.inletsRect().width(), 4 * XW);
    QCOMPARE(dev.inletsYOff(), DEF_TXT_HT);
    QCOMPARE(dev.inletsRect(), QRect(-2 * XW, DEF_TXT_HT, 4 * XW, XH));

    QCOMPARE(dev.inConnectionPoint(0), QPointF(-1.5 * XW, DEF_TXT_HT));
    QCOMPARE(dev.inConnectionPoint(1), QPointF(-0.5 * XW, DEF_TXT_HT));
    QCOMPARE(dev.inConnectionPoint(2), QPointF(0.5 * XW, DEF_TXT_HT));
    QCOMPARE(dev.inConnectionPoint(3), QPointF(1.5 * XW, DEF_TXT_HT));
    QCOMPARE(dev.inConnectionPoint(4), QPointF());

    QCOMPARE(dev.outConnectionPoint(0), QPointF(-0.5 * XW, 2 * XH + DEF_TXT_HT));
    QCOMPARE(dev.outConnectionPoint(1), QPointF(0.5 * XW, 2 * XH + DEF_TXT_HT));
    QCOMPARE(dev.outConnectionPoint(2), QPointF());
}

void TestDevice::createNoTitle()
{
    {
        Device dev(make_data(100, 2, 0));
        QCOMPARE(dev.id(), 100);

        QCOMPARE(dev.deviceData()->inputs().count(), 2);
        QCOMPARE(dev.deviceData()->outputs().count(), 0);

        QCOMPARE(dev.boundingRect(), QRectF(-1 * XW, 0, 2 * XW, XH));
        QCOMPARE(dev.inletsRect(), QRect(-1 * XW, 0, 2 * XW, XH));
        QCOMPARE(dev.inConnectionPoint(0), QPointF(-0.5 * XW, 0));
        QCOMPARE(dev.inConnectionPoint(1), QPointF(0.5 * XW, 0));
        QCOMPARE(dev.outConnectionPoint(0), QPointF());
    }

    {
        Device dev(make_data(100, 0, 1));

        QCOMPARE(dev.boundingRect(), QRectF(-0.5 * XW, 0, XW, XH));
        QCOMPARE(dev.inletsRect(), QRect(0, 0, 0, 0));
        QCOMPARE(dev.inConnectionPoint(0), QPointF());
        QCOMPARE(dev.inConnectionPoint(1), QPointF());
        QCOMPARE(dev.outConnectionPoint(0), QPointF(0, XH));
    }

    {
        Device dev(make_data(100, 2, 1));

        QCOMPARE(dev.boundingRect(), QRectF(-1 * XW, 0, 2 * XW, 2 * XH));
        QCOMPARE(dev.inletsRect(), QRect(-1 * XW, 0, 2 * XW, XH));
        QCOMPARE(dev.inConnectionPoint(0), QPointF(-0.5 * XW, 0));
        QCOMPARE(dev.inConnectionPoint(1), QPointF(0.5 * XW, 0));
        QCOMPARE(dev.outConnectionPoint(0), QPointF(0, 2 * XH));
    }
}

void TestDevice::inletRect()
{
    {
        Device dev(make_data(100, 0, 0, {}));
        QCOMPARE(dev.inletsRect(), QRect(0, 0, 0, 0));
    }

    {
        Device dev(make_data(100, 1, 0, {}));
        QCOMPARE(dev.inletsRect(), QRectF(-0.5 * XW, 0, XW, XH));
    }

    {
        Device dev(make_data(100, 2, 0, {}));
        QCOMPARE(dev.inletsRect(), QRectF(-1 * XW, 0, 2 * XW, XH));
    }

    {
        Device dev(make_data(100, 2, 0, "MIN"));
        QCOMPARE(dev.inletsRect(), QRectF(-1 * XW, DEF_TXT_HT, 2 * XW, XH));
    }
}

void TestDevice::outletRect()
{
    {
        Device dev(make_data(100, 0, 0, {}));
        QCOMPARE(dev.outletsRect(), QRect(0, 0, 0, 0));
    }

    {
        Device dev(make_data(100, 0, 1, {}));
        QCOMPARE(dev.outletsRect(), QRectF(-0.5 * XW, 0, XW, XH));
    }

    {
        Device dev(make_data(100, 4, 2, {}));
        QCOMPARE(dev.outletsRect(), QRectF(-XW, XH, 2 * XW, XH));
    }

    {
        Device dev(make_data(100, 4, 5, {}));
        QCOMPARE(dev.outletsRect(), QRectF(-2 * XW, XH, 4 * XW, 2 * XH));
    }

    {
        Device dev(make_data(100, 7, 5, {}));
        QCOMPARE(dev.outletsRect(), QRectF(-2 * XW, 2 * XH, 4 * XW, 2 * XH));
    }
}
