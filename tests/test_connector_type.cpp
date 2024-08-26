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
#include "test_connector_type.h"
#include "connector_type.h"

#include <QTest>

using namespace ceam;

void TestConnectorType::testInit()
{
    ConnectorType ct;
    QVERIFY(ct.isSocket());
    QVERIFY(ct.toJsonString() == "socket_female");
    QVERIFY(ct.toJson().isString());

    QVERIFY(ConnectorType::plug_female.isPlug());
    QVERIFY(ConnectorType::plug_male.isPlug());
    QVERIFY(!ConnectorType::socket_female.isPlug());
    QVERIFY(!ConnectorType::socket_male.isPlug());

    QVERIFY(!ConnectorType::plug_female.isSocket());
    QVERIFY(!ConnectorType::plug_male.isSocket());
    QVERIFY(ConnectorType::socket_female.isSocket());
    QVERIFY(ConnectorType::socket_male.isSocket());

    QVERIFY(ConnectorType::fromJson(ConnectorType::plug_female.toJson()));
    QVERIFY(ConnectorType::fromJson(ConnectorType::plug_male.toJson()));
    QVERIFY(ConnectorType::fromJson(ConnectorType::socket_female.toJson()));
    QVERIFY(ConnectorType::fromJson(ConnectorType::socket_male.toJson()));

    QCOMPARE(ConnectorType::fromJson(ConnectorType::plug_female.toJson()).value(), ConnectorType::plug_female);
    QCOMPARE(ConnectorType::fromJson(ConnectorType::plug_male.toJson()), ConnectorType::plug_male);
    QCOMPARE(ConnectorType::fromJson(ConnectorType::socket_female.toJson()), ConnectorType::socket_female);
    QCOMPARE(ConnectorType::fromJson(ConnectorType::socket_male.toJson()), ConnectorType::socket_male);

    QCOMPARE(ConnectorType::plug_female.complement(), ConnectorType::socket_male);
    QCOMPARE(ConnectorType::plug_male.complement(), ConnectorType::socket_female);
    QCOMPARE(ConnectorType::socket_female.complement(), ConnectorType::plug_male);
    QCOMPARE(ConnectorType::socket_male.complement(), ConnectorType::plug_female);

    ConnectorType::foreachType([](ConnectorType t) {
        QCOMPARE(t, t.complement().complement());
    });
}

static TestConnectorType test_connector_type;
