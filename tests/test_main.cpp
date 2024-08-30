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
#include <QtTest>

#include "tests/test_connection.h"
#include "tests/test_connection_database.h"
#include "tests/test_connector_type.h"
#include "tests/test_device_data.h"
#include "tests/test_device_library.h"
#include "tests/test_diagram.h"
#include "tests/test_diagram_image.h"
#include "tests/test_scene_connections.h"
#include "tests/test_scene_devices.h"
#include "tests/test_xlet.h"

int main(int argc, char* argv[])
{
#ifndef Q_OS_WIN
    setenv("LANG", "C", 1);
#endif

    QApplication app(argc, argv);

    int status = 0;

    TestConnection tst_connection;
    status |= QTest::qExec(&tst_connection, argc, argv);

    TestConnectionDatabase tst_connection_db;
    status |= QTest::qExec(&tst_connection_db, argc, argv);

    TestConnectorType tst_connector_type;
    status |= QTest::qExec(&tst_connector_type, argc, argv);

    TestDeviceData tst_device_data;
    status |= QTest::qExec(&tst_device_data, argc, argv);

    TestDeviceLibrary tst_device_library;
    status |= QTest::qExec(&tst_device_library, argc, argv);

    TestDiagram tst_diagram;
    status |= QTest::qExec(&tst_diagram, argc, argv);

    TestSceneConnections test_scene_connections;
    status |= QTest::qExec(&test_scene_connections, argc, argv);

    TestSceneDevices test_scene_devices;
    status |= QTest::qExec(&test_scene_devices, argc, argv);

    TestXlet test_xlet;
    status |= QTest::qExec(&test_xlet, argc, argv);

    TestDiagramImage test_diagram_image;
    status |= QTest::qExec(&test_diagram_image, argc, argv);

    return status;
}
