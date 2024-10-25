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
    QGraphicsScene scene;
    SceneConnections sc(&scene);
    QSignalSpy sig_spy(&sc, &SceneConnections::added);
    QVERIFY(sig_spy.isValid());

    QVERIFY(!sc.add(ConnectionId { 0, 0, 0, 0 }));
    QCOMPARE(sc.count(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionId { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionId { 0, 0, 1, 1 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(!sc.add(ConnectionId { 0, 1, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(sc.add(ConnectionId { 0, 1, 1, 1 }));
    QCOMPARE(sc.count(), 2);
    QCOMPARE(sig_spy.count(), 2);

    QVERIFY(sc.add(ConnectionId { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 3);
    QCOMPARE(sig_spy.count(), 3);
}

void TestSceneConnections::remove()
{
    QGraphicsScene scene;
    SceneConnections sc(&scene);
    QSignalSpy sig_spy(&sc, &SceneConnections::removed);
    QVERIFY(sig_spy.isValid());
    QVERIFY(!sc.remove(XletInfo { 0, 0, XletType::In }));
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QCOMPARE(sc.count(), 1);
    QVERIFY(sc.add(ConnectionId { 1, 0, 0, 0 }));
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
    QGraphicsScene scene;
    SceneConnections sc(&scene);
    QSignalSpy sig_spy(&sc, &SceneConnections::removed);
    QVERIFY(sig_spy.isValid());

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionId { 1, 0, 0, 0 }));
    QCOMPARE(sc.count(), 2);

    sc.clear();
    QCOMPARE(sig_spy.count(), 2);
}

void TestSceneConnections::removeAll()
{
    QGraphicsScene scene;
    SceneConnections sc(&scene);
    QSignalSpy sig_spy(&sc, &SceneConnections::removed);
    QVERIFY(sig_spy.isValid());

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionId { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionId { 0, 2, 1, 2 }));
    QVERIFY(sc.add(ConnectionId { 2, 0, 0, 3 }));
    QVERIFY(sc.add(ConnectionId { 3, 1, 1, 3 }));
    QVERIFY(sc.add(ConnectionId { 3, 2, 1, 4 }));
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
    QGraphicsScene scene;
    SceneConnections sc(&scene);

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionId { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionId { 0, 2, 1, 2 }));
    QVERIFY(sc.add(ConnectionId { 2, 0, 0, 3 }));
    QVERIFY(sc.add(ConnectionId { 3, 1, 1, 3 }));
    QVERIFY(sc.add(ConnectionId { 3, 2, 1, 4 }));
    QCOMPARE(sc.count(), 6);

    auto conns = sc.findConnectionsData(0);
    std::sort(conns.begin(), conns.end(), [](const ConnectionInfo& a, const ConnectionInfo& b) {
        return a.first.destination() < b.first.destination();
    });
    QCOMPARE(conns.size(), 4);
    QCOMPARE(conns[0].first, ConnectionId(2, 0, 0, 3));
    QCOMPARE(conns[1].first, ConnectionId(0, 0, 1, 0));
    QCOMPARE(conns[2].first, ConnectionId(0, 1, 1, 1));
    QCOMPARE(conns[3].first, ConnectionId(0, 2, 1, 2));
}

void TestSceneConnections::findConnections()
{
    QGraphicsScene scene;
    SceneConnections sc(&scene);

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionId { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionId { 0, 2, 1, 2 }));
}

void TestSceneConnections::setVisible()
{
    QGraphicsScene scene;
    SceneConnections sc(&scene);
    QSignalSpy sig_spy(&sc, SIGNAL(visibleChanged(bool)));
    QVERIFY(sig_spy.isValid());

    sc.setVisible(true);
    sc.setVisible(false);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(sc.add(ConnectionId { 0, 0, 1, 0 }));
    QVERIFY(sc.add(ConnectionId { 0, 1, 1, 1 }));
    QVERIFY(sc.add(ConnectionId { 0, 2, 1, 2 }));

    sc.setVisible(true);
    QCOMPARE(sig_spy.count(), 0);

    sc.setVisible(false);
    QCOMPARE(sig_spy.count(), 1);
    QCOMPARE(sig_spy.takeLast().at(0).toBool(), false);

    sc.setVisible(true);
    QCOMPARE(sig_spy.count(), 1);
    QCOMPARE(sig_spy.takeLast().at(0).toBool(), true);
}

void TestSceneConnections::checkConnection()
{
    QGraphicsScene scene;
    SceneConnections sc(&scene);

    QVERIFY(sc.add(ConnectionId { 100, 0, 101, 0 }));
    QVERIFY(sc.add(ConnectionId { 100, 1, 101, 1 }));

    using P = std::pair<XletInfo, XletData>;
    // self connect
    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::In), {}), P(XletInfo(100, 0, XletType::Out), {})));
    // in to in
    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::In), {}), P(XletInfo(101, 0, XletType::In), {})));
    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::Out), {}), P(XletInfo(101, 0, XletType::Out), {})));

    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::Out), {}), P(XletInfo(101, 0, XletType::In), {})));
    QVERIFY(!sc.checkConnection(P(XletInfo(101, 0, XletType::In), {}), P(XletInfo(100, 0, XletType::Out), {})));

    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::None), {}), P(XletInfo(101, 0, XletType::In), {})));
    QVERIFY(!sc.checkConnection(P(XletInfo(100, 0, XletType::Out), {}), P(XletInfo(101, 0, XletType::None), {})));

    QVERIFY(sc.checkConnection(P(XletInfo(100, 2, XletType::Out), {}), P(XletInfo(101, 2, XletType::In), {})));
    QVERIFY(sc.checkConnection(P(XletInfo(101, 2, XletType::In), {}), P(XletInfo(100, 2, XletType::Out), {})));
}
