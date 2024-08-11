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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

constexpr const char* KEY_MODEL = "model";
constexpr const char* KEY_VENDOR = "vendor";
constexpr const char* KEY_TITLE = "title";
constexpr const char* KEY_ZOOM = "zoom";
constexpr const char* KEY_IMAGE = "image";
constexpr const char* KEY_INPUTS = "inputs";
constexpr const char* KEY_OUTPUTS = "outputs";
constexpr const char* KEY_CATEGORY = "category";

constexpr qreal MIN_ZOOM = 0.25;
constexpr qreal MAX_ZOOM = 4;
constexpr qreal DEF_ZOOM = 1;

DeviceInfo::DeviceInfo()
    : category_(ItemCategory::Device)
    , zoom_(DEF_ZOOM)
{
}

void DeviceInfo::setZoom(qreal z)
{
    zoom_ = qBound(MIN_ZOOM, z, MAX_ZOOM);
}

QString DeviceInfo::title() const
{
    if (title_.isEmpty()) {
        QString res;
        if (!vendor_.isEmpty()) {
            res += vendor_;
            res += ' ';
        }
        res += model_;
        return res;
    } else
        return title_;
}

bool DeviceInfo::setCategory(const QString& cat)
{
    return fromQString(cat, category_);
}

QString DeviceInfo::categoryName() const
{
    return toString(category_);
}

bool DeviceInfo::setJson(const QJsonObject& obj)
{
    setModel(obj.value(KEY_MODEL).toString());
    setVendor(obj.value(KEY_VENDOR).toString());
    setTitle(obj.value(KEY_TITLE).toString());
    setImage(obj.value(KEY_IMAGE).toString());
    setZoom(obj.value(KEY_ZOOM).toDouble(DEF_ZOOM));

    ItemCategory cat = ItemCategory::Device;
    if (fromQString(obj.value(KEY_CATEGORY).toString(), cat))
        setCategory(cat);

    auto ins = obj.value(KEY_INPUTS).toArray();
    for (const auto& x : ins) {
        XletData xlet;
        if (XletData::fromJson(x.toObject(), xlet))
            inputs_.push_back(xlet);
    }

    auto outs = obj.value(KEY_OUTPUTS).toArray();
    for (const auto& x : outs) {
        XletData xlet;
        if (XletData::fromJson(x.toObject(), xlet))
            outputs_.push_back(xlet);
    }

    return true;
}

QJsonObject DeviceInfo::toJson() const
{
    QJsonObject res;
    res[KEY_MODEL] = model_;
    res[KEY_VENDOR] = vendor_;
    res[KEY_TITLE] = title_;
    res[KEY_IMAGE] = image_;
    res[KEY_ZOOM] = zoom_;
    res[KEY_CATEGORY] = toString(category_);

    QJsonArray inputs;
    for (auto& x : inputs_) {
        inputs.append(x.toJson());
    }
    res[KEY_INPUTS] = inputs;

    QJsonArray outputs;
    for (auto& x : outputs_) {
        outputs.append(x.toJson());
    }
    res[KEY_OUTPUTS] = outputs;

    return res;
}

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

    readDevices(lib.toObject().value("devices"));
    readInstruments(lib.toObject().value("instruments"));
    readSends(lib.toObject().value("sends"));
    readReturns(lib.toObject().value("returns"));

    return true;
}

bool DeviceLibrary::readDevices(const QJsonValue& devs)
{
    if (devs.isArray()) {
        auto arr = devs.toArray();
        for (const auto& obj : arr) {
            if (obj.isObject()) {
                auto dev = obj.toObject();

                DeviceInfo dev_info;
                dev_info.setJson(dev);
                dev_info.setCategory(ItemCategory::Device);
                devices_.push_back(dev_info);
            }
        }

        return true;
    } else {
        return false;
    }
}

bool DeviceLibrary::readInstruments(const QJsonValue& instr)
{
    if (instr.isArray()) {
        auto arr = instr.toArray();
        for (const auto& obj : arr) {
            if (obj.isObject()) {
                auto item = obj.toObject();

                DeviceInfo dev_info;
                dev_info.setJson(item);
                dev_info.setCategory(ItemCategory::Instrument);
                instruments_.push_back(dev_info);
            }
        }

        return true;
    } else {
        return false;
    }
}

bool DeviceLibrary::readSends(const QJsonValue& sends)
{
    if (sends.isArray()) {
        auto arr = sends.toArray();
        for (const auto& obj : arr) {
            if (obj.isObject()) {
                auto item = obj.toObject();

                DeviceInfo dev_info;
                dev_info.setJson(item);
                dev_info.setCategory(ItemCategory::Send);
                sends_.push_back(dev_info);
            }
        }

        return true;
    } else {
        return false;
    }
}

bool DeviceLibrary::readReturns(const QJsonValue& returns)
{
    if (returns.isArray()) {
        auto arr = returns.toArray();
        for (const auto& obj : arr) {
            if (obj.isObject()) {
                auto item = obj.toObject();

                DeviceInfo dev_info;
                dev_info.setJson(item);
                dev_info.setCategory(ItemCategory::Return);
                returns_.push_back(dev_info);
            }
        }

        return true;
    } else {
        return false;
    }
}
