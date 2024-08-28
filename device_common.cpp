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
#include <QJsonParseError>

namespace {

constexpr const char* STR_DEVICE = "device";
constexpr const char* STR_SEND = "send";
constexpr const char* STR_RETURN = "return";
constexpr const char* STR_INSTRUMENT = "instrument";
constexpr const char* STR_HUMAN = "human";
constexpr const char* STR_FURNITURE = "furniture";

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
constexpr const char* JSON_KEY_SHOW_TITLE = "show-title";

constexpr int MAX_BATTERIES_COUNT = 10;

QString readLocalizedKey(const QJsonObject& obj, const QString& key, const QString& lang)
{
    auto loc_value = obj.value(key + '-' + lang).toString();
    if (!loc_value.isEmpty())
        return loc_value;

    return obj.value(key).toString();
}

}

using namespace ceam;

const char* ceam::toString(ItemCategory cat)
{
    switch (cat) {
    case ItemCategory::Human:
        return STR_HUMAN;
    case ItemCategory::Furniture:
        return STR_FURNITURE;
    case ItemCategory::Send:
        return STR_SEND;
    case ItemCategory::Return:
        return STR_RETURN;
    case ItemCategory::Instrument:
        return STR_INSTRUMENT;
    case ItemCategory::Device:
        return STR_DEVICE;
    case ItemCategory::MaxCategory:
    default:
        return STR_DEVICE;
    }
}

std::optional<ItemCategory> ceam::fromQString(const QString& str)
{
    if (str.isEmpty())
        return ItemCategory::Device;

    auto icat = str.toLower();
    if (icat == STR_DEVICE) {
        return ItemCategory::Device;
    } else if (icat == STR_RETURN) {
        return ItemCategory::Return;
    } else if (icat == STR_SEND) {
        return ItemCategory::Send;
    } else if (icat == STR_FURNITURE) {
        return ItemCategory::Furniture;
    } else if (icat == STR_HUMAN) {
        return ItemCategory::Human;
    } else if (icat == STR_INSTRUMENT) {
        return ItemCategory::Instrument;
    } else {
        qWarning() << __FUNCTION__ << "unknown category:" << str;
        return {};
    }
}

void ceam::foreachItemCategory(std::function<void(const char*, int)> fn)
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
        [](const XletData& x) { return x.isVisible(); });
}

size_t DeviceData::visOutputCount() const
{
    return std::count_if(outputs_.begin(), outputs_.end(),
        [](const XletData& x) { return x.isVisible(); });
}

bool DeviceData::hasVisInputs() const
{
    for (auto& x : inputs_)
        if (x.isVisible())
            return true;

    return false;
}

bool DeviceData::hasVisOutputs() const
{
    for (auto& x : outputs_)
        if (x.isVisible())
            return true;

    return false;
}

bool DeviceData::showInDeviceCategory() const
{
    return category_ == ItemCategory::Device
        || (category_ == ItemCategory::Instrument && (visInputCount() > 0 || hasVisOutputs() > 0));
}

void DeviceData::setZoom(qreal z)
{
    zoom_ = qBound(MIN_ZOOM, z, MAX_ZOOM);
}

QString DeviceData::title() const
{
    if (title_.isEmpty())
        return modelVendor();
    else
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

QString DeviceData::modelVendor() const
{
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
        qWarning() << __FILE__ << __FUNCTION__ << "json object expected, got:" << v;
        return false;
    }

    auto obj = v.toObject();

    auto json_id = obj.take(JSON_KEY_ID).toInteger(DEV_NULL_ID);
    if (json_id < 0)
        json_id = DEV_NULL_ID;

    id_ = json_id;

    auto lang = QLocale::system().bcp47Name();
    title_ = readLocalizedKey(obj, JSON_KEY_TITLE, lang);
    vendor_ = obj.value(JSON_KEY_VENDOR).toString();
    model_ = obj.value(JSON_KEY_MODEL).toString();
    pos_.setX(obj.value(JSON_KEY_X).toDouble(0));
    pos_.setY(obj.value(JSON_KEY_Y).toDouble(0));

    image_ = obj.value(JSON_KEY_IMAGE).toString();
    setZoom(obj.value(JSON_KEY_ZOOM).toDouble(1));

    auto cat = fromQString(obj.value(JSON_KEY_CATEGORY).toString());
    if (cat)
        category_ = cat.value();

    setXletJson(obj.value(JSON_KEY_INPUTS), inputs_);
    setXletJson(obj.value(JSON_KEY_OUTPUTS), outputs_);

    battery_count_ = qBound<int>(0, obj.value(JSON_KEY_BATTERY_COUNT).toInt(), MAX_BATTERIES_COUNT);
    battery_type_ = fromJsonString(obj.value(JSON_KEY_BATTERY_TYPE).toString());

    show_title_ = obj[JSON_KEY_SHOW_TITLE].toBool(true);

    return true;
}

bool DeviceData::setJson(const QByteArray& json)
{
    if (json.isEmpty()) {
        qDebug() << __FUNCTION__ << "empty data";
        return false;
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (doc.isNull()) {
        qWarning() << doc << err.errorString();
        return false;
    }

    if (!doc.isObject())
        return false;

    return setJson(doc.object());
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
    json[JSON_KEY_BATTERY_TYPE] = toJsonString(battery_type_);
    json[JSON_KEY_BATTERY_COUNT] = battery_count_;

    json[JSON_KEY_INPUTS] = xletToJson(inputs_);
    json[JSON_KEY_OUTPUTS] = xletToJson(outputs_);
    json[JSON_KEY_SHOW_TITLE] = show_title_;

    return json;
}

void DeviceData::foreachVisInput(std::function<void(XletIndex, XletData&)> fn)
{
    XletIndex idx = 0;
    for (auto& x : inputs_) {
        if (x.isVisible())
            fn(idx++, x);
    }
}

const XletData& DeviceData::inputAt(XletIndex n) const
{
    return inputs_.at(n);
}

void DeviceData::foreachVisOutput(std::function<void(XletIndex, XletData&)> fn)
{
    XletIndex idx = 0;
    for (auto& x : outputs_) {
        if (x.isVisible())
            fn(idx++, x);
    }
}

void DeviceData::setBatteryCount(int v)
{
    battery_count_ = qBound(0, v, 10);
}

void DeviceData::setBatteryType(int type)
{
    if (type < static_cast<int>(BatteryType::None) || type >= static_cast<int>(BatteryType::MaxBattery_))
        return;

    battery_type_ = static_cast<BatteryType>(type);
}

BatteryChange DeviceData::calcBatteryChange(const DeviceData& data) const
{
    return BatteryChange(battery_type_, battery_count_, data.battery_type_, data.battery_count_);
}

size_t DeviceData::calcModelId() const
{
    return ::qHash(title_)
        ^ ::qHash(model_)
        ^ ::qHash(vendor_)
        ^ ::qHash(static_cast<int>(category_))
        ^ ::qHash(static_cast<int>(battery_type_))
        ^ ::qHash(battery_count_);
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
        qWarning() << __FILE__ << __FUNCTION__ << "json array expected, got:" << v;
        return false;
    }

    xlets.clear();

    auto arr = v.toArray();
    for (const auto& x : arr) {
        auto data = XletData::fromJson(x);
        if (data)
            xlets.push_back(data.value());
    }

    return true;
}

namespace {

using BatteryMapType = QMap<BatteryType, std::pair<const char*, const char*>>;
const BatteryMapType& batteryNameMap()
{
    // clang-format off
    static const BatteryMapType map_ = {
        { BatteryType::None,        { "None",         "none" } },
        { BatteryType::AA,          { "AA",           "aa" } },
        { BatteryType::AAA,         { "AAA",          "aaa" } },
        { BatteryType::AAAA,        { "AAAA",         "aaaa" } },
        { BatteryType::B,           { "B",            "b" } },
        { BatteryType::C,           { "C",            "c" } },
        { BatteryType::A23,         { "A23",          "a23" } },
        { BatteryType::A27,         { "A27",          "a27" } },
        { BatteryType::PP3_Krona,   { "PP3 (Krona)",  "krona" } },
    };
    // clang-format on
    return map_;
}

}

// clang-format on

const char* ceam::toString(BatteryType type)
{
    auto it = batteryNameMap().find(type);
    return (it == batteryNameMap().end()) ? "?" : it->first;
}

const char* ceam::toJsonString(BatteryType type)
{
    auto it = batteryNameMap().find(type);
    return (it == batteryNameMap().end()) ? "?" : it->second;
}

BatteryType ceam::fromJsonString(const QString& str)
{
    for (auto it = batteryNameMap().keyValueBegin(); it != batteryNameMap().keyValueEnd(); ++it) {
        if (it->second.second == str.toLower())
            return it->first;
    }

    return BatteryType::None;
}

void ceam::foreachBatteryType(std::function<void(const char*, int)> fn)
{
    for (int i = static_cast<int>(BatteryType::None);
         i < static_cast<int>(BatteryType::MaxBattery_);
         i++) //
    {
        fn(toString(static_cast<BatteryType>(i)), i);
    }
}

BatteryChange::BatteryChange(BatteryType typeA, int countA, BatteryType typeB, int countB)
{
    typeA_ = typeA;
    typeB_ = typeB;
    countA_ = countA;
    countB_ = countB;
}

BatteryChange::operator bool() const
{
    return typeA_ != typeB_ || countA_ != countB_;
}

QDebug operator<<(QDebug debug, const ceam::DeviceData& data)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << data.toJson();
    return debug;
}
