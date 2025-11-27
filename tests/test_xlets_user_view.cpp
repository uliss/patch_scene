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
#include "QtTest/qtestcase.h"
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

    QCOMPARE(xv.data().columnCount(), 6);
    QCOMPARE(xv.data().rowCount(), 3);
    QCOMPARE(xv.data().cellCount(), 18);
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

    xv.data().setColumnCount(3);
    xv.data().setRowCount(2);
    QCOMPARE(xv.data().cellCount(), 6);
    QCOMPARE(xv.data().columnCount(), 3);
    QCOMPARE(xv.data().rowCount(), 2);

    QCOMPARE(xv.posToIndex({ 0, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 0, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, XH }), std::nullopt);

    QVERIFY(xv.data().insertXlet({ 0, 0 }, XletViewIndex { 0, XletType::In }));

    QCOMPARE(xv.posToIndex({ 0, 0 }), XletViewIndex(0, XletType::In));
    QCOMPARE(xv.posToIndex({ XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, 0 }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 0, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ XW, XH }), std::nullopt);
    QCOMPARE(xv.posToIndex({ 2 * XW, XH }), std::nullopt);

    QVERIFY(xv.data().insertXlet({ 1, 2 }, XletViewIndex { 0, XletType::Out }));

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

    xv.data().setColumnCount(3);
    xv.data().setRowCount(2);

    QVERIFY(xv.data().insertXlet({ 0, 0 }, XletViewIndex { 0, XletType::In }));
    QVERIFY(xv.data().insertXlet({ 1, 2 }, XletViewIndex { 0, XletType::Out }));

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
    xv.data().setColumnCount(3);
    xv.data().setRowCount(2);

    xv.placeXlets({});

    for (int i = 0; i < xlets.inletCount(); i++) {
        QCOMPARE(xlets.inletAt(i)->pos(), QPoint());
        QVERIFY(!xlets.inletAt(i)->isVisible());
    }

    for (int i = 0; i < xlets.outletCount(); i++) {
        QCOMPARE(xlets.outletAt(i)->pos(), QPoint());
        QVERIFY(!xlets.outletAt(i)->isVisible());
    }

    QVERIFY(xv.data().insertXlet({ 1, 1 }, { 0, XletType::In }));
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
    QVERIFY(xlets.currentView());
    QCOMPARE(xlets.userViewCount(), 0);
    auto xv = dynamic_cast<XletsUserView*>(xlets.currentView());
    QVERIFY(!xv);

    SharedItemData data(new ItemData(SCENE_ITEM_NULL_ID));
    XletsUserViewData vdata { 2, 3 };
    vdata.setName("UserView");
    data->userViewData().append(vdata);

    xlets.setData(data);
    QCOMPARE(xlets.userViewCount(), 1);

    xlets.setCurrentView("UserView");
    QVERIFY(dynamic_cast<XletsUserView*>(xlets.currentView()));
    auto vuser = dynamic_cast<XletsUserView*>(xlets.currentView());
    QCOMPARE(vuser->data().rowCount(), 2);
    QCOMPARE(vuser->data().columnCount(), 3);
    QCOMPARE(vuser->data().cellCount(), 6);

    vdata.setColumnCount(5);
    vdata.setRowCount(4);
    vdata.setName("UserView 2");
    QVERIFY(vdata.insertXlet({ 3, 1 }, { 11, XletType::In }));
    data->userViewData().append(vdata);

    xlets.setData(data);
    QCOMPARE(xlets.userViewCount(), 2);

    xlets.setCurrentView("UserView 2");
    QVERIFY(dynamic_cast<XletsUserView*>(xlets.currentView()));
    vuser = dynamic_cast<XletsUserView*>(xlets.currentView());
    QCOMPARE(vuser->data().columnCount(), 5);
    QCOMPARE(vuser->data().rowCount(), 4);
    QVERIFY(vuser->data().xletAt(16).isInlet());
}

void TestXletsUserView::testToJson()
{
    XletsUserViewData vdata { 2, 3 };
    vdata.setName("Name");
    vdata.insertXlet({ 0, 0 }, { 12, XletType::In });
    vdata.insertXlet({ 1, 0 }, { 3, XletType::Out });

    auto j = vdata.toJson();

    QCOMPARE(j["name"].toString(), "Name");
    QCOMPARE(j["num-rows"].toInt(), 2);
    QCOMPARE(j["num-cols"].toInt(), 3);
    QVERIFY(j["indexes"].isArray());

    auto arr = j["indexes"].toArray();
    QCOMPARE(arr.size(), 2);
    QVERIFY(arr[0].isObject());
    QVERIFY(arr[1].isObject());
    QCOMPARE(arr[0].toObject()["src"].toInt(), 12);
    QCOMPARE(arr[0].toObject()["type"].toString(), "in");
    QCOMPARE(arr[1].toObject()["src"].toInt(), 3);
    QCOMPARE(arr[1].toObject()["type"].toString(), "out");
}

void TestXletsUserView::testFromJson()
{
}

void TestXletsUserView::testJson()
{
    XletsUserViewData vd0 { 2, 3 };
    vd0.setName("Name");

    auto vd1 = XletsUserViewData::fromJson(vd0.toJson());
    QVERIFY(vd1);
    QCOMPARE(vd0, *vd1);

    vd0.insertXlet({ 0, 0 }, { 12, XletType::In });
    vd0.insertXlet({ 1, 0 }, { 3, XletType::Out });

    vd1 = XletsUserViewData::fromJson(vd0.toJson());
    QVERIFY(vd1);
    QCOMPARE(vd0, *vd1);
}
