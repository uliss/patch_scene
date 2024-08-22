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
#include "test_xlet.h"
#include "device_xlet.h"

#include <QTest>

using namespace ceam;

void TestXlet::init()
{
    XletData data;
    QCOMPARE(data.name, QString {});
    QCOMPARE(data.model, ConnectorModel::UNKNOWN);
    QCOMPARE(data.phantom_power, false);
    QCOMPARE(data.power_type, PowerType::None);
    QCOMPARE(data.type, ConnectorType::Plug_Female);
    QCOMPARE(data.visible, true);
}

static TestXlet test;
