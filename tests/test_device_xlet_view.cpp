/*****************************************************************************
 * Copyright XH24 Serge Poltavski. All rights reserved.
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
#include "test_device_xlet_view.h"
#include "device_xlet.h"
#include "device_xlet_view.h"
#include "socket.h"

#include <QTest>

namespace {
constexpr int XW = 22;
constexpr int XH = 20;
}

using namespace ceam;

void TestDeviceXletView::create()
{
    DeviceXlets xv;
    QCOMPARE(xv.isEmpty(), true);
    QCOMPARE(xv.inletCount(), 0);
    QCOMPARE(xv.outletCount(), 0);
    QCOMPARE(xv.currentView(), nullptr);
}

void TestDeviceXletView::indexToCell()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);

    QCOMPARE(view->data().maxInputColumnCount(), 8);
    QCOMPARE(view->data().maxOutputColumnCount(), 8);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    QVERIFY(!view->indexToCell({ 0, XletType::In }));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->name(), "logical");
    QCOMPARE(view->width(), 22);
    QCOMPARE(view->height(), 20);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 22, 20));
    QVERIFY(!view->indexToCell({ 1, XletType::In }));
    QVERIFY(!view->indexToCell({ 0, XletType::Out }));
    QVERIFY(!view->indexToCell({ 1, XletType::Out }));
    QCOMPARE(view->indexToCell({ 0, XletType::In }), CellIndex(0, 0));

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(view->indexToCell({ 1, XletType::In }), CellIndex(0, 1));
    QCOMPARE(view->indexToCell({ 2, XletType::In }), CellIndex(0, 2));
    QCOMPARE(view->indexToCell({ 3, XletType::In }), CellIndex(0, 3));
    QCOMPARE(view->indexToCell({ 4, XletType::In }), CellIndex(1, 0));
    QCOMPARE(view->indexToCell({ 5, XletType::In }), CellIndex(1, 1));
    QCOMPARE(view->indexToCell({ 6, XletType::In }), CellIndex(1, 2));
    QCOMPARE(view->indexToCell({ 7, XletType::In }), CellIndex(1, 3));
}

void TestDeviceXletView::boundingRect()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    QCOMPARE(view->boundingRect(), QRectF(0, 0, 0, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 1 * XW, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 2 * XW, XH));
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 3 * XW, XH));
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 4 * XW, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 4 * XW, 2 * XH));

    QVERIFY(view->data().setMaxInputColumnCount(5));
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 5 * XW, XH));

    QVERIFY(view->data().setMaxInputColumnCount(2));
    QCOMPARE(view->boundingRect(), QRectF(0, 0, 2 * XW, 3 * XH));
}

void TestDeviceXletView::cellToIndex()
{
    DeviceXlets xv;
    xv.clearXlets();
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    QVERIFY(!view->cellToIndex(CellIndex(0, 0), XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(1, 0), XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(-1, 0), XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(0, -1), XletType::In));

    xv.append({}, XletType::In, nullptr);
    QVERIFY(view->cellToIndex({ 0, 0 }, XletType::In));
    QCOMPARE(view->cellToIndex({ 0, 0 }, XletType::In), XletViewIndex(0, XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(1, 0), XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(0, 1), XletType::In));
    QVERIFY(!view->cellToIndex(CellIndex(0, -1), XletType::In));

    xv.append({}, XletType::In, nullptr);
    QVERIFY(view->cellToIndex({ 0, 0 }, XletType::In));
    QVERIFY(view->cellToIndex({ 0, 1 }, XletType::In));

    QCOMPARE(view->cellToIndex({ 0, 0 }, XletType::In), XletViewIndex(0, XletType::In));
    QCOMPARE(view->cellToIndex({ 0, 1 }, XletType::In), XletViewIndex(1, XletType::In));

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QVERIFY(view->cellToIndex({ 0, 0 }, XletType::In));
    QVERIFY(view->cellToIndex({ 0, 1 }, XletType::In));
    QVERIFY(view->cellToIndex({ 0, 2 }, XletType::In));
    QVERIFY(view->cellToIndex({ 0, 3 }, XletType::In));
    QVERIFY(view->cellToIndex({ 1, 0 }, XletType::In));
    QVERIFY(view->cellToIndex({ 1, 1 }, XletType::In));

    QCOMPARE(view->cellToIndex({ 0, 0 }, XletType::In), XletViewIndex(0, XletType::In));
    QCOMPARE(view->cellToIndex({ 0, 1 }, XletType::In), XletViewIndex(1, XletType::In));
    QCOMPARE(view->cellToIndex({ 0, 2 }, XletType::In), XletViewIndex(2, XletType::In));
    QCOMPARE(view->cellToIndex({ 0, 3 }, XletType::In), XletViewIndex(3, XletType::In));
    QCOMPARE(view->cellToIndex({ 1, 0 }, XletType::In), XletViewIndex(4, XletType::In));
    QCOMPARE(view->cellToIndex({ 1, 1 }, XletType::In), XletViewIndex(5, XletType::In));

    QVERIFY(view->data().setMaxOutputColumnCount(3));
    xv.append({}, XletType::Out, nullptr);
    xv.append({}, XletType::Out, nullptr);
    xv.append({}, XletType::Out, nullptr);
    xv.append({}, XletType::Out, nullptr);
    xv.append({}, XletType::Out, nullptr);
    xv.append({}, XletType::Out, nullptr);

    QVERIFY(view->cellToIndex({ 0, 0 }, XletType::Out));
    QVERIFY(view->cellToIndex({ 0, 1 }, XletType::Out));
    QVERIFY(view->cellToIndex({ 0, 2 }, XletType::Out));
    QVERIFY(view->cellToIndex({ 1, 0 }, XletType::Out));
    QVERIFY(view->cellToIndex({ 1, 1 }, XletType::Out));
    QVERIFY(view->cellToIndex({ 1, 2 }, XletType::Out));

    QCOMPARE(view->cellToIndex({ 0, 0 }, XletType::Out), XletViewIndex(0, XletType::Out));
    QCOMPARE(view->cellToIndex({ 0, 1 }, XletType::Out), XletViewIndex(1, XletType::Out));
    QCOMPARE(view->cellToIndex({ 0, 2 }, XletType::Out), XletViewIndex(2, XletType::Out));
    QCOMPARE(view->cellToIndex({ 1, 0 }, XletType::Out), XletViewIndex(3, XletType::Out));
    QCOMPARE(view->cellToIndex({ 1, 1 }, XletType::Out), XletViewIndex(4, XletType::Out));
    QCOMPARE(view->cellToIndex({ 1, 2 }, XletType::Out), XletViewIndex(5, XletType::Out));
}

void TestDeviceXletView::clear()
{
    DeviceXlets xv;
    xv.clearXlets();
    xv.initDefaultView();

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 2);
    QCOMPARE(xv.outletCount(), 0);
    QVERIFY(!xv.isEmpty());
    xv.clearXlets();
    QCOMPARE(xv.inletCount(), 0);
    QCOMPARE(xv.outletCount(), 0);
    QVERIFY(xv.isEmpty());
    QVERIFY(xv.currentView());
    QCOMPARE(xv.userViewCount(), 0);

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::Out, nullptr);
    QVERIFY(!xv.isEmpty());
    QCOMPARE(xv.inletCount(), 1);
    QCOMPARE(xv.outletCount(), 1);
    xv.clearXlets();
    QVERIFY(xv.isEmpty());
    QCOMPARE(xv.inletCount(), 0);
    QCOMPARE(xv.outletCount(), 0);
}

void TestDeviceXletView::place()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    QCOMPARE(view->boundingRect(), QRectF(0, 0, 0, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 1);
    view->placeXlets({});
    QVERIFY(xv.xletAtIndex({ 0, XletType::In }));
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint());

    view->placeXlets(QPoint(10, XH));
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(10, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 2);
    view->placeXlets({});
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::In })->pos(), QPoint(22, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 3);
    view->placeXlets({});
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::In })->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::In })->pos(), QPoint(44, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 4);
    view->placeXlets({});
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::In })->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::In })->pos(), QPoint(44, 0));
    QCOMPARE(xv.xletAtIndex({ 3, XletType::In })->pos(), QPoint(66, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.inletCount(), 5);
    view->placeXlets({});
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::In })->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::In })->pos(), QPoint(44, 0));
    QCOMPARE(xv.xletAtIndex({ 3, XletType::In })->pos(), QPoint(66, 0));
    QCOMPARE(xv.xletAtIndex({ 4, XletType::In })->pos(), QPoint(0, XH));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.outletCount(), 1);
    view->placeXlets({});
    QCOMPARE(xv.xletAtIndex({ 0, XletType::In })->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::In })->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::In })->pos(), QPoint(44, 0));
    QCOMPARE(xv.xletAtIndex({ 3, XletType::In })->pos(), QPoint(66, 0));
    QCOMPARE(xv.xletAtIndex({ 4, XletType::In })->pos(), QPoint(0, XH));
    //  [] [] [] []
    //  []
    //      []
    QCOMPARE(xv.xletAtIndex({ 0, XletType::Out })->pos(), QPoint(33, 2 * XH));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.outletCount(), 2);
    view->placeXlets({});
    //  [] [] [] []
    //  []
    //     [] []
    QCOMPARE(xv.xletAtIndex({ 0, XletType::Out })->pos(), QPoint(22, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::Out })->pos(), QPoint(44, 2 * XH));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.outletCount(), 3);
    view->placeXlets({});
    //  [] [] [] []
    //  []
    //   [] [] []
    QCOMPARE(xv.xletAtIndex({ 0, XletType::Out })->pos(), QPoint(11, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::Out })->pos(), QPoint(33, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::Out })->pos(), QPoint(55, 2 * XH));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.outletCount(), 4);
    view->placeXlets({});
    //  [] [] [] []
    //  []
    //  [] [] [] []
    QCOMPARE(xv.xletAtIndex({ 0, XletType::Out })->pos(), QPoint(0 * XW, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 1, XletType::Out })->pos(), QPoint(1 * XW, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 2, XletType::Out })->pos(), QPoint(2 * XW, 2 * XH));
    QCOMPARE(xv.xletAtIndex({ 3, XletType::Out })->pos(), QPoint(3 * XW, 2 * XH));
}

void TestDeviceXletView::indexToCell2()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(view->indexToCell({ 0, XletType::In }), CellIndex(0, 0));
    QCOMPARE(view->indexToCell({ 1, XletType::In }), CellIndex(0, 1));
    QCOMPARE(view->indexToCell({ 2, XletType::In }), CellIndex(0, 2));
    QCOMPARE(view->indexToCell({ 3, XletType::In }), CellIndex(0, 3));
    QCOMPARE(view->indexToCell({ 4, XletType::In }), CellIndex(1, 0));
    QCOMPARE(view->indexToCell({ 5, XletType::In }), CellIndex(1, 1));
    QCOMPARE(view->indexToCell({ 6, XletType::In }), CellIndex(1, 2));
    QVERIFY(!view->indexToCell({ 7, XletType::In }));
    QVERIFY(!view->indexToCell({ 8, XletType::In }));
}

void TestDeviceXletView::posToIndex()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(view->posToIndex({ 0, 0 }), XletViewIndex(0, XletType::In));
    QCOMPARE(view->posToIndex({ XW, 0 }), XletViewIndex(1, XletType::In));
    QCOMPARE(view->posToIndex({ 2 * XW, 0 }), XletViewIndex(2, XletType::In));
    QCOMPARE(view->posToIndex({ 3 * XW, 0 }), XletViewIndex(3, XletType::In));
    QCOMPARE(view->posToIndex({ 0, XH }), XletViewIndex(4, XletType::In));
    QCOMPARE(view->posToIndex({ XW, XH }), XletViewIndex(5, XletType::In));
    QCOMPARE(view->posToIndex({ 2 * XW, XH }), XletViewIndex(6, XletType::In));
    QVERIFY(!view->posToIndex({ 3 * XW, XH }));
}

void TestDeviceXletView::xletRect()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);

    QVERIFY(view->data().setMaxInputColumnCount(4));
    QCOMPARE(view->xletRect({ 0, XletType::In }), QRect());
    QCOMPARE(view->xletRect({ 1, XletType::In }), QRect());
    QCOMPARE(view->xletRect({ 0, XletType::Out }), QRect());
    QCOMPARE(view->xletRect({ 1, XletType::Out }), QRect());

    QVERIFY(xv.append({}, XletType::In, nullptr));
    QVERIFY(xv.append({}, XletType::In, nullptr));
    QVERIFY(xv.append({}, XletType::In, nullptr));

    QCOMPARE(view->xletRect({ 0, XletType::In }), QRect(0, 0, 22, XH));
    QCOMPARE(view->xletRect({ 1, XletType::In }), QRect(22, 0, 22, XH));
    QCOMPARE(view->xletRect({ 2, XletType::In }), QRect(44, 0, 22, XH));
    QCOMPARE(view->xletRect({ 3, XletType::In }), QRect());

    QCOMPARE(view->xletRect({ 0, XletType::Out }), QRect());
    QCOMPARE(view->xletRect({ 1, XletType::Out }), QRect());

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->xletRect({ 0, XletType::In }), QRect(0, 0, 22, XH));
    QCOMPARE(view->xletRect({ 1, XletType::In }), QRect(22, 0, 22, XH));
    QCOMPARE(view->xletRect({ 2, XletType::In }), QRect(44, 0, 22, XH));
    QCOMPARE(view->xletRect({ 3, XletType::In }), QRect(66, 0, 22, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(view->xletRect({ 0, XletType::In }), QRect(0, 0, 22, XH));
    QCOMPARE(view->xletRect({ 1, XletType::In }), QRect(22, 0, 22, XH));
    QCOMPARE(view->xletRect({ 2, XletType::In }), QRect(44, 0, 22, XH));
    QCOMPARE(view->xletRect({ 3, XletType::In }), QRect(66, 0, 22, XH));
    QCOMPARE(view->xletRect({ 4, XletType::In }), QRect(0, XH, 22, XH));
}

void TestDeviceXletView::inletConnectionPoint()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxInputColumnCount(4));

    QVERIFY(!xv.connectionPoint({ 0, XletType::In }));
    QVERIFY(!xv.connectionPoint({ 1, XletType::In }));
    QVERIFY(!xv.connectionPoint({ 0, XletType::Out }));
    QCOMPARE(view->width(), 0);
    QCOMPARE(view->height(), 0);

    xv.append({}, XletType::In, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::In }), QPointF(11, 0));
    QVERIFY(!xv.connectionPoint({ 1, XletType::In }));
    QVERIFY(!xv.connectionPoint({ 0, XletType::Out }));
    QCOMPARE(view->width(), XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::In, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::In }), QPointF(11, 0));
    QCOMPARE(xv.connectionPoint({ 1, XletType::In }), QPointF(33, 0));
    QVERIFY(!xv.connectionPoint({ 2, XletType::In }));
    QCOMPARE(view->width(), 2 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::In, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::In }), QPointF(11, 0));
    QCOMPARE(xv.connectionPoint({ 1, XletType::In }), QPointF(33, 0));
    QCOMPARE(xv.connectionPoint({ 2, XletType::In }), QPointF(55, 0));
    QCOMPARE(view->width(), 3 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::In, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::In }), QPointF(11, 0));
    QCOMPARE(xv.connectionPoint({ 1, XletType::In }), QPointF(33, 0));
    QCOMPARE(xv.connectionPoint({ 2, XletType::In }), QPointF(55, 0));
    QCOMPARE(xv.connectionPoint({ 3, XletType::In }), QPointF(77, 0));
    QCOMPARE(view->width(), 4 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::In, nullptr);
    view->placeXlets({ 100, 200 });
    QCOMPARE(xv.connectionPoint({ 0, XletType::In }), QPointF(111, 200));
    QCOMPARE(xv.connectionPoint({ 1, XletType::In }), QPointF(133, 200));
    QCOMPARE(xv.connectionPoint({ 2, XletType::In }), QPointF(155, 200));
    QCOMPARE(xv.connectionPoint({ 3, XletType::In }), QPointF(177, 200));
    QCOMPARE(xv.connectionPoint({ 4, XletType::In }), QPointF(111, 220));
    QCOMPARE(view->width(), 4 * XW);
    QCOMPARE(view->height(), 2 * XH);
}

void TestDeviceXletView::outletConnectionPoint()
{
    DeviceXlets xv;
    xv.initDefaultView();
    auto view = dynamic_cast<XletsTableView*>(xv.currentView());
    QVERIFY(view);
    QVERIFY(view->data().setMaxOutputColumnCount(4));

    QVERIFY(!xv.connectionPoint({ 0, XletType::In }));
    QVERIFY(!xv.connectionPoint({ 1, XletType::In }));
    QVERIFY(!xv.connectionPoint({ 0, XletType::Out }));
    QCOMPARE(view->width(), 0);
    QCOMPARE(view->height(), 0);

    xv.append({}, XletType::Out, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.outletCount(), 1);
    QCOMPARE(view->xletRect({ 0, XletType::Out }), QRect(0, 0, 22, 20));
    QCOMPARE(xv.connectionPoint({ 0, XletType::Out }), QPointF(11, 20));
    QVERIFY(!xv.connectionPoint({ 1, XletType::Out }));
    QVERIFY(!xv.connectionPoint({ 0, XletType::In }));
    QCOMPARE(view->width(), XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::Out, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::Out }), QPointF(11, 20));
    QCOMPARE(xv.connectionPoint({ 1, XletType::Out }), QPointF(33, 20));
    QVERIFY(!xv.connectionPoint({ 2, XletType::Out }));
    QCOMPARE(view->width(), 2 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::Out, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::Out }), QPointF(11, 20));
    QCOMPARE(xv.connectionPoint({ 1, XletType::Out }), QPointF(33, 20));
    QCOMPARE(xv.connectionPoint({ 2, XletType::Out }), QPointF(55, 20));
    QCOMPARE(view->width(), 3 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::Out, nullptr);
    view->placeXlets({});
    QCOMPARE(xv.connectionPoint({ 0, XletType::Out }), QPointF(11, 20));
    QCOMPARE(xv.connectionPoint({ 1, XletType::Out }), QPointF(33, 20));
    QCOMPARE(xv.connectionPoint({ 2, XletType::Out }), QPointF(55, 20));
    QCOMPARE(xv.connectionPoint({ 3, XletType::Out }), QPointF(77, 20));
    QCOMPARE(view->width(), 4 * XW);
    QCOMPARE(view->height(), XH);

    xv.append({}, XletType::Out, nullptr);
    view->placeXlets({ 100, 200 });
    QCOMPARE(xv.connectionPoint({ 0, XletType::Out }), QPointF(111, 220));
    QCOMPARE(xv.connectionPoint({ 1, XletType::Out }), QPointF(133, 220));
    QCOMPARE(xv.connectionPoint({ 2, XletType::Out }), QPointF(155, 220));
    QCOMPARE(xv.connectionPoint({ 3, XletType::Out }), QPointF(177, 220));
    QCOMPARE(xv.connectionPoint({ 4, XletType::Out }), QPointF(111, 240));
    QCOMPARE(view->width(), 4 * XW);
    QCOMPARE(view->height(), 2 * XH);
}
