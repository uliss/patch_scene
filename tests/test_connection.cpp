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
#include "device.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QJsonValue>
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

void TestConnection::findConnected()
{
    QGraphicsScene sc;
    auto d1 = new Device(SharedDeviceData { new DeviceData(100) });
    auto data = d1->deviceData();
    data->appendInput({});
    data->appendOutput({});
    d1->setDeviceData(data);

    d1->setPos(10, 0);
    sc.addItem(d1);
    auto d2 = new Device(SharedDeviceData { new DeviceData(200) });
    data = d2->deviceData();
    data->appendInput({});
    data->appendOutput({});
    d2->setDeviceData(data);
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
    QCOMPARE(segs.size(), 2);
    QCOMPARE(segs.at(0), 250);
    QCOMPARE(segs.at(1), 50);
    QCOMPARE(segs.pointAt(-1, {}), std::nullopt);
    QCOMPARE(segs.pointAt(2, {}), std::nullopt);

    using OptPoint = std::optional<QPointF>;

    QCOMPARE(segs.pointAt(0, { 0, 0 }), OptPoint(QPointF(0, 250)));
    QCOMPARE(segs.pointAt(1, { 0, 0 }), OptPoint(QPointF(50, 250)));

    QCOMPARE(segs.pointAt(0, { 10, 20 }), OptPoint(QPointF(10, 270)));
    QCOMPARE(segs.pointAt(1, { 10, 20 }), OptPoint(QPointF(60, 270)));
}
