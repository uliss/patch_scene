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
#include "logging.hpp"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

using namespace ceam;

namespace {

#define CONST_STR(str) constexpr const char* str_##str = #str;

CONST_STR(computer)
CONST_STR(device)
CONST_STR(furniture)
CONST_STR(human)
CONST_STR(instrument)
CONST_STR(return )
CONST_STR(send)

#define CONST_PAIR(cat, name) constexpr auto cat_##name = std::make_pair(cat, #name);

CONST_PAIR(DeviceCategory::Amplifier, amp)
CONST_PAIR(DeviceCategory::Computer, computer)
CONST_PAIR(DeviceCategory::Microphone, mic)
CONST_PAIR(DeviceCategory::Midi, midi)
CONST_PAIR(DeviceCategory::Mixer, mix)
CONST_PAIR(DeviceCategory::Network, net)
CONST_PAIR(DeviceCategory::Phones, phones)
CONST_PAIR(DeviceCategory::Photo, photo)
CONST_PAIR(DeviceCategory::SoundCard, soundcard)
CONST_PAIR(DeviceCategory::Speaker, speaker)
CONST_PAIR(DeviceCategory::Synth, synth)
CONST_PAIR(DeviceCategory::Video, video)

CONST_PAIR(InstrumentCategory::Keyboard, keyboard)
CONST_PAIR(InstrumentCategory::PercussionNoise, perc_noise)
CONST_PAIR(InstrumentCategory::PercussionTonal, perc_tonal)
CONST_PAIR(InstrumentCategory::StringsBowed, bowed)
CONST_PAIR(InstrumentCategory::StringsPlucked, plucked)
CONST_PAIR(InstrumentCategory::WindsBrass, brass)
CONST_PAIR(InstrumentCategory::WindsReed, reed)
CONST_PAIR(InstrumentCategory::WindsWood, wood)

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
constexpr const char* JSON_KEY_INPUT_COLUMNS = "input-columns";
constexpr const char* JSON_KEY_OUTPUT_COLUMNS = "output-columns";

constexpr int MAX_BATTERIES_COUNT = 10;

QString readLocalizedKey(const QJsonObject& obj, const QString& key, const QString& lang)
{
    auto loc_value = obj.value(key + '-' + lang).toString();
    if (!loc_value.isEmpty())
        return loc_value;

    return obj.value(key).toString();
}

}

const char* ceam::toString(ItemCategory cat)
{
    switch (cat) {
    case ItemCategory::Human:
        return str_human;
    case ItemCategory::Furniture:
        return str_furniture;
    case ItemCategory::Send:
        return str_send;
    case ItemCategory::Return:
        return str_return;
    case ItemCategory::Instrument:
        return str_instrument;
    case ItemCategory::Device:
        return str_device;
    case ItemCategory::MaxCategory:
    default:
        return str_device;
    }
}

std::optional<ItemCategory> ceam::fromQString(const QString& str)
{
    if (str.isEmpty())
        return ItemCategory::Device;

    auto icat = str.toLower();
    if (icat == str_device) {
        return ItemCategory::Device;
    } else if (icat == str_return) {
        return ItemCategory::Return;
    } else if (icat == str_send) {
        return ItemCategory::Send;
    } else if (icat == str_furniture) {
        return ItemCategory::Furniture;
    } else if (icat == str_human) {
        return ItemCategory::Human;
    } else if (icat == str_instrument) {
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

bool DeviceData::showInDeviceCategory() const
{
    return category_ == ItemCategory::Device
        || (category_ == ItemCategory::Instrument && (inputs_.size() > 0 || outputs_.size() > 0));
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
    max_input_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_INPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);
    max_output_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_OUTPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);

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
    json[JSON_KEY_INPUT_COLUMNS] = max_input_column_count_;
    json[JSON_KEY_OUTPUT_COLUMNS] = max_output_column_count_;

    return json;
}

const XletData& DeviceData::inputAt(XletIndex n) const
{
    return inputs_.at(n);
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

bool DeviceData::setMaxInputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_input_column_count_ = n;
    return true;
}

bool DeviceData::setMaxOutputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_output_column_count_ = n;
    return true;
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

bool DeviceData::operator==(const DeviceData& data) const
{
    if (this == &data)
        return true;

    return inputs_ == data.inputs_
        && outputs_ == data.outputs_
        && model_ == data.model_
        && vendor_ == data.vendor_
        && title_ == data.title_
        && pos_ == data.pos_
        && zoom_ == data.zoom_
        && battery_count_ == data.battery_count_
        && battery_type_ == data.battery_type_
        && category_ == data.category_
        && show_title_ == data.show_title_;
}

size_t ceam::qHash(const ceam::DeviceData& data)
{
    return ::qHash(data.inputs())
        ^ ::qHash(data.outputs())
        ^ ::qHash(data.model())
        ^ ::qHash(data.vendor())
        ^ ::qHash(data.title())
        ^ ::qHash(data.pos().toPoint())
        ^ ::qHash(data.zoom())
        ^ ::qHash(data.batteryCount())
        ^ ::qHash((int)data.batteryType())
        ^ ::qHash((int)data.category())
        ^ ::qHash(data.showTitle());
}

QDebug operator<<(QDebug debug, const ceam::DeviceData& data)
{
    QDebugStateSaver saver(debug);

#define PRINT_LINE(name, value) debug.nospace() << " - " << name << ": " << value << "\n";

    debug << "Device #" << data.id() << "\n";
    PRINT_LINE("pos", data.pos());
    if (!data.title().isEmpty())
        PRINT_LINE(JSON_KEY_TITLE, data.title());

    PRINT_LINE(JSON_KEY_SHOW_TITLE, data.showTitle());

    if (!data.vendor().isEmpty())
        PRINT_LINE(JSON_KEY_VENDOR, data.vendor());

    if (!data.model().isEmpty())
        PRINT_LINE(JSON_KEY_MODEL, data.model());

    if (data.zoom() != 1)
        PRINT_LINE(JSON_KEY_ZOOM, data.zoom());

    if (!data.image().isEmpty())
        PRINT_LINE(JSON_KEY_IMAGE, data.image());

    PRINT_LINE(JSON_KEY_CATEGORY, toString(data.category()));
    if (data.batteryCount() > 0) {
        PRINT_LINE(JSON_KEY_BATTERY_COUNT, data.batteryCount());
        PRINT_LINE(JSON_KEY_BATTERY_TYPE, toJsonString(data.batteryType()));
    }

    if (!data.inputs().empty()) {
        PRINT_LINE(JSON_KEY_INPUTS, data.inputs().size());
    }

    if (!data.outputs().empty()) {
        PRINT_LINE(JSON_KEY_OUTPUTS, data.outputs().size());
    }

    return debug;
}

QJsonValue SubCategory::toJson() const
{
#define ADD_CAT(cat_name)           \
    {                               \
        if (*pval & cat_name.first) \
            arr << cat_name.second; \
    }

    if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        QJsonArray arr;

        ADD_CAT(cat_amp);
        ADD_CAT(cat_soundcard);
        ADD_CAT(cat_computer);
        ADD_CAT(cat_mic);
        ADD_CAT(cat_midi);
        ADD_CAT(cat_mix);
        ADD_CAT(cat_net);
        ADD_CAT(cat_phones);
        ADD_CAT(cat_photo);
        ADD_CAT(cat_speaker);
        ADD_CAT(cat_synth);
        ADD_CAT(cat_video);

        if (arr.empty())
            return {};
        else if (arr.size() == 1)
            return arr.first();
        else
            return arr;

    } else if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        QJsonArray arr;

        ADD_CAT(cat_bowed);
        ADD_CAT(cat_plucked);
        ADD_CAT(cat_perc_tonal);
        ADD_CAT(cat_perc_noise);
        ADD_CAT(cat_brass);
        ADD_CAT(cat_wood);
        ADD_CAT(cat_reed);
        ADD_CAT(cat_keyboard);

        if (arr.empty())
            return {};
        else if (arr.size() == 1)
            return arr.first();
        else
            return arr;
    } else
        return {};

#undef ADD_CAT
}

SubCategory& SubCategory::operator|=(const SubCategory& cat)
{
    if (cat.index() != index()) {
        WARN() << "non compatible subcategories";
        return *this;
    }

    if (auto pval0 = std::get_if<DeviceCategoryFlags>(this);
        auto pval1 = std::get_if<DeviceCategoryFlags>(&cat)) {
        *pval0 |= *pval1;
    } else if (auto pval0 = std::get_if<InstrumentCategoryFlags>(this);
               auto pval1 = std::get_if<InstrumentCategoryFlags>(&cat)) {
        *pval0 |= *pval1;
    }

    return *this;
}

bool SubCategory::operator|(DeviceCategory cat) const
{
    if (auto pval = std::get_if<DeviceCategoryFlags>(this))
        return *pval | cat;
    else
        return false;
}

bool SubCategory::operator|(InstrumentCategory cat) const
{
    if (auto pval = std::get_if<InstrumentCategoryFlags>(this))
        return *pval | cat;
    else
        return false;
}

SubCategory& SubCategory::operator|=(DeviceCategory cat)
{
    if (index() == 0) {
        *this = SubCategory { DeviceCategoryFlags(cat) };
    } else if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        *pval |= cat;
    }

    return *this;
}

SubCategory& SubCategory::operator|=(InstrumentCategory cat)
{
    if (index() == 0) {
        *this = SubCategory { InstrumentCategoryFlags(cat) };
    } else if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        *pval |= cat;
    }

    return *this;
}

std::optional<SubCategory> SubCategory::fromJson(const QJsonValue& val)
{
    if (val.isArray()) {
        auto arr = val.toArray();

        std::optional<SubCategory> res;

        for (const auto& x : arr) {
            if (x.isString()) {
                auto str_cat = fromJson(x);
                if (str_cat) {
                    if (res)
                        *res |= *str_cat;
                    else
                        res = str_cat;
                } else {
                    WARN() << "invalid subcategory value:" << x;
                }
            } else {
                WARN() << "invalid array value:" << x;
            }
        }

        return res;

    } else if (val.isString()) {
        const auto str = val.toString();

#define CHECK_CAT(cat_name)                    \
    if (str == cat_name.second) {              \
        return SubCategory { cat_name.first }; \
    }

        CHECK_CAT(cat_amp);
        CHECK_CAT(cat_soundcard);
        CHECK_CAT(cat_computer);
        CHECK_CAT(cat_mic);
        CHECK_CAT(cat_midi);
        CHECK_CAT(cat_mix);
        CHECK_CAT(cat_net);
        CHECK_CAT(cat_phones);
        CHECK_CAT(cat_photo);
        CHECK_CAT(cat_speaker);
        CHECK_CAT(cat_synth);
        CHECK_CAT(cat_video);

        CHECK_CAT(cat_bowed);
        CHECK_CAT(cat_plucked);
        CHECK_CAT(cat_perc_tonal);
        CHECK_CAT(cat_perc_noise);
        CHECK_CAT(cat_brass);
        CHECK_CAT(cat_wood);
        CHECK_CAT(cat_reed);
        CHECK_CAT(cat_keyboard);

        return {};

#undef CHECK_CAT
    } else
        return {};
}
