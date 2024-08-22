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
    ConnectionData cd(0, 0, 0, 0);
    QVERIFY(!ConnectionData::fromJson({}, cd));
    auto js = cd.toJson();
    QVERIFY(ConnectionData::fromJson(cd.toJson(), cd));

    ConnectionData cd2(1, 2, 3, 4);
    QVERIFY(cd.toJson() != cd2.toJson());
    ConnectionData cd3(1, 2, 3, 4);
    QCOMPARE(cd2, cd3);
    QVERIFY(cd3.toJson() == cd2.toJson());

    QVERIFY(ConnectionData::fromJson(cd3.toJson(), cd));
    QCOMPARE(cd3, cd);
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
    QCOMPARE_EQ(conn.source(), 1);
    QCOMPARE_EQ(conn.sourceOutput(), 2);
    QCOMPARE_EQ(conn.destination(), 3);
    QCOMPARE_EQ(conn.destinationInput(), 4);
    QVERIFY(!conn.checkConnectedElements());
}

static TestConnection conn;
