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
#include "device_library.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>

constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_INSTRUMENTS = "instruments";
constexpr const char* JSON_KEY_SENDS = "sends";
constexpr const char* JSON_KEY_RETURNS = "returns";
constexpr const char* JSON_KEY_HUMANS = "humans";
constexpr const char* JSON_KEY_FURNITURE = "furniture";

DeviceLibrary::DeviceLibrary() { }

bool DeviceLibrary::readFile(const QString& filename)
{
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << __FUNCTION__ << "file not exists:" << filename;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << __FUNCTION__ << "can't open file for reading:" << filename;
        return false;
    }

    auto val = file.readAll();
    file.close();

    auto doc = QJsonDocument::fromJson(val);
    if (!doc.isObject()) {
        qCritical() << __FUNCTION__ << "invalid JSON file:" << filename;
        return false;
    }

    auto root = doc.object();
    auto keys = root.keys();
    if (keys != QStringList { "library" }) {
        qCritical() << __FUNCTION__ << "invalid key in library:" << keys;
        return false;
    }

    auto lib = root.value("library");
    if (!lib.isObject()) {
        qCritical() << __FUNCTION__ << "'library' expected to be an object, got type:" << lib.type();
        return false;
    }

    readItems(lib.toObject().value(JSON_KEY_DEVICES), devices_, ItemCategory::Device);
    readItems(lib.toObject().value(JSON_KEY_INSTRUMENTS), instruments_, ItemCategory::Instrument);
    readItems(lib.toObject().value(JSON_KEY_SENDS), sends_, ItemCategory::Send);
    readItems(lib.toObject().value(JSON_KEY_RETURNS), returns_, ItemCategory::Return);
    readItems(lib.toObject().value(JSON_KEY_FURNITURE), furniture_, ItemCategory::Furniture);
    readItems(lib.toObject().value(JSON_KEY_DEVICES), humans_, ItemCategory::Human);

    return true;
}

bool DeviceLibrary::readItems(const QJsonValue& value, QList<SharedDeviceData>& items, ItemCategory cat)
{
    if (value.isNull())
        return false;

    if (!value.isArray()) {
        qWarning() << __FILE_NAME__ << __FUNCTION__ << "json array expected, got:" << value;
        return false;
    }

    auto arr = value.toArray();
    for (const auto& v : arr) {
        SharedDeviceData data(new DeviceData(DEV_NULL_ID));
        if (data->setJson(v)) {
            data->setCategory(cat);
            items.append(data);
        }
    }

    return true;
}
