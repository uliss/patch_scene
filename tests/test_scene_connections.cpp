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
#include <QSignalSpy>
#include <QTest>

void TestSceneConnections::add()
{
    SceneConnections sc;
    QSignalSpy sig_spy(&sc, SIGNAL(added(ConnectionData)));
    QVERIFY(sig_spy.isValid());
    QVERIFY(!sc.add(ConnectionData { 0, 0, 0, 0 }));
    QCOMPARE(sc.count(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 0, 0 }));
    QCOMPARE(sc.count(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 0, 1, 1 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionData { 0, 1, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(sc.add(ConnectionData { 0, 1, 1, 1 }));
    QCOMPARE(sc.count(), 2);
    QCOMPARE(sig_spy.count(), 2);

    QVERIFY(sc.add(ConnectionData { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 3);
    QCOMPARE(sig_spy.count(), 3);
}

void TestSceneConnections::remove()
{
    SceneConnections sc;
    QSignalSpy sig_spy(&sc, SIGNAL(removed(ConnectionData)));
    QVERIFY(sig_spy.isValid());
    QVERIFY(!sc.remove(XletInfo { 0, 0, XletType::In }));
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QVERIFY(sc.add(ConnectionData { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 2);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(!sc.remove({ 0, 1, XletType::In }));
    QVERIFY(!sc.remove({ 0, 1, XletType::None }));
    QVERIFY(!sc.remove({ 0, 1, XletType::Out }));
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.remove({ 0, 0, XletType::Out }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.remove({ 0, 0, XletType::Out }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(sc.remove({ 0, 0, XletType::In }));
    QCOMPARE(sc.count(), 0);
    QCOMPARE(sig_spy.count(), 2);
}

void TestSceneConnections::clear()
{
    SceneConnections sc;
    QSignalSpy sig_spy(&sc, SIGNAL(removed(ConnectionData)));
    QVERIFY(sig_spy.isValid());
    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionData { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 2);

    sc.clear();
    QCOMPARE(sig_spy.count(), 2);
}

void TestSceneConnections::removeAll()
{
    SceneConnections sc;
    QSignalSpy sig_spy(&sc, SIGNAL(removed(ConnectionData)));
    QVERIFY(sig_spy.isValid());
    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionData { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionData { 0, 2, 1, 2 }));
    QVERIFY(sc.add(ConnectionData { 2, 0, 0, 3 }));
    QVERIFY(sc.add(ConnectionData { 3, 1, 1, 3 }));
    QVERIFY(sc.add(ConnectionData { 3, 2, 1, 4 }));
    QCOMPARE(sc.count(), 6);

    sc.removeAll(2);
    QCOMPARE(sc.count(), 5);
    QCOMPARE(sig_spy.count(), 1);

    sc.removeAll(3);
    QCOMPARE(sc.count(), 3);
    QCOMPARE(sig_spy.count(), 3);

    sc.removeAll(0);
    QCOMPARE(sc.count(), 0);
    QCOMPARE(sig_spy.count(), 6);
}

void TestSceneConnections::findConnection()
{
    SceneConnections sc;
    QGraphicsScene scene;
    sc.setScene(&scene);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QVERIFY(sc.findConnection(XletInfo::outlet(0, 0)));
    QVERIFY(sc.findConnection(XletInfo::inlet(1, 0)));
    QVERIFY(!sc.findConnection(XletInfo::outlet(1, 0)));
    QVERIFY(!sc.findConnection(XletInfo::inlet(0, 0)));
}

void TestSceneConnections::setVisible()
{
    SceneConnections sc;
    QGraphicsScene scene;
    sc.setScene(&scene);
    QSignalSpy sig_spy(&sc, SIGNAL(visibleChanged(bool)));
    QVERIFY(sig_spy.isValid());

    sc.setVisible(true);
    sc.setVisible(false);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.add(ConnectionData { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionData { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionData { 0, 2, 1, 2 }));

    sc.setVisible(true);
    QCOMPARE(sig_spy.count(), 0);

    sc.setVisible(false);
    QCOMPARE(sig_spy.count(), 1);
    QCOMPARE(sig_spy.takeLast().at(0).toBool(), false);

    sc.setVisible(true);
    QCOMPARE(sig_spy.count(), 1);
    QCOMPARE(sig_spy.takeLast().at(0).toBool(), true);
}

static TestSceneConnections test_scene_connections;
