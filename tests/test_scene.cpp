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
#include "test_scene.h"
#include "comment_item.h"
#include "scene.h"
#include "scene_item.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

using namespace ceam;

namespace {

constexpr int XW = 22;
constexpr int XH = 20;
constexpr int MIN_TXT_WD = 80;
constexpr int MIN_TITLE_WD = 70;

#ifdef Q_OS_LINUX
constexpr int DEF_TXT_HT = 26;
#else
constexpr int DEF_TXT_HT = 24;
#endif

SharedItemData data0(SceneItemId id)
{
    auto data = new ItemData(id);
    return SharedItemData { data };
}

SharedItemData data1(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setShowTitle(false);
    return SharedItemData { data };
}

SharedItemData data2(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setShowTitle(false);
    data->setTitle("DATA2");
    data->setVendor("VENDOR2");
    data->setModel("MODEL2");
    return SharedItemData { data };
}

SharedItemData data3(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setTitle("DATA3");
    data->setVendor("VENDOR3");
    data->setModel("MODEL3");
    return SharedItemData { data };
}

SharedItemData data4(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setTitle("DATA4");
    data->appendInput(XletData {});
    data->appendOutput(XletData {});
    return SharedItemData { data };
}

SharedItemData data_comment(SceneItemId id)
{
    auto data = new ItemData(id);
    data->setTitle("COMMENT");
    data->setCategory(ItemCategory::Comment);
    return SharedItemData { data };
}

SharedItemData data_n(SceneItemId id, int in = 0, int out = 0, const QString& title = {})
{
    auto data = new ItemData(id);

    data->setTitle(title);
    data->setShowTitle(!title.isEmpty());

    for (int i = 0; i < in; i++)
        data->appendInput(XletData {});

    for (int i = 0; i < out; i++)
        data->appendOutput(XletData {});

    return SharedItemData { data };
}

QList<SceneItemId> sorted(const QList<SceneItemId>& l)
{
    auto res = l;
    std::sort(res.begin(), res.end());
    return res;
}

template <class T>
QList<T> list(std::initializer_list<T> args) { return QList<T>(args); }

QList<SceneItemId> id_list(std::initializer_list<SceneItemId> args) { return list<SceneItemId>(args); }
} // namespace

void TestScene::initTestCase()
{
    qRegisterMetaType<SharedItemData>();
}

void TestScene::add()
{
    Scene dev;
    QSignalSpy sig_spy(&dev, &Scene::added);
    QVERIFY(sig_spy.isValid());
    QCOMPARE(sig_spy.count(), 0);

    QCOMPARE(dev.count(), 0);
    QVERIFY(!dev.add({}));
    QVERIFY(!dev.add(SceneItem::defaultDeviceData()));
    QCOMPARE(dev.count(), 0);
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);
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

    // add comment data
    dev.clear();
    auto c0 = dev.add(data_comment(200));
    QVERIFY(c0);
    QCOMPARE(sig_spy.count(), 6);
    QCOMPARE(c0->deviceData()->title(), "Comment");
    QVERIFY(dynamic_cast<CommentItem*>(c0) != nullptr);
}

void TestScene::addComment()
{
    Scene dev;
    QSignalSpy sig_spy(&dev, &Scene::added);
    QVERIFY(sig_spy.isValid());

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

    QVERIFY(dev.addComment());
    QCOMPARE(dev.count(), 1);
    QCOMPARE(sig_spy.count(), 0);
}

void TestScene::remove()
{
    Scene dev;
    QSignalSpy sig_spy(&dev, &Scene::removed);
    QVERIFY(sig_spy.isValid());

    QVERIFY(!dev.remove(SCENE_ITEM_NULL_ID));
    QVERIFY(!dev.remove(100));
    QCOMPARE(sig_spy.count(), 0);

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);
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
    QCOMPARE(qvariant_cast<SharedItemData>(sig_spy.at(0).at(0))->id(), 100);
    QVERIFY(dev.remove(101));
    QCOMPARE(dev.idList(), id_list({}));
    QCOMPARE(sig_spy.count(), 2);
    QCOMPARE(qvariant_cast<SharedItemData>(sig_spy.at(1).at(0))->id(), 101);
    QVERIFY(!dev.remove(102));
    QCOMPARE(sig_spy.count(), 2);
}

void TestScene::clear()
{
    Scene dev;
    QSignalSpy sig_spy(&dev, &Scene::removed);
    QVERIFY(sig_spy.isValid());

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

    QVERIFY(dev.add(data1(100)));
    QVERIFY(dev.add(data1(101)));
    QVERIFY(dev.add(data1(102)));

    dev.clear();

    QCOMPARE(sig_spy.count(), 3);
}

void TestScene::setSelected()
{
    Scene dev;
    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);
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

void TestScene::toggleSelected()
{
    Scene dev;
    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);
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

void TestScene::boundingRect()
{
    Scene dev;
    QCOMPARE(dev.boundingRect(), QRectF {});

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);
    auto dev1 = dev.add(data3(100));
    QCOMPARE(dev1->deviceData()->title(), "DATA3");
    QCOMPARE(dev.boundingRect(), QRectF(-0.5 * MIN_TXT_WD, 0, MIN_TXT_WD, DEF_TXT_HT));

    auto dev2 = dev.add(data3(100));
    dev2->setPos(0, 100);
    QCOMPARE(dev.boundingRect(), QRectF(-0.5 * MIN_TXT_WD, 0, MIN_TXT_WD, 100 + DEF_TXT_HT));
}

void TestScene::findConnectionInfo()
{
    Scene dev;
    auto info = dev.connectionInfo(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(!info);

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

    info = dev.connectionInfo(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(!info);

    QVERIFY(dev.add(data4(100)));
    info = dev.connectionInfo(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(!info);

    QVERIFY(dev.add(data4(101)));
    info = dev.connectionInfo(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(info);
    QCOMPARE(info->src_data->id(), 100);
    QCOMPARE(info->dest_data->id(), 101);

    info = dev.connectionInfo(ConnectionId { 101, 0, 100, 0 });
    QVERIFY(info);
    QCOMPARE(info->src_data->id(), 101);
    QCOMPARE(info->dest_data->id(), 100);
}

void TestScene::findConnectionPair()
{
    Scene dev;
    auto pair = dev.connectionPair(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(!pair);

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

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

    pair = dev.connectionPair(ConnectionId { 100, 0, 101, 0 });
    QVERIFY(pair);
    QCOMPARE(pair->p0.model, ConnectorModel::XLR);
    QCOMPARE(pair->p0.type, ConnectorType::plug_female);
    QCOMPARE(pair->p1.model, ConnectorModel::XLR);
    QCOMPARE(pair->p1.type, ConnectorType::plug_male);
}

void TestScene::checkConnection()
{
    Scene dev;
    QVERIFY(!dev.checkConnection({ 0, 0, 0, 0 }));

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

    dev.add(data_n(100, 1, 5));
    dev.add(data_n(101, 5, 1));

    QVERIFY(!dev.checkConnection({ 100, 0, 100, 0 }));
    QVERIFY(!dev.checkConnection({ 101, 0, 101, 0 }));

    QVERIFY(dev.checkConnection({ 100, 0, 101, 0 }));
    QVERIFY(dev.checkConnection({ 100, 1, 101, 0 }));
    QVERIFY(dev.checkConnection({ 100, 2, 101, 0 }));
    QVERIFY(dev.checkConnection({ 100, 3, 101, 0 }));
    QVERIFY(dev.checkConnection({ 100, 4, 101, 0 }));
    QVERIFY(!dev.checkConnection({ 100, 5, 101, 0 }));

    QVERIFY(dev.checkConnection({ 100, 0, 101, 4 }));
    QVERIFY(dev.checkConnection({ 100, 1, 101, 4 }));
    QVERIFY(dev.checkConnection({ 100, 2, 101, 4 }));
    QVERIFY(dev.checkConnection({ 100, 3, 101, 4 }));
    QVERIFY(dev.checkConnection({ 100, 4, 101, 4 }));
    QVERIFY(!dev.checkConnection({ 100, 5, 101, 0 }));

    QVERIFY(!dev.checkConnection({ 100, 0, 101, 5 }));
    QVERIFY(!dev.checkConnection({ 100, 1, 101, 5 }));
    QVERIFY(!dev.checkConnection({ 100, 2, 101, 5 }));
    QVERIFY(!dev.checkConnection({ 100, 3, 101, 5 }));
    QVERIFY(!dev.checkConnection({ 100, 4, 101, 5 }));
    QVERIFY(!dev.checkConnection({ 100, 5, 101, 0 }));

    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
    QVERIFY(dev.checkConnection({ 101, 0, 100, 0 }));
    QVERIFY(!dev.checkConnection({ 101, 1, 100, 0 }));
}

void TestScene::findConnectionPoints()
{
    Scene dev;
    QVERIFY(!dev.connectionPoints({ 0, 0, 0, 0 }));

    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

    auto dev1 = dev.add(data1(100));
    auto dev2 = dev.add(data1(101));
    dev1->setPos(100, 200);
    dev2->setPos(300, 400);

    auto pts = dev.connectionPoints({ 100, 0, 101, 0 });
    QVERIFY(!pts);

    pts = dev.connectionPoints({ 101, 0, 100, 0 });
    QVERIFY(!pts);

    dev.clear();

    dev1 = dev.add(data4(100));
    dev2 = dev.add(data4(101));
    dev1->setPos(100, 200);
    dev2->setPos(300, 400);
    QCOMPARE(dev1->deviceData()->title(), "DATA4");

    pts = dev.connectionPoints({ 100, 0, 101, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF(100, 240 + DEF_TXT_HT));
    QCOMPARE(pts->second, QPointF(300, 400 + DEF_TXT_HT));

    pts = dev.connectionPoints({ 101, 0, 100, 0 });
    QVERIFY(pts);
    QCOMPARE(pts->first, QPointF(300, 440 + DEF_TXT_HT));
    QCOMPARE(pts->second, QPointF(100, 200 + DEF_TXT_HT));
}

void TestScene::toJson()
{
    Scene dev;
    QGraphicsScene scene;
    dev.setGraphicsScene(&scene);

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

void TestScene::compare()
{
    Scene sc;
    QCOMPARE(sc, sc);
    QVERIFY(!(sc != sc));
}
