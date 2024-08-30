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
#include "test_connection_database.h"
#include "connection_database.h"

using namespace ceam;

#include <QTest>

void TestConnectionDatabase::jackPair()
{
    auto j0 = ConnectionEndPoint { ConnectorModel::XLR, ConnectorType::plug_male };
    auto j1 = ConnectionEndPoint { ConnectorModel::XLR, ConnectorType::plug_female };

    auto jp00 = ConnectionPair { j0, j0 };
    auto jp01 = ConnectionPair { j0, j1 };
    auto jp10 = ConnectionPair { j1, j0 };
    auto jp11 = ConnectionPair { j1, j1 };

    QCOMPARE(jp00, jp00);
    QCOMPARE(jp01, jp01);
    QCOMPARE(jp10, jp10);
    QCOMPARE(jp11, jp11);

    QCOMPARE(jp01, jp10);
    QCOMPARE(jp10, jp01);
}

void TestConnectionDatabase::construct()
{
    ConnectionDatabase db;
    QCOMPARE(db.count(), 0);
}

void TestConnectionDatabase::add()
{
    ConnectionDatabase db;
    QCOMPARE(db.count(), 0);

    auto j0 = ConnectionEndPoint { ConnectorModel::XLR, ConnectorType::plug_male };
    auto j1 = ConnectionEndPoint { ConnectorModel::XLR, ConnectorType::plug_female };

    auto jp00 = ConnectionPair { j0, j0 };
    auto jp01 = ConnectionPair { j0, j1 };
    auto jp10 = ConnectionPair { j1, j0 };
    auto jp11 = ConnectionPair { j1, j1 };

    QVERIFY(!db.contains(jp00));
    QVERIFY(!db.contains(jp01));
    QVERIFY(!db.contains(jp10));
    QVERIFY(!db.contains(jp11));

    QVERIFY(db.add(jp00, ConnectionStyle::Audio));
    QVERIFY(db.add(jp01, ConnectionStyle::Power));
    QVERIFY(db.add(jp11, ConnectionStyle::Network));

    QVERIFY(db.contains(jp00));
    QVERIFY(db.contains(jp01));
    QVERIFY(db.contains(jp10));
    QVERIFY(db.contains(jp11));

    QCOMPARE(db.search(jp00), ConnectionStyle::Audio);
    QCOMPARE(db.search(jp01), ConnectionStyle::Power);
    QCOMPARE(db.search(jp10), ConnectionStyle::Power);
    QCOMPARE(db.search(jp11), ConnectionStyle::Network);
}

void TestConnectionDatabase::exists()
{

}
