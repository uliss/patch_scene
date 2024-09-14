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
#include "test_scene_background.h"
#include "scene_background.h"

#include <QGraphicsScene>
#include <QJsonValue>
#include <QSignalSpy>
#include <QTest>

using namespace ceam;

void TestSceneBackground::construct()
{
    SceneBackground img;
    QVERIFY(img.isEmpty());

    QSignalSpy spy(&img, SIGNAL(backgroundChanged()));

    img.clear();
    QVERIFY(img.toJson().isNull());
    QCOMPARE(spy.count(), 0);
}

void TestSceneBackground::pixmapTest()
{
    SceneBackground img;
    QSignalSpy spy(&img, SIGNAL(backgroundChanged()));

    QVERIFY(!img.loadImage(":/ceam_logo_color.jpg"));
    QCOMPARE(spy.count(), 0);

    QGraphicsScene sc;
    img.setScene(&sc);
    QCOMPARE(img.imageRect(), QRectF());
    QVERIFY(img.toJson().isNull());

    QVERIFY(img.loadImage(":/ceam_logo_color.jpg"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());
    QCOMPARE(spy.count(), 1);

    QVERIFY(!img.loadImage(":/not-exists"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());
    QCOMPARE(spy.count(), 1);

    QVERIFY(img.loadImage(":/ceam_logo_color.png"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());
    QCOMPARE(spy.count(), 2);

    QVERIFY(img.loadImage(":/app_icon.svg"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.toJson().isNull());
    QCOMPARE(spy.count(), 3);

    QVERIFY(!img.loadImage(":/not-exists"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.toJson().isNull());
    QCOMPARE(spy.count(), 3);

    img.clear();
    QCOMPARE(spy.count(), 4);
}

void TestSceneBackground::json()
{
    SceneBackground img;
    QGraphicsScene sc;
    img.setScene(&sc);
    QVERIFY(img.loadImage(":/ceam_logo_color.jpg"));
    QVERIFY(!img.isEmpty());

    auto json_img = img.toJson();
    QVERIFY(json_img.isObject());

    img.clear();
    QVERIFY(img.isEmpty());
    QVERIFY(img.toJson().isNull());

    QVERIFY(img.setFromJson(json_img));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.isEmpty());

    QVERIFY(img.loadImage(":/app_icon.svg"));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 512, 512));
    json_img = img.toJson();
    QVERIFY(json_img.isObject());

    img.clear();
    QVERIFY(img.isEmpty());
    QVERIFY(img.toJson().isNull());
    QCOMPARE(img.imageRect(), QRectF());

    QVERIFY(img.setFromJson(json_img));
    QCOMPARE(img.imageRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.isEmpty());
}
