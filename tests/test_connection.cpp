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
    ConnectionData cd(0, 0, 0, 0);
    QVERIFY(!cd.isValid());
    QVERIFY(cd.isSameDestimation(cd));
    QVERIFY(cd.isSameSource(cd));
    QVERIFY(cd == cd);
    QCOMPARE(cd, cd);
    QCOMPARE(cd, ConnectionData(0, 0, 0, 0));
}

void TestConnection::dataJson()
{
    ConnectionData cd1(0, 0, 0, 0);
    QCOMPARE(cd1.toJson().count(), 4);
    QVERIFY(!ConnectionData::fromJson({}));
    QVERIFY(ConnectionData::fromJson(cd1.toJson()));
    QVERIFY(ConnectionData::fromJson(cd1.toJson()).value() == cd1);

    ConnectionData cd2(1, 2, 3, 4);
    QVERIFY(cd1.toJson() != cd2.toJson());

    ConnectionData cd3(1, 2, 3, 4);
    QCOMPARE(cd2, cd3);
    QVERIFY(ConnectionData::fromJson(cd2.toJson()).value() == cd2);
}

void TestConnection::dataHash()
{
    QCOMPARE_EQ(qHash(ConnectionData(0, 0, 0, 0)), qHash(ConnectionData(0, 0, 0, 0)));
    QCOMPARE_NE(qHash(ConnectionData(0, 0, 0, 0)), qHash(ConnectionData(1, 0, 0, 0)));
    QCOMPARE_NE(qHash(ConnectionData(0, 0, 0, 0)), qHash(ConnectionData(0, 1, 0, 0)));
    QCOMPARE_NE(qHash(ConnectionData(0, 0, 0, 0)), qHash(ConnectionData(0, 0, 1, 0)));
    QCOMPARE_NE(qHash(ConnectionData(0, 0, 0, 0)), qHash(ConnectionData(0, 0, 0, 1)));
}

void TestConnection::testConnection()
{
    Connection conn(ConnectionData(1, 2, 3, 4));
    QCOMPARE_EQ(conn.connectionData(), ConnectionData(1, 2, 3, 4));
    QVERIFY(conn.relatesToDevice(1));
    QVERIFY(!conn.relatesToDevice(2));
    QVERIFY(conn.relatesToDevice(3));
    QVERIFY(!conn.relatesToDevice(4));
    QCOMPARE_EQ(conn, ConnectionData(1, 2, 3, 4));
    QVERIFY(!(conn == ConnectionData(1, 2, 3, 1)));

    QVERIFY(!conn.checkConnectedElements());
    QVERIFY(!conn.updateCachedPos());

    QVERIFY(!conn.findConnectedElements());
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
    QVERIFY(!conn->checkConnectedElements());
    sc.addItem(conn);

    QVERIFY(conn->checkConnectedElements());
    auto x = conn->findConnectedElements();
    QVERIFY(x);
    QCOMPARE(x->first->id(), 100);
    QCOMPARE(x->second->id(), 200);
    QCOMPARE(conn->boundingRect(), QRect());
    conn->updateCachedPos();
    QCOMPARE(conn->boundingRect(), QRectF(10, 64.5, 0, 60));

    auto conn2 = new Connection({ 101, 0, 200, 0 });
    sc.addItem(conn2);
    QVERIFY(!conn2->checkConnectedElements());

    auto conn3 = new Connection({ 100, 0, 201, 0 });
    sc.addItem(conn3);
    QVERIFY(!conn3->checkConnectedElements());

    auto conn4 = new Connection({ 101, 0, 201, 0 });
    sc.addItem(conn4);
    QVERIFY(!conn4->checkConnectedElements());
}

static TestConnection conn;
