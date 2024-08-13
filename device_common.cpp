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
#include "device_common.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>

constexpr const char* STR_DEVICE = "device";
constexpr const char* STR_SEND = "send";
constexpr const char* STR_RETURN = "return";
constexpr const char* STR_INSTRUMENT = "instrument";

constexpr const char* JSON_KEY_ID = "id";
constexpr const char* JSON_KEY_X = "x";
constexpr const char* JSON_KEY_Y = "y";
constexpr const char* JSON_KEY_TITLE = "title";
constexpr const char* JSON_KEY_MODEL = "model";
constexpr const char* JSON_KEY_VENDOR = "vendor";
constexpr const char* JSON_KEY_ZOOM = "zoom";
constexpr const char* JSON_KEY_IMAGE = "image";
constexpr const char* JSON_KEY_INPUTS = "inputs";
constexpr const char* JSON_KEY_OUTPUTS = "outputs";
constexpr const char* JSON_KEY_CATEGORY = "category";
constexpr const char* JSON_KEY_BATTERY_TYPE = "battery-type";
constexpr const char* JSON_KEY_BATTERY_COUNT = "battery-count";

const char* toString(ItemCategory cat)
{
    switch (cat) {
    case ItemCategory::Instrument:
        return STR_INSTRUMENT;
    case ItemCategory::Human:
        return "human";
    case ItemCategory::Furniture:
        return "furniture";
    case ItemCategory::Send:
        return STR_SEND;
    case ItemCategory::Return:
        return STR_RETURN;
    case ItemCategory::Device:
    default:
        return STR_DEVICE;
    }
}

bool fromQString(const QString& str, ItemCategory& cat)
{
    if (str.isEmpty()) {
        cat = ItemCategory::Device;
        return true;
    }

    auto icat = str.toLower();
    if (icat == STR_DEVICE) {
        cat = ItemCategory::Device;
        return true;
    } else if (icat == STR_RETURN) {
        cat = ItemCategory::Return;
        return true;
    } else if (icat == STR_SEND) {
        cat = ItemCategory::Send;
        return true;
    } else if (icat == "furniture") {
        cat = ItemCategory::Furniture;
        return true;
    } else if (icat == "human") {
        cat = ItemCategory::Human;
        return true;
    } else if (icat == STR_INSTRUMENT) {
        cat = ItemCategory::Instrument;
        return true;
    } else {
        qWarning() << __FUNCTION__ << "unknown category:" << str;
        return false;
    }
}

void foreachItemCategory(std::function<void(const char*, int)> fn)
{
    for (int i = static_cast<int>(ItemCategory::Device);
         i < static_cast<int>(ItemCategory::MaxCategory);
         i++) //
    {
        fn(toString(static_cast<ItemCategory>(i)), i);
    }
}

DeviceData::DeviceData(DeviceId id)
    : id_(id)
{
}

size_t DeviceData::visInputCount() const
{
    return std::count_if(inputs_.begin(), inputs_.end(),
        [](const XletData& x) { return x.visible; });
}

size_t DeviceData::visOutputCount() const
{
    return std::count_if(outputs_.begin(), outputs_.end(),
        [](const XletData& x) { return x.visible; });
}

bool DeviceData::hasVisInputs() const
{
    return visInputCount() > 0;
}

bool DeviceData::hasVisOutputs() const
{
    return visOutputCount() > 0;
}

void DeviceData::setZoom(qreal z)
{
    zoom_ = qBound(MIN_ZOOM, z, MAX_ZOOM);
}

QString DeviceData::title() const
{
    if (title_.isEmpty()) {
        const bool v = !vendor_.isEmpty();
        const bool m = !model_.isEmpty();

        if (!m && !v)
            return "Unknown";
        else if (m && v)
            return QString("%1 %2").arg(vendor_, model_);
        else if (!m && v)
            return vendor_;
        else if (m && !v)
            return model_;
        else
            return "????";
    } else
        return title_;
}

void DeviceData::setTitle(const QString& title)
{
    title_ = title.trimmed();
}

void DeviceData::setVendor(const QString& vendor)
{
    vendor_ = vendor.trimmed();
}

void DeviceData::setModel(const QString& model)
{
    model_ = model.trimmed();
}

void DeviceData::setImage(const QString& image)
{
    image_ = image.trimmed();
}

QString DeviceData::imageIconPath() const
{
    return image_.isEmpty() ? QString {} : QString(":/devices/%1.svg").arg(image_);
}

bool DeviceData::setCategoryIndex(int idx)
{
    if (idx < 0 || idx >= static_cast<int>(ItemCategory::MaxCategory))
        return false;

    category_ = static_cast<ItemCategory>(idx);
    return true;
}

bool DeviceData::setJson(const QJsonValue& v)
{
    if (v.isUndefined())
        return false;

    if (!v.isObject()) {
        qWarning() << __FILE_NAME__ << __FUNCTION__ << "json object expected, got:" << v;
        return false;
    }

    auto obj = v.toObject();

    auto json_id = obj.take(JSON_KEY_ID).toInteger(DEV_NULL_ID);
    if (json_id < 0)
        json_id = DEV_NULL_ID;

    id_ = json_id;
    title_ = obj.value(JSON_KEY_TITLE).toString();
    vendor_ = obj.value(JSON_KEY_VENDOR).toString();
    model_ = obj.value(JSON_KEY_MODEL).toString();
    pos_.setX(obj.value(JSON_KEY_X).toDouble(0));
    pos_.setY(obj.value(JSON_KEY_Y).toDouble(0));

    image_ = obj.value(JSON_KEY_IMAGE).toString();
    setZoom(obj.value(JSON_KEY_ZOOM).toDouble(1));

    ItemCategory cat;
    if (fromQString(obj.value(JSON_KEY_CATEGORY).toString(), cat))
        category_ = cat;

    setXletJson(obj.value(JSON_KEY_INPUTS), inputs_);
    setXletJson(obj.value(JSON_KEY_OUTPUTS), outputs_);

    return true;
}

QJsonObject DeviceData::toJson() const
{
    QJsonObject json;

    json[JSON_KEY_ID] = static_cast<qint32>(id_);
    json[JSON_KEY_X] = pos_.x();
    json[JSON_KEY_Y] = pos_.y();
    json[JSON_KEY_TITLE] = title_;
    json[JSON_KEY_VENDOR] = vendor_;
    json[JSON_KEY_MODEL] = model_;
    json[JSON_KEY_ZOOM] = zoom_;
    // json["zvalue"] = data_->zvalue;
    json[JSON_KEY_IMAGE] = image_;
    json[JSON_KEY_CATEGORY] = toString(category_);
    json[JSON_KEY_BATTERY_TYPE] = toQString(battery_);
    json[JSON_KEY_BATTERY_COUNT] = battery_count_;

    json[JSON_KEY_INPUTS] = xletToJson(inputs_);
    json[JSON_KEY_OUTPUTS] = xletToJson(outputs_);

    return json;
}

void DeviceData::foreachVisInput(std::function<void(XletIndex, const XletData&)> fn)
{
    XletIndex idx = 0;
    for (auto& x : inputs_) {
        if (x.visible)
            fn(idx++, x);
    }
}

const XletData& DeviceData::inputAt(XletIndex n) const
{
    return inputs_.at(n);
}

QJsonArray DeviceData::xletToJson(const QList<XletData>& xlets)
{
    QJsonArray arr;
    for (auto& x : xlets)
        arr.append(x.toJson());

    return arr;
}

bool DeviceData::setXletJson(const QJsonValue& v, QList<XletData>& xlets)
{
    if (v.isUndefined())
        return false;

    if (!v.isArray()) {
        qWarning() << __FILE_NAME__ << __FUNCTION__ << "json array expected, got:" << v;
        return false;
    }

    xlets.clear();

    auto arr = v.toArray();
    for (const auto& x : arr) {
        XletData data;
        if (XletData::fromJson(x, data))
            xlets.push_back(data);
    }

    return true;
}

QString toQString(BatteryType type)
{
    switch (type) {
    case BatteryType::AA:
        return "AA";
    case BatteryType::AAA:
        return "AAA";
    case BatteryType::Crona:
        return "Crona";
    case BatteryType::None:
    default:
        return {};
    }
}

QDebug operator<<(QDebug debug, const DeviceData& data)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << data.toJson();
    return debug;
}
