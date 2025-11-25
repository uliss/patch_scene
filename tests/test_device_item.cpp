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
#include "test_device_item.h"
#include "scene_item.h"
#include "xlets_view.h"

#include <QTest>

using namespace ceam;

namespace {

constexpr int XW = 22;
constexpr int XH = 20;
constexpr int MIN_TXT_WD = 80;
constexpr int MIN_TITLE_WD = 70;

#ifdef Q_OS_LINUX
constexpr int DEF_TXT_HT = 26;
#else
constexpr int DEF_TXT_HT = 24;
#endif

SharedDeviceData make_data(SceneItemId id, int numIn = 0, int numOut = 0, const QString& title = {})
{
    auto data = new DeviceData(id);
    data->setTitle(title);
    data->setShowTitle(!title.isEmpty());
    data->logicViewData().setMaxInputColumnCount(4);
    data->logicViewData().setMaxOutputColumnCount(4);

    for (auto i = 0; i < numIn; i++)
        data->appendInput({});

    for (auto i = 0; i < numOut; i++)
        data->appendOutput({});

    return SharedDeviceData { data };
}
} // namespace

void TestDeviceItem::qtTest()
{
    QGraphicsTextItem xi("text");
    QCOMPARE(xi.boundingRect().height(), DEF_TXT_HT);

    xi.setPlainText("....");
    QCOMPARE(xi.boundingRect().height(), DEF_TXT_HT);

    xi.setTextWidth(MIN_TITLE_WD);
    QCOMPARE(xi.boundingRect().height(), DEF_TXT_HT);
}

void TestDeviceItem::createDefault()
{
    SceneItem dev;
    QVERIFY(dev.id() != SCENE_ITEM_NULL_ID);
    QCOMPARE(dev.deviceData()->inputs().count(), 4);
    QCOMPARE(dev.deviceData()->outputs().count(), 2);
    QCOMPARE(dev.xlets().inletCount(), 4);
    QCOMPARE(dev.xlets().outletCount(), 2);
    QVERIFY(dev.xlets().currentView());
    QCOMPARE(dev.xlets().userViewCount(), 0);

    QCOMPARE(dev.boundingRect(), QRectF(-2 * XW, 0, 4 * XW, 2 * XH + DEF_TXT_HT));
    QVERIFY(dev.xlets().currentView());
    auto view = dynamic_cast<XletsLogicView*>(dev.xlets().currentView());
    QVERIFY(view);

    QCOMPARE(dev.xlets().inletCount(), 4);
    QCOMPARE(dev.xlets().outletCount(), 2);
    QCOMPARE(view->width(), 4 * XW);
    view->placeXlets({});

    QCOMPARE(dev.xlets().connectionPoint({ 0, XletType::In }), QPointF(0.5 * XW, 0));
    QCOMPARE(dev.xlets().connectionPoint({ 1, XletType::In }), QPointF(1.5 * XW, 0));
    QCOMPARE(dev.xlets().connectionPoint({ 2, XletType::In }), QPointF(2.5 * XW, 0));
    QCOMPARE(dev.xlets().connectionPoint({ 3, XletType::In }), QPointF(3.5 * XW, 0));
    QVERIFY(!dev.xlets().connectionPoint({ 4, XletType::In }));

    QCOMPARE(dev.xlets().connectionPoint({ 0, XletType::Out }), QPointF(1.5 * XW, 2 * XH));
    QCOMPARE(dev.xlets().connectionPoint({ 1, XletType::Out }), QPointF(2.5 * XW, 2 * XH));
    QVERIFY(!dev.xlets().connectionPoint({ 2, XletType::Out }));
}

void TestDeviceItem::createNoTitle()
{
    {
        SceneItem dev(make_data(100, 2, 0));
        QCOMPARE(dev.id(), 100);

        QCOMPARE(dev.deviceData()->inputs().count(), 2);
        QCOMPARE(dev.deviceData()->outputs().count(), 0);

        QCOMPARE(dev.boundingRect(), QRectF(-1 * XW, 0, 2 * XW, XH));
        QCOMPARE(dev.xletRect(), QRect(-1 * XW, 0, 2 * XW, XH));
    }

    {
        SceneItem dev(make_data(100, 0, 1));

        QCOMPARE(dev.boundingRect(), QRectF(-0.5 * XW, 0, XW, XH));
        QCOMPARE(dev.xletRect(), QRect(-0.5 * XW, 0, XW, XH));
    }

    {
        SceneItem dev(make_data(100, 2, 1));

        QCOMPARE(dev.boundingRect(), QRectF(-1 * XW, 0, 2 * XW, 2 * XH));
        QCOMPARE(dev.xletRect(), QRect(-1 * XW, 0, 2 * XW, 2 * XH));
    }
}

void TestDeviceItem::boundingRect()
{
    {
        SceneItem dev(make_data(100, 0, 0, {}));
        QCOMPARE(dev.boundingRect(), QRect(0, 0, 0, 0));
    }

    {
        SceneItem dev(make_data(100, 1, 0, {}));
        QCOMPARE(dev.boundingRect(), QRectF(-0.5 * XW, 0, XW, XH));
    }

    {
        SceneItem dev(make_data(100, 2, 0, {}));
        QCOMPARE(dev.boundingRect(), QRectF(-1 * XW, 0, 2 * XW, XH));
    }

    {
        SceneItem dev(make_data(100, 2, 0, "MIN"));
        QCOMPARE(dev.boundingRect(), QRectF(-0.5 * MIN_TXT_WD, 0, MIN_TXT_WD, DEF_TXT_HT + XH));
    }
}

void TestDeviceItem::titleRect()
{
    {
        SceneItem dev(make_data(100, 0, 0, {}));
        QCOMPARE(dev.titleRect(), QRect());
    }

    {
        SceneItem dev(make_data(100, 0, 0, "..."));
        QCOMPARE(dev.titleRect(), QRectF(-0.5 * MIN_TITLE_WD, 0, MIN_TITLE_WD, DEF_TXT_HT));
    }
}

void TestDeviceItem::xletRect()
{
    {
        SceneItem dev(make_data(100, 0, 0, {}));
        QCOMPARE(dev.xletRect(), QRect());
    }

    {
        SceneItem dev(make_data(100, 1, 0, {}));
        QCOMPARE(dev.xletRect(), QRect(-0.5 * XW, 0, XW, XH));
    }

    {
        SceneItem dev(make_data(100, 2, 0, {}));
        QCOMPARE(dev.xletRect(), QRect(-XW, 0, 2 * XW, XH));
    }

    {
        SceneItem dev(make_data(100, 5, 0, {}));
        QCOMPARE(dev.xletRect(), QRect(-2 * XW, 0, 4 * XW, 2 * XH));
    }

    {
        SceneItem dev(make_data(100, 0, 1, {}));
        QCOMPARE(dev.xletRect(), QRect(-0.5 * XW, 0, XW, XH));
    }

    {
        SceneItem dev(make_data(100, 0, 2, {}));
        QCOMPARE(dev.xletRect(), QRect(-XW, 0, 2 * XW, XH));
    }

    {
        SceneItem dev(make_data(100, 0, 5, {}));
        QCOMPARE(dev.xletRect(), QRect(-2 * XW, 0, 4 * XW, 2 * XH));
    }

    {
        SceneItem dev(make_data(100, 2, 2, {}));
        QCOMPARE(dev.xletRect(), QRect(-XW, 0, 2 * XW, 2 * XH));
    }

    {
        SceneItem dev(make_data(100, 2, 1, {}));
        QCOMPARE(dev.xletRect(), QRect(-XW, 0, 2 * XW, 2 * XH));
    }

    {
        SceneItem dev(make_data(100, 1, 2, {}));
        QCOMPARE(dev.xletRect(), QRect(-XW, 0, 2 * XW, 2 * XH));
    }
}
