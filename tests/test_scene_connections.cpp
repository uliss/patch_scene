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
#include "test_scene_connections.h"
#include "scene_connections.h"

using namespace ceam;

#include <QGraphicsScene>
#include <QTest>

void TestSceneConnections::add()
{
    SceneConnections sc;
    QVERIFY(!sc.add(ConnectionData { 0, 0, 0, 0 }));
    QCOMPARE(sc.count(), 0);

    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 0, 0 }));
    QCOMPARE(sc.count(), 0);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 1, 1 }));
    QCOMPARE(sc.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 1, 1, 0 }));
    QCOMPARE(sc.count(), 1);

    QVERIFY(sc.add(ConnectionData { 0, 1, 1, 1 }));
    QCOMPARE(sc.count(), 2);

    QVERIFY(sc.add(ConnectionData { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 3);
}

static TestSceneConnections test_scene_connections;