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
#include "logging.hpp"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>

namespace {

constexpr int FORMAT_VERSION_MAJOR = 1;
constexpr int FORMAT_VERSION_MINOR = 0;

constexpr const char* JSON_KEY_META = "meta";
constexpr const char* JSON_KEY_META_FORMAT_MAJOR = "format-major";
constexpr const char* JSON_KEY_META_FORMAT_MINOR = "format-minor";
constexpr const char* JSON_KEY_META_TITLE = "title";
constexpr const char* JSON_KEY_META_AUTHOR = "author";
constexpr const char* JSON_KEY_META_VERSION = "version";
constexpr const char* JSON_KEY_META_DATE = "date";

constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_FORMAT = "format";
constexpr const char* JSON_KEY_FURNITURE = "furniture";
constexpr const char* JSON_KEY_HUMANS = "humans";
constexpr const char* JSON_KEY_INSTRUMENTS = "instruments";
constexpr const char* JSON_KEY_LIBRARY = "library";
constexpr const char* JSON_KEY_RETURNS = "returns";
constexpr const char* JSON_KEY_SENDS = "sends";

}

using namespace ceam;

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
    readItems(lib.toObject().value(JSON_KEY_HUMANS), humans_, ItemCategory::Human);

    for (auto& h : humans_)
        h->setShowTitle(false);

    for (auto& f : furniture_)
        f->setShowTitle(false);

    return true;
}

bool DeviceLibrary::writeFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODeviceBase::WriteOnly))
        return false;

    QJsonObject lib;
    writeItems(lib, devices_, ItemCategory::Device);
    writeItems(lib, instruments_, ItemCategory::Instrument);
    writeItems(lib, humans_, ItemCategory::Human);
    writeItems(lib, furniture_, ItemCategory::Furniture);
    writeItems(lib, sends_, ItemCategory::Send);
    writeItems(lib, returns_, ItemCategory::Return);

    QJsonObject meta;
    meta[JSON_KEY_META_AUTHOR] = "auto_export";
    meta[JSON_KEY_META_TITLE] = "my favorites";
    meta[JSON_KEY_META_FORMAT_MAJOR] = FORMAT_VERSION_MAJOR;
    meta[JSON_KEY_META_FORMAT_MINOR] = FORMAT_VERSION_MINOR;
    meta[JSON_KEY_META_VERSION] = 0;
    meta[JSON_KEY_META_DATE] = QDateTime::currentDateTime().toLocalTime().toString();

    QJsonObject root;
    root[JSON_KEY_META] = meta;
    root[JSON_KEY_LIBRARY] = lib;
    QJsonDocument doc(root);

    return file.write(doc.toJson()) > 0;
}

void DeviceLibrary::addItems(const QList<SharedDeviceData>& items)
{
    for (auto& x : items) {
        if (!x)
            continue;

        switch (x->category()) {
        case ItemCategory::Device:
            devices_.push_back(x);
            break;
        case ItemCategory::Instrument:
            instruments_.push_back(x);
            break;
        case ItemCategory::Human:
            humans_.push_back(x);
            break;
        case ItemCategory::Furniture:
            furniture_.push_back(x);
            break;
        case ItemCategory::Send:
            sends_.push_back(x);
            break;
        case ItemCategory::Return:
            returns_.push_back(x);
            break;
        default:
            break;
        }
    }
}

QMap<SubCategory, QList<SharedDeviceData>> DeviceLibrary::splitBySubcategory(const QList<SharedDeviceData>& items)
{
    QMap<SubCategory, QList<SharedDeviceData>> res;

    for (auto& data : items) {
        auto subcat = data->subCategory();
        if (subcat.isValid()) {
            for (auto& cat : data->subCategory().separate()) {
                res[cat] << data;
            }
        } else {
            res[subcat] << data;
        }
    }

    return res;
}

bool DeviceLibrary::readItems(const QJsonValue& value, QList<SharedDeviceData>& items, ItemCategory cat)
{
    if (value.isUndefined())
        return false;

    if (!value.isArray()) {
        WARN() << "json array expected, got:" << value;
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

bool DeviceLibrary::writeItems(QJsonObject& value, const QList<SharedDeviceData>& items, ItemCategory cat)
{
    QJsonArray arr;
    for (auto& x : items) {
        if (!x)
            continue;

        arr.append(x->toJson());
    }

    auto cat2key = [](ItemCategory cat) {
        switch (cat) {
        case ItemCategory::Device:
            return JSON_KEY_DEVICES;
        case ItemCategory::Instrument:
            return JSON_KEY_INSTRUMENTS;
        case ItemCategory::Human:
            return JSON_KEY_HUMANS;
        case ItemCategory::Furniture:
            return JSON_KEY_FURNITURE;
        case ItemCategory::Send:
            return JSON_KEY_SENDS;
        case ItemCategory::Return:
            return JSON_KEY_RETURNS;
        case ItemCategory::MaxCategory:
            return "?";
        }
    };

    value[cat2key(cat)] = arr;
    return true;
}
