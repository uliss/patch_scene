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
#include "test_scene_devices.h"
#include "device.h"
#include "scene_devices.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

using namespace ceam;

namespace {

SharedDeviceData data0(DeviceId id)
{
    auto data = new DeviceData(id);
    return SharedDeviceData { data };
}

SharedDeviceData data1(DeviceId id)
{
    auto data = new DeviceData(id);
    data->setShowTitle(false);
    return SharedDeviceData { data };
}

SharedDeviceData data2(DeviceId id)
{
    auto data = new DeviceData(id);
    data->setShowTitle(false);
    data->setTitle("DATA2");
    data->setVendor("VENDOR2");
    data->setModel("MODEL2");
    return SharedDeviceData { data };
}

SharedDeviceData data3(DeviceId id)
{
    auto data = new DeviceData(id);
    data->setTitle("DATA3");
    data->setVendor("VENDOR3");
    data->setModel("MODEL3");
    return SharedDeviceData { data };
}

SharedDeviceData data4(DeviceId id)
{
    auto data = new DeviceData(id);
    data->setTitle("DATA4");
    data->appendInput(XletData {});
    data->appendOutput(XletData {});
    return SharedDeviceData { data };
}

QList<DeviceId> sorted(const QList<DeviceId>& l)
{
    auto res = l;
    std::sort(res.begin(), res.end());
    return res;
}

template <class T>
QList<T> list(std::initializer_list<T> args) { return QList<T>(args); }

QList<DeviceId> id_list(std::initializer_list<DeviceId> args) { return list<DeviceId>(args); }
}

void TestSceneDevices::initTestCase()
{
    qRegisterMetaType<SharedDeviceData>();
}

void TestSceneDevices::add()
{
    SceneDevices dev;
    QSignalSpy sig_spy(&dev, SIGNAL(added(SharedDeviceData)));
    QVERIFY(sig_spy.isValid());
    QCOMPARE(sig_spy.count(), 0);

    QCOMPARE(dev.count(), 0);
    QVERIFY(!dev.add({}));
    QVERIFY(!dev.add(Device::defaultDeviceData()));
    QCOMPARE(dev.count(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    dev.setScene(&scene);
    QCOMPARE(dev.count(), 0);
    QCOMPARE(scene.items().size(), 0);

    QVERIFY(!dev.add({}));
    QVERIFY(!dev.find(100));
    QCOMPARE(scene.items().size(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QVERIFY(dev.add(data1(100)));
    QCOMPARE(scene.items().size(), 1);
    QCOMPARE(dev.count(), 1);
    QCOMPARE(dev.idList(), id_list({ 100 }));
    QCOMPARE(sig_spy.count(), 1);

    QVERIFY(dev.add(data2(100)));
    QCOMPARE(dev.count(), 2);
    QCOMPARE(scene.items().size(), 2);
    QVERIFY(dev.find(100));
    QCOMPARE(sig_spy.count(), 2);

    dev.clear();
    QCOMPARE(dev.count(), 0);
    QCOMPARE(scene.items().size(), 0);
    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(dev.add(data1(102)));
    QCOMPARE(dev.count(), 3);
    QCOMPARE(scene.items().size(), 3);
    QCOMPARE(sorted(dev.idList()), id_list({ 100, 101, 102 }));
    QCOMPARE(sig_spy.count(), 5);

    QVERIFY(dev.find(100));
    QCOMPARE(dev.find(100)->id(), 100);
    QVERIFY(dev.find(101));
    QCOMPARE(dev.find(101)->id(), 101);
    QVERIFY(dev.find(102));
    QCOMPARE(dev.find(102)->id(), 102);
    QVERIFY(!dev.find(103));
}

void TestSceneDevices::remove()
{
    SceneDevices dev;
    QSignalSpy sig_spy(&dev, SIGNAL(removed(SharedDeviceData)));
    QVERIFY(sig_spy.isValid());

    QVERIFY(!dev.remove(DEV_NULL_ID));
    QVERIFY(!dev.remove(100));
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    dev.setScene(&scene);
    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(!dev.hasSelected());

    QVERIFY(dev.find(100));
    QCOMPARE(dev.find(100)->id(), 100);
    QVERIFY(dev.find(101));
    QCOMPARE(dev.find(101)->id(), 101);

    QVERIFY(dev.remove(100));
    QCOMPARE(dev.idList(), id_list({ 101 }));
    QCOMPARE(sig_spy.count(), 1);
    QCOMPARE(qvariant_cast<SharedDeviceData>(sig_spy.at(0).at(0))->id(), 100);
    QVERIFY(dev.remove(101));
    QCOMPARE(dev.idList(), id_list({}));
    QCOMPARE(sig_spy.count(), 2);
    QCOMPARE(qvariant_cast<SharedDeviceData>(sig_spy.at(1).at(0))->id(), 101);
    QVERIFY(!dev.remove(102));
    QCOMPARE(sig_spy.count(), 2);
}

void TestSceneDevices::clear()
{
    SceneDevices dev;
    QSignalSpy sig_spy(&dev, SIGNAL(removed(SharedDeviceData)));
    QVERIFY(sig_spy.isValid());

    QGraphicsScene scene;
    dev.setScene(&scene);

    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(dev.add(data1(102)));

    dev.clear();

    QCOMPARE(sig_spy.count(), 3);
}

void TestSceneDevices::setSelected()
{
    SceneDevices dev;
    QGraphicsScene scene;
    dev.setScene(&scene);
    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(dev.add(data1(102)));

    dev.setSelected({ 100 }, true);
    QVERIFY(dev.hasSelected());
    QCOMPARE(dev.selectedIdList(), id_list({ 100 }));

    dev.setSelected({ 100, 102 }, true);
    QVERIFY(dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({ 100, 102 }));

    dev.setSelected({ 100 }, false);
    QVERIFY(dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({ 102 }));

    dev.setSelected({ 102 }, false);
    QVERIFY(!dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({}));
}

void TestSceneDevices::toggleSelected()
{
    SceneDevices dev;
    QGraphicsScene scene;
    dev.setScene(&scene);
    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(dev.add(data1(102)));

    dev.toggleSelected({});
    QVERIFY(!dev.hasSelected());

    dev.toggleSelected(dev.idList());
    QVERIFY(dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({ 100, 101, 102 }));

    dev.toggleSelected(dev.idList());
    QVERIFY(!dev.hasSelected());

    dev.toggleSelected({ 100, 101, 102 });
    QVERIFY(dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({ 100, 101, 102 }));
    dev.toggleSelected({ 100, 101 });
    QVERIFY(dev.hasSelected());
    QCOMPARE(sorted(dev.selectedIdList()), id_list({ 102 }));
}

void TestSceneDevices::boundingRect()
{
    SceneDevices dev;
    QCOMPARE(dev.boundingRect(), QRectF {});

    QGraphicsScene scene;
    dev.setScene(&scene);
    auto dev1 = dev.add(data3(100));
    QCOMPARE(dev1->deviceData()->title(), "DATA3");
    QCOMPARE(dev.boundingRect(), QRectF(-40, 0, 80, 24));

    auto dev2 = dev.add(data3(100));
    dev2->setPos(0, 100);
    QCOMPARE(dev.boundingRect(), QRectF(-40, 0, 80, 124));
}

void TestSceneDevices::findConnectionInfo()
{
    SceneDevices dev;
    auto info = dev.connectionInfo(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(!info);

    QGraphicsScene scene;
    dev.setScene(&scene);

    info = dev.connectionInfo(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(!info);

    QVERIFY(dev.add(data4(100)));
    info = dev.connectionInfo(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(!info);

    QVERIFY(dev.add(data4(101)));
    info = dev.connectionInfo(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(info);
    QCOMPARE(info->src_data->id(), 100);
    QCOMPARE(info->dest_data->id(), 101);

    info = dev.connectionInfo(ConnectionData { 101, 0, 100, 0 });
    QVERIFY(info);
    QCOMPARE(info->src_data->id(), 101);
    QCOMPARE(info->dest_data->id(), 100);
}

void TestSceneDevices::findConnectionPair()
{
    SceneDevices dev;
    auto pair = dev.connectionPair(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(!pair);

    QGraphicsScene scene;
    dev.setScene(&scene);

    auto data = data0(100);
    data->setId(100);
    data->appendInput(XletData::createSocket(ConnectorModel::XLR, true));
    data->appendOutput(XletData::createSocket(ConnectorModel::XLR, false));
    QVERIFY(dev.add(data));
    data->setId(101);
    QVERIFY(dev.add(data));
    data->setId(102);
    QVERIFY(dev.add(data));
    data->setId(103);
    QVERIFY(dev.add(data));

    pair = dev.connectionPair(ConnectionData { 100, 0, 101, 0 });
    QVERIFY(pair);
    QCOMPARE(pair->p0.model, ConnectorModel::XLR);
    QCOMPARE(pair->p0.type, ConnectorType::plug_female);
    QCOMPARE(pair->p1.model, ConnectorModel::XLR);
    QCOMPARE(pair->p1.type, ConnectorType::plug_male);

    {
        QBENCHMARK_ONCE(dev.connectionPair(ConnectionData { 100, 0, 101, 0 }));
    }
}

void TestSceneDevices::checkConnection()
{
    SceneDevices dev;
    QVERIFY(!dev.checkConnection({ 0, 0, 0, 0 }));

    QGraphicsScene scene;
    dev.setScene(&scene);

    dev.add(data4(100));
    dev.add(data4(101));

    QVERIFY(dev.checkConnection({ 100, 0, 101, 0 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));

    QVERIFY(!dev.checkConnection({ 100, 0, 100, 0 }));
    QVERIFY(!dev.checkConnection({ 100, 0, 102, 0 }));
    QVERIFY(!dev.checkConnection({ 102, 0, 100, 0 }));

    QVERIFY(!dev.checkConnection({ 101, 1, 100, 0 }));
    QVERIFY(!dev.checkConnection({ 101, 0, 100, 1 }));
    QVERIFY(!dev.checkConnection({ 101, 1, 100, 1 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
}

void TestSceneDevices::findConnectionPoints()
{
    SceneDevices dev;
    QVERIFY(!dev.connectionPoints({ 0, 0, 0, 0 }));

    QGraphicsScene scene;
    dev.setScene(&scene);

    auto dev1 = dev.add(data1(100));
    auto dev2 = dev.add(data1(101));
    dev1->setPos(100, 200);
    dev2->setPos(300, 400);

    auto pts = dev.connectionPoints({ 100, 0, 101, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF {});
    QCOMPARE(pts->second, QPointF {});

    pts = dev.connectionPoints({ 101, 0, 100, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF {});
    QCOMPARE(pts->second, QPointF {});

    dev.clear();

    dev1 = dev.add(data4(100));
    dev2 = dev.add(data4(101));
    dev1->setPos(100, 200);
    dev2->setPos(300, 400);

    pts = dev.connectionPoints({ 100, 0, 101, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF(100, 264));
    QCOMPARE(pts->second, QPointF(300, 424));

    pts = dev.connectionPoints({ 101, 0, 100, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF(300, 464));
    QCOMPARE(pts->second, QPointF(100, 224));
}

void TestSceneDevices::toJson()
{
    SceneDevices dev;
    QGraphicsScene scene;
    dev.setScene(&scene);

    auto dev1 = dev.add(data1(100));
    auto dev2 = dev.add(data1(101));
    dev1->setPos(100, 200);
    dev2->setPos(300, 400);

    auto json = dev.toJson();
    QVERIFY(json.isArray());
    QCOMPARE(json.toArray().count(), 2);
    auto arr = json.toArray();
    auto o1 = arr[0].toObject();
    auto o2 = arr[1].toObject();

    QCOMPARE(o1["zoom"].toDouble(), 1);
    QCOMPARE(o2["zoom"].toDouble(), 1);
    QCOMPARE(o1["category"].toString(), "device");
    QCOMPARE(o2["category"].toString(), "device");
}

void TestSceneDevices::compare()
{
    SceneDevices sc;
    QCOMPARE(sc, sc);
    QVERIFY(!(sc != sc));
}
