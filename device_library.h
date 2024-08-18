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
#ifndef DEVICE_LIBRARY_H
#define DEVICE_LIBRARY_H

#include "device_common.h"

#include <QJsonObject>
#include <QString>

namespace ceam {

class DeviceLibrary {
public:
    DeviceLibrary();

    bool readFile(const QString& filename);

    const QList<SharedDeviceData>& devices() const { return devices_; }
    const QList<SharedDeviceData>& furniture() const { return furniture_; }
    const QList<SharedDeviceData>& humans() const { return humans_; }
    const QList<SharedDeviceData>& instruments() const { return instruments_; }
    const QList<SharedDeviceData>& returns() const { return returns_; }
    const QList<SharedDeviceData>& sends() const { return sends_; }

private:
    static bool readItems(const QJsonValue& value, QList<SharedDeviceData>& items, ItemCategory cat);

private:
    QList<SharedDeviceData> devices_, instruments_, sends_, returns_, humans_, furniture_;
};

}

#endif // DEVICE_LIBRARY_H
