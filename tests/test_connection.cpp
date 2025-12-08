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
#include "test_connection.h"
#include "connection.h"
#include "device_item.h"
#include "diagram.h"
#include "diagram_scene.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QJsonObject>
#include <QJsonValue>
#include <QSignalSpy>
#include <QTest>

using namespace ceam;

void TestConnection::dataInit()
{
    ConnectionId cd(0, 0, 0, 0);
    QVERIFY(!cd.isValid());
    QVERIFY(cd == cd);
    QCOMPARE(cd, cd);
    QCOMPARE(cd, ConnectionId(0, 0, 0, 0));
}

void TestConnection::dataJson()
{
    ConnectionId cd1(0, 0, 0, 0);
    QCOMPARE(cd1.toJson().count(), 4);
    QVERIFY(!ConnectionId::fromJson({}));
    QVERIFY(ConnectionId::fromJson(cd1.toJson()));
    QVERIFY(ConnectionId::fromJson(cd1.toJson()).value() == cd1);

    ConnectionId cd2(1, 2, 3, 4);
    QVERIFY(cd1.toJson() != cd2.toJson());

    ConnectionId cd3(1, 2, 3, 4);
    QCOMPARE(cd2, cd3);
    QVERIFY(ConnectionId::fromJson(cd2.toJson()).value() == cd2);

    QCOMPARE(ConnectionId::fromJson(QJsonObject {}).value(), ConnectionId(0, 0, 0, 0));
}

void TestConnection::dataHash()
{
    QCOMPARE(qHash(ConnectionId(0, 0, 0, 0)), qHash(ConnectionId(0, 0, 0, 0)));
    QVERIFY(qHash(ConnectionId(0, 0, 0, 0)) != qHash(ConnectionId(1, 0, 0, 0)));
    QVERIFY(qHash(ConnectionId(0, 0, 0, 0)) != qHash(ConnectionId(0, 1, 0, 0)));
    QVERIFY(qHash(ConnectionId(0, 0, 0, 0)) != qHash(ConnectionId(0, 0, 1, 0)));
    QVERIFY(qHash(ConnectionId(0, 0, 0, 0)) != qHash(ConnectionId(0, 0, 0, 1)));
}

void TestConnection::testConnection()
{
    Connection conn(ConnectionId(1, 2, 3, 4));
    QCOMPARE(conn.connectionId(), ConnectionId(1, 2, 3, 4));
    QCOMPARE(conn, ConnectionId(1, 2, 3, 4));
    QVERIFY(!(conn == ConnectionId(1, 2, 3, 1)));

    QCOMPARE(conn.sourceInfo(), XletInfo(1, 2, XletType::Out));
    QCOMPARE(conn.destinationInfo(), XletInfo(3, 4, XletType::In));
}

void TestConnection::testRemoveRequested()
{
    DiagramScene ds(100, 100);
    auto d0 = ds.addSceneItem(SharedItemData(new ItemData(1)));
    auto d0_data = d0->itemData();
    d0_data->appendOutput({});
    d0->setItemData(d0_data);

    auto d1 = ds.addSceneItem(SharedItemData(new ItemData(2)));
    auto d1_data = d1->itemData();
    d1_data->setPos({ 200, 200 });
    d1_data->appendInput({});
    d1->setItemData(d1_data);

    QSignalSpy scene_changed(&ds, &DiagramScene::sceneChanged);
    QSignalSpy conn_added(&ds, &DiagramScene::connectionAdded);
    auto res = ds.connectDevices({ 1, 0, 2, 0 }, {});

    QVERIFY(res);
    QCOMPARE(scene_changed.count(), 1);
    QCOMPARE(conn_added.count(), 1);

    auto conn = ds.connections()->findById({ 1, 0, 2, 0 });
    QVERIFY(conn);
    QCOMPARE(conn->mapRectToScene(conn->boundingRect()).toRect(), QRect(-1, 14, 2, 41));

    // QTest::mousePress(&ds, Qt::LeftButton, {}, ds.mapFromScene(QPoint { 0, 30 }));
}

void TestConnection::findConnected()
{
    QGraphicsScene sc;
    auto d1 = new DeviceItem(SharedItemData { new ItemData(100) });
    auto data = d1->itemData();
    data->appendInput({});
    data->appendOutput({});
    d1->setItemData(data);

    d1->setPos(10, 0);
    sc.addItem(d1);
    auto d2 = new DeviceItem(SharedItemData { new ItemData(200) });
    data = d2->itemData();
    data->appendInput({});
    data->appendOutput({});
    d2->setItemData(data);
    d2->setPos(10, 100);
    sc.addItem(d2);

    QCOMPARE(d1->id(), 100);
    QCOMPARE(d2->id(), 200);

    auto conn = new Connection({ 100, 0, 200, 0 });
    sc.addItem(conn);

    QCOMPARE(conn->boundingRect(), QRect());

    auto conn2 = new Connection({ 101, 0, 200, 0 });
    sc.addItem(conn2);

    auto conn3 = new Connection({ 100, 0, 201, 0 });
    sc.addItem(conn3);

    auto conn4 = new Connection({ 101, 0, 201, 0 });
    sc.addItem(conn4);
}

void TestConnection::makeSegments()
{
    Connection c0({ 100, 0, 200, 0 });
    c0.setPoints({ 100, 0 }, { 150, 500 });

    auto segs = c0.viewData().makeSegments();
    QVERIFY(!segs.isEmpty());
    QCOMPARE(segs.size(), 1);
    QCOMPARE(segs.pointAt(0), QPoint(150, 250));

    c0.setPoints({ 150, 500 }, { 100, 0 });

    segs = c0.viewData().makeSegments();
    QVERIFY(!segs.isEmpty());
    QCOMPARE(segs.size(), 2);
    QCOMPARE(segs.pointAt(0), QPoint(125, 505));
    QCOMPARE(segs.pointAt(1), QPoint(125, -25));

    segs.clear();
    QVERIFY(segs.isEmpty());
    QCOMPARE(segs.size(), 0);
}
