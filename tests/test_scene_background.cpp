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
#include <QTest>

using namespace ceam;

void TestSceneBackground::construct()
{
    SceneBackground img;
    QVERIFY(img.isEmpty());
    QVERIFY(!img.isVisible());

    img.clear();
    QVERIFY(img.toJson().isNull());
}

void TestSceneBackground::pixmapTest()
{
    SceneBackground img;
    QVERIFY(!img.loadImage(":/ceam_logo_color.jpg"));

    QGraphicsScene sc;
    img.setScene(&sc);
    QCOMPARE(img.boundingRect(), QRectF());
    QVERIFY(img.toJson().isNull());
    QVERIFY(img.isVisible());

    QVERIFY(img.loadImage(":/ceam_logo_color.jpg"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());

    QVERIFY(!img.loadImage(":/not-exists"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());

    QVERIFY(img.loadImage(":/ceam_logo_color.png"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.toJson().isNull());

    QVERIFY(img.loadImage(":/app_icon.svg"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.toJson().isNull());

    QVERIFY(!img.loadImage(":/not-exists"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.toJson().isNull());
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
    QVERIFY(img.isVisible());
    QVERIFY(img.toJson().isNull());

    QVERIFY(img.setFromJson(json_img));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 100, 100));
    QVERIFY(!img.isEmpty());

    QVERIFY(img.loadImage(":/app_icon.svg"));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 512, 512));
    json_img = img.toJson();
    QVERIFY(json_img.isObject());

    img.clear();
    QVERIFY(img.isEmpty());
    QVERIFY(img.isVisible());
    QVERIFY(img.toJson().isNull());
    QCOMPARE(img.boundingRect(), QRectF());

    QVERIFY(img.setFromJson(json_img));
    QCOMPARE(img.boundingRect(), QRectF(0, 0, 512, 512));
    QVERIFY(!img.isEmpty());
}
