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
#include "test_xlets_user_view.h"
#include "device_xlet_view.h"
#include "xlets_user_view.h"

#include <QJsonObject>
#include <QTest>

namespace {
constexpr int XW = 22;
constexpr int XH = 20;
}

using namespace ceam;

void TestXletsUserView::testInit()
{
    DeviceXlets xlets;
    XletsUserView xv("test", xlets);

    QCOMPARE(xv.columnCount(), 6);
    QCOMPARE(xv.rowCount(), 3);
    QCOMPARE(xv.cellCount(), 18);
    QCOMPARE(xv.width(), 132);
    QCOMPARE(xv.height(), 60);
    QCOMPARE(xv.posToIndex({ 0, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 50, 30 }), std::nullopt);
    QCOMPARE(xv.name(), "test");
}

void TestXletsUserView::testPosToIndex()
{
    DeviceXlets xlets;
    XletsUserView xv("test", xlets);

    xv.setColumnCount(3);
    xv.setRowCount(2);
    QCOMPARE(xv.cellCount(), 6);
    QCOMPARE(xv.columnCount(), 3);
    QCOMPARE(xv.rowCount(), 2);

    QCOMPARE(xv.posToIndex({ 0, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 0, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, XH }), std::nullopt);

    QVERIFY(xv.insertXlet({ 0, 0 }, XletViewIndex { 0, XletType::In }));

    QCOMPARE(xv.posToIndex({ 0, 0 }), XletViewIndex(0, XletType::In));
    QCOMPARE(xv.posToIndex({ XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 0, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, XH }), std::nullopt);

    QVERIFY(xv.insertXlet({ 1, 2 }, XletViewIndex { 0, XletType::Out }));

    QCOMPARE(xv.posToIndex({ 0, 0 }), XletViewIndex(0, XletType::In));
    QCOMPARE(xv.posToIndex({ XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 0, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, XH }), XletViewIndex(0, XletType::Out));
}

void TestXletsUserView::testIndexToPos()
{
    DeviceXlets xlets;
    XletsUserView xv("test", xlets);

    xv.setColumnCount(3);
    xv.setRowCount(2);

    QVERIFY(xv.insertXlet({ 0, 0 }, XletViewIndex { 0, XletType::In }));
    QVERIFY(xv.insertXlet({ 1, 2 }, XletViewIndex { 0, XletType::Out }));

    QCOMPARE(xv.indexToPos(XletViewIndex { 12, XletType::In }), std::nullopt);
    QCOMPARE(xv.indexToPos(XletViewIndex { 1, XletType::In }), std::nullopt);

    QCOMPARE(xv.indexToPos(XletViewIndex { 0, XletType::In }), QPoint(0, 0));
    QCOMPARE(xv.indexToPos(XletViewIndex { 0, XletType::Out }), QPoint(2 * XW, 1 * XH));
}

void TestXletsUserView::testPlaceXlets()
{
    DeviceXlets xlets;
    xlets.append({}, XletType::In, nullptr);
    xlets.append({}, XletType::In, nullptr);
    xlets.append({}, XletType::In, nullptr);
    xlets.append({}, XletType::Out, nullptr);
    xlets.append({}, XletType::Out, nullptr);

    XletsUserView xv("test", xlets);
    xv.setColumnCount(3);
    xv.setRowCount(2);

    xv.placeXlets({});

    for (int i = 0; i < xlets.inletCount(); i++) {
        QCOMPARE(xlets.inletAt(i)->pos(), QPoint());
        QVERIFY(!xlets.inletAt(i)->isVisible());
    }

    for (int i = 0; i < xlets.outletCount(); i++) {
        QCOMPARE(xlets.outletAt(i)->pos(), QPoint());
        QVERIFY(!xlets.outletAt(i)->isVisible());
    }

    QVERIFY(xv.insertXlet({ 1, 1 }, { 0, XletType::In }));
    xv.placeXlets({});
    QVERIFY(xlets.inletAt(0)->isVisible());
    QCOMPARE(xlets.inletAt(0)->pos(), QPoint(XW, XH));
    QVERIFY(!xlets.inletAt(1)->isVisible());
    QVERIFY(!xlets.inletAt(2)->isVisible());
    QVERIFY(!xlets.outletAt(0)->isVisible());
    QVERIFY(!xlets.outletAt(1)->isVisible());
}

void TestXletsUserView::testSetData()
{
    DeviceXlets xlets;

    QVERIFY(!xlets.currentView());
    xlets.initDefaultView();
    // QVERIFY(xlets.appendView(std::make_unique<XletsUserView>("test", xlets)));
    // xlets.setCurrentView("test");
    auto xv = dynamic_cast<XletsUserView*>(xlets.currentView());
    QVERIFY(!xv);
    // QCOMPARE(xv->columnCount(), 6);
    // QCOMPARE(xv->rowCount(), 3);

    SharedDeviceData data(new DeviceData(DEV_NULL_ID));

    QJsonObject jv, jd;
    jv["nrows"] = 2;
    jv["ncols"] = 3;
    jd["test"] = jv;

    data->setViewData(jd);

    xlets.setData(data);
    // QCOMPARE(xv->rowCount(), 2);
    // QCOMPARE(xv->columnCount(), 3);
}

void TestXletsUserView::testFactory()
{
}
