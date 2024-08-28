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
#include "test_device_library.h"
#include "device_library.h"
#include "tests/json_common.h"

#include <QJsonDocument>
#include <QTest>

static TestDeviceLibrary test_device_library;

using namespace ceam;

void TestDeviceLibrary::addItems()
{
    DeviceLibrary lib;

    QVERIFY(lib.devices().isEmpty());
    QVERIFY(lib.instruments().isEmpty());
    QVERIFY(lib.humans().isEmpty());
    QVERIFY(lib.furniture().isEmpty());
    QVERIFY(lib.sends().isEmpty());
    QVERIFY(lib.returns().isEmpty());

    QList<SharedDeviceData> items;
    lib.addItems(items);
    QVERIFY(lib.devices().isEmpty());
    QVERIFY(lib.instruments().isEmpty());
    QVERIFY(lib.humans().isEmpty());
    QVERIFY(lib.furniture().isEmpty());
    QVERIFY(lib.sends().isEmpty());
    QVERIFY(lib.returns().isEmpty());

    items << read_device_json_file("test_device_library_1.json");
    lib.addItems(items);

    QCOMPARE(lib.devices().count(), 1);
    QVERIFY(lib.instruments().isEmpty());
    QVERIFY(lib.humans().isEmpty());
    QVERIFY(lib.furniture().isEmpty());
    QVERIFY(lib.sends().isEmpty());
    QVERIFY(lib.returns().isEmpty());

    items.clear();
    items << read_device_json_file("test_device_library_2.json");
    lib.addItems(items);
    QCOMPARE(lib.devices().count(), 2);
    QVERIFY(lib.instruments().isEmpty());
    QVERIFY(lib.humans().isEmpty());
    QVERIFY(lib.furniture().isEmpty());
    QVERIFY(lib.sends().isEmpty());
    QVERIFY(lib.returns().isEmpty());

    items.clear();
    items << read_device_json_file("test_device_library_3.json");

    lib.addItems(items);
    QCOMPARE(lib.devices().count(), 2);
    QCOMPARE(lib.instruments().count(), 1);
    QVERIFY(lib.humans().isEmpty());
    QVERIFY(lib.furniture().isEmpty());
    QVERIFY(lib.sends().isEmpty());
    QVERIFY(lib.returns().isEmpty());
}

void TestDeviceLibrary::readLibrary()
{
    DeviceLibrary lib;
    QVERIFY(!lib.readFile(""));
    QVERIFY(lib.readFile(make_test_filename("test_device_library_lib1.json")));

    QCOMPARE(lib.devices().count(), 1);
    QCOMPARE(lib.instruments().count(), 1);
    QCOMPARE(lib.humans().count(), 1);
    QCOMPARE(lib.furniture().count(), 1);
    QCOMPARE(lib.sends().count(), 1);
    QCOMPARE(lib.returns().count(), 1);

    QCOMPARE(lib.devices().front()->title(), "Device 1");
    QCOMPARE(lib.instruments().front()->title(), "Instrument");
    QCOMPARE(lib.humans().front()->title(), "Alice");
    QCOMPARE(lib.furniture().front()->title(), "Furniture 1");
    QCOMPARE(lib.sends().front()->title(), "Send 1");
    QCOMPARE(lib.returns().front()->title(), "Return 1");
}
