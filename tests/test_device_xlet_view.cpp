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
    DeviceXletView xv;
    QCOMPARE(xv.count(), 0);
    QCOMPARE(xv.cellCount(), 0);
    QCOMPARE(xv.maxColumnCount(), 8);
    QCOMPARE(xv.rowCount(), 0);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 0, 0));
}

void TestDeviceXletView::indexToCell()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    QVERIFY(!xv.indexToCell(0));

    xv.append({}, XletType::In, nullptr);
    QVERIFY(!xv.indexToCell(1));
    QVERIFY(xv.indexToCell(0));
    QCOMPARE(*xv.indexToCell(0), CellIndex(0, 0));

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(*xv.indexToCell(1), CellIndex(0, 1));
    QCOMPARE(*xv.indexToCell(2), CellIndex(0, 2));
    QCOMPARE(*xv.indexToCell(3), CellIndex(0, 3));
    QCOMPARE(*xv.indexToCell(4), CellIndex(1, 0));
    QCOMPARE(*xv.indexToCell(5), CellIndex(1, 1));
    QCOMPARE(*xv.indexToCell(6), CellIndex(1, 2));
    QCOMPARE(*xv.indexToCell(7), CellIndex(1, 3));
}

void TestDeviceXletView::boundingRect()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 0, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 1 * XW, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 2 * XW, XH));
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 3 * XW, XH));
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 4 * XW, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 4 * XW, 2 * XH));

    QVERIFY(xv.setMaxColumnCount(5));
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 5 * XW, XH));

    QVERIFY(xv.setMaxColumnCount(2));
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 2 * XW, 3 * XH));
}

void TestDeviceXletView::cellToIndex()
{
    DeviceXletView xv;

    QVERIFY(!xv.cellToIndex(0, 0));
    QVERIFY(!xv.cellToIndex(1, 0));

    xv.append({}, XletType::In, nullptr);
    QVERIFY(xv.cellToIndex(0, 0));
    QCOMPARE(*xv.cellToIndex(0, 0), 0);
    QVERIFY(!xv.cellToIndex(1, 0));
    QVERIFY(!xv.cellToIndex(0, 1));

    xv.append({}, XletType::In, nullptr);
    QVERIFY(xv.cellToIndex(0, 0));
    QVERIFY(xv.cellToIndex(0, 1));
    QCOMPARE(*xv.cellToIndex(0, 0), 0);
    QCOMPARE(*xv.cellToIndex(0, 1), 1);
    QVERIFY(!xv.cellToIndex(1, 0));
}

void TestDeviceXletView::clear()
{
    DeviceXletView xv;
    xv.clear();
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 2);
    xv.clear();
    QCOMPARE(xv.count(), 0);
}

void TestDeviceXletView::place()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    xv.placeXlets({});
    QCOMPARE(xv.boundingRect(), QRectF(0, 0, 0, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 1);
    xv.placeXlets({});
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint());

    xv.placeXlets(QPoint(10, XH));
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint(10, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 2);
    xv.placeXlets({});
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex(1)->pos(), QPoint(22, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 3);
    xv.placeXlets({});
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex(1)->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex(2)->pos(), QPoint(44, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 4);
    xv.placeXlets({});
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex(1)->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex(2)->pos(), QPoint(44, 0));
    QCOMPARE(xv.xletAtIndex(3)->pos(), QPoint(66, 0));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.count(), 5);
    xv.placeXlets({});
    QCOMPARE(xv.xletAtIndex(0)->pos(), QPoint(0, 0));
    QCOMPARE(xv.xletAtIndex(1)->pos(), QPoint(22, 0));
    QCOMPARE(xv.xletAtIndex(2)->pos(), QPoint(44, 0));
    QCOMPARE(xv.xletAtIndex(3)->pos(), QPoint(66, 0));
    QCOMPARE(xv.xletAtIndex(4)->pos(), QPoint(0, XH));
}

void TestDeviceXletView::posToCell()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(*xv.posToCell(QPoint(0, 0)), CellIndex(0, 0));
    QCOMPARE(*xv.posToCell(QPoint(XW, 0)), CellIndex(0, 1));
    QCOMPARE(*xv.posToCell(QPoint(2 * XW, 0)), CellIndex(0, 2));
    QCOMPARE(*xv.posToCell(QPoint(3 * XW, 0)), CellIndex(0, 3));

    QCOMPARE(*xv.posToCell(QPoint(0, XH)), CellIndex(1, 0));
    QCOMPARE(*xv.posToCell(QPoint(XW, XH)), CellIndex(1, 1));
    QCOMPARE(*xv.posToCell(QPoint(2 * XW, XH)), CellIndex(1, 2));
    QVERIFY(!xv.posToCell(QPoint(3 * XW, XH)));
}

void TestDeviceXletView::posToIndex()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(*xv.posToIndex(QPoint(0, 0)), 0);
    QCOMPARE(*xv.posToIndex(QPoint(XW, 0)), 1);
    QCOMPARE(*xv.posToIndex(QPoint(2 * XW, 0)), 2);
    QCOMPARE(*xv.posToIndex(QPoint(3 * XW, 0)), 3);
    QCOMPARE(*xv.posToIndex(QPoint(0, XH)), 4);
    QCOMPARE(*xv.posToIndex(QPoint(XW, XH)), 5);
    QCOMPARE(*xv.posToIndex(QPoint(2 * XW, XH)), 6);
    QVERIFY(!xv.posToIndex(QPoint(3 * XW, XH)));
}

void TestDeviceXletView::xletRect()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    QCOMPARE(xv.xletRect(0), QRect());
    QCOMPARE(xv.xletRect(1), QRect());

    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);
    xv.append({}, XletType::In, nullptr);

    QCOMPARE(xv.xletRect(0), QRect(0, 0, 22, XH));
    QCOMPARE(xv.xletRect(1), QRect(22, 0, 22, XH));
    QCOMPARE(xv.xletRect(2), QRect(44, 0, 22, XH));
    QCOMPARE(xv.xletRect(3), QRect());

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.xletRect(0), QRect(0, 0, 22, XH));
    QCOMPARE(xv.xletRect(1), QRect(22, 0, 22, XH));
    QCOMPARE(xv.xletRect(2), QRect(44, 0, 22, XH));
    QCOMPARE(xv.xletRect(3), QRect(66, 0, 22, XH));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.xletRect(0), QRect(0, 0, 22, XH));
    QCOMPARE(xv.xletRect(1), QRect(22, 0, 22, XH));
    QCOMPARE(xv.xletRect(2), QRect(44, 0, 22, XH));
    QCOMPARE(xv.xletRect(3), QRect(66, 0, 22, XH));
    QCOMPARE(xv.xletRect(4), QRect(0, XH, 22, XH));
}

void TestDeviceXletView::inletConnectionPoint()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    QVERIFY(!xv.connectionPoint(0));
    QVERIFY(!xv.connectionPoint(1));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.connectionPoint(0), QPointF(11, 0));
    QVERIFY(!xv.connectionPoint(1));

    xv.append({}, XletType::In, nullptr);
    QCOMPARE(xv.connectionPoint(0), QPointF(11, 0));
    QCOMPARE(xv.connectionPoint(1), QPointF(11, 0));
    xv.placeXlets({});
    QCOMPARE(xv.connectionPoint(0), QPointF(11, 0));
    QCOMPARE(xv.connectionPoint(1), QPointF(33, 0));
    QVERIFY(!xv.connectionPoint(2));

    xv.append({}, XletType::In, nullptr);
    xv.placeXlets(QPoint(10, 0));
    QCOMPARE(xv.connectionPoint(0), QPointF(21, 0));
    QCOMPARE(xv.connectionPoint(1), QPointF(43, 0));
    QCOMPARE(xv.connectionPoint(2), QPointF(65, 0));

    xv.append({}, XletType::In, nullptr);
    xv.placeXlets(QPoint(0, 10));
    QCOMPARE(xv.connectionPoint(0), QPointF(11, 10));
    QCOMPARE(xv.connectionPoint(1), QPointF(33, 10));
    QCOMPARE(xv.connectionPoint(2), QPointF(55, 10));
    QCOMPARE(xv.connectionPoint(3), QPointF(77, 10));

    xv.append({}, XletType::In, nullptr);
    xv.placeXlets(QPoint(-10, 15));
    QCOMPARE(xv.connectionPoint(0), QPointF(1, 15));
    QCOMPARE(xv.connectionPoint(1), QPointF(23, 15));
    QCOMPARE(xv.connectionPoint(2), QPointF(45, 15));
    QCOMPARE(xv.connectionPoint(3), QPointF(67, 15));
    QCOMPARE(xv.connectionPoint(4), QPointF(1, 35));
}

void TestDeviceXletView::outletConnectionPoint()
{
    DeviceXletView xv;
    xv.setMaxColumnCount(4);
    QVERIFY(!xv.connectionPoint(0));
    QVERIFY(!xv.connectionPoint(1));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.connectionPoint(0), QPointF(11, XH));
    QVERIFY(!xv.connectionPoint(1));

    xv.append({}, XletType::Out, nullptr);
    QCOMPARE(xv.connectionPoint(0), QPointF(11, XH));
    QCOMPARE(xv.connectionPoint(1), QPointF(11, XH));
    xv.placeXlets({});
    QCOMPARE(xv.connectionPoint(0), QPointF(11, XH));
    QCOMPARE(xv.connectionPoint(1), QPointF(33, XH));
    QVERIFY(!xv.connectionPoint(2));

    xv.append({}, XletType::Out, nullptr);
    xv.placeXlets(QPoint(10, 0));
    QCOMPARE(xv.connectionPoint(0), QPointF(21, XH));
    QCOMPARE(xv.connectionPoint(1), QPointF(43, XH));
    QCOMPARE(xv.connectionPoint(2), QPointF(65, XH));

    xv.append({}, XletType::Out, nullptr);
    xv.placeXlets(QPoint(0, 10));
    QCOMPARE(xv.connectionPoint(0), QPointF(11, 30));
    QCOMPARE(xv.connectionPoint(1), QPointF(33, 30));
    QCOMPARE(xv.connectionPoint(2), QPointF(55, 30));
    QCOMPARE(xv.connectionPoint(3), QPointF(77, 30));

    xv.append({}, XletType::Out, nullptr);
    xv.placeXlets(QPoint(-10, 15));
    QCOMPARE(xv.connectionPoint(0), QPointF(1, 35));
    QCOMPARE(xv.connectionPoint(1), QPointF(23, 35));
    QCOMPARE(xv.connectionPoint(2), QPointF(45, 35));
    QCOMPARE(xv.connectionPoint(3), QPointF(67, 35));
    QCOMPARE(xv.connectionPoint(4), QPointF(1, 55));
}
