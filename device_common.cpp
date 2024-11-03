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

using DevCatTuple = std::tuple<DeviceCategory, const char*, const char*>;
constexpr DevCatTuple DEV_CATS[] = {
    // clang-format off
    { DeviceCategory::Amplifier,    "amp",      QT_TRANSLATE_NOOP("ceam", "amplifiers") },
    { DeviceCategory::Computer,     "computer", QT_TRANSLATE_NOOP("ceam", "computers") },
    { DeviceCategory::Microphone,   "mic",      QT_TRANSLATE_NOOP("ceam", "microphones") },
    { DeviceCategory::Midi,         "midi",     QT_TRANSLATE_NOOP("ceam", "MIDI") },
    { DeviceCategory::Misc,         "misc",     QT_TRANSLATE_NOOP("ceam", "miscellaneous") },
    { DeviceCategory::Mixer,        "mix",      QT_TRANSLATE_NOOP("ceam", "mixing consoles") },
    { DeviceCategory::Network,      "net",      QT_TRANSLATE_NOOP("ceam", "network") },
    { DeviceCategory::Phones,       "phones",   QT_TRANSLATE_NOOP("ceam", "headphones") },
    { DeviceCategory::Photo,        "photo",    QT_TRANSLATE_NOOP("ceam", "photo") },
    { DeviceCategory::SoundCard,    "soundcard",QT_TRANSLATE_NOOP("ceam", "soundcards") },
    { DeviceCategory::Speaker,      "speaker",  QT_TRANSLATE_NOOP("ceam", "speakers") },
    { DeviceCategory::Synth,        "synth",    QT_TRANSLATE_NOOP("ceam", "synths") },
    { DeviceCategory::Video,        "video",    QT_TRANSLATE_NOOP("ceam", "video") },
    { DeviceCategory::Power,        "power",    QT_TRANSLATE_NOOP("ceam", "power") },
    { DeviceCategory::Guitar,       "guitar",   QT_TRANSLATE_NOOP("ceam", "guitars") },
    { DeviceCategory::Light,        "light",    QT_TRANSLATE_NOOP("ceam", "lights") },
    { DeviceCategory::Radio,        "radio",    QT_TRANSLATE_NOOP("ceam", "radio") },
    { DeviceCategory::Player,       "play",     QT_TRANSLATE_NOOP("ceam", "players") },
    { DeviceCategory::Recorder,     "rec",      QT_TRANSLATE_NOOP("ceam", "recorders") },
    // clang-format on
};

using InstrCatTuple = std::tuple<InstrumentCategory, const char*, const char*>;
constexpr InstrCatTuple INST_CATS[] = {
    // clang-format off
    { InstrumentCategory::Keyboard,         "keyboard",     QT_TRANSLATE_NOOP("ceam", "keyboards") },
    { InstrumentCategory::PercussionNoise,  "perc_noise",   QT_TRANSLATE_NOOP("ceam", "percussion (noise)") },
    { InstrumentCategory::PercussionTonal,  "perc_tonal",   QT_TRANSLATE_NOOP("ceam", "percussion (tonal)") },
    { InstrumentCategory::StringsBowed,     "bowed",        QT_TRANSLATE_NOOP("ceam", "strings (bowed)") },
    { InstrumentCategory::StringsPlucked,   "plucked",      QT_TRANSLATE_NOOP("ceam", "strings (plucked)") },
    { InstrumentCategory::WindsBrass,       "brass",        QT_TRANSLATE_NOOP("ceam", "brass") },
    { InstrumentCategory::WindsReed,        "reed",         QT_TRANSLATE_NOOP("ceam", "reed") },
    { InstrumentCategory::WindsWood,        "wood",         QT_TRANSLATE_NOOP("ceam", "woodwinds") },
    // clang-format on
};

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
constexpr const char* JSON_KEY_SUBCAT = "subcat";
constexpr const char* JSON_KEY_IMAGE_MIRROR = "image-mirror";
constexpr const char* JSON_KEY_LOGIC_VIEW = "view-logic";
constexpr const char* JSON_KEY_USER_VIEW = "view-user";
constexpr const char* JSON_KEY_NUM_COLS = "num-cols";
constexpr const char* JSON_KEY_NUM_ROWS = "num-rows";
constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_INDEXES = "indexes";
constexpr const char* JSON_KEY_TYPE = "type";
constexpr const char* JSON_STR_LOGIC = "logic";
constexpr const char* JSON_KEY_CURRENT_USER_VIEW = "current-view";
constexpr const char* JSON_KEY_SRC = "src";
constexpr const char* JSON_KEY_DEST = "dest";

constexpr const char* JSON_MIRROR_HORIZONTAL = "horizontal";

constexpr int MAX_BATTERIES_COUNT = 10;

QString readLocalizedKey(const QJsonObject& obj, const QString& key, const QString& lang)
{
    auto loc_value = obj.value(key + '-' + lang).toString();
    if (!loc_value.isEmpty())
        return loc_value;

    return obj.value(key).toString();
}

QJsonValue toJsonValue(ImageMirrorType type)
{
    switch (type) {
    case ImageMirrorType::Horizontal:
        return JSON_MIRROR_HORIZONTAL;
    case ImageMirrorType::None:
    default:
        return {};
    }
}

ImageMirrorType imageMirrorFromJson(const QJsonValue& v)
{
    if (v.isString()) {
        const auto str = v.toString();
        if (str == JSON_MIRROR_HORIZONTAL)
            return ImageMirrorType::Horizontal;
        else
            return ImageMirrorType::None;
    } else
        return ImageMirrorType::None;
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
        WARN() << "unknown category:" << str;
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
        WARN() << "json object expected, got:" << v;
        return false;
    }

    auto obj = v.toObject();

    auto json_id = obj.take(JSON_KEY_ID).toInteger(DEV_NULL_ID);
    if (json_id < 0)
        json_id = DEV_NULL_ID;

    id_ = json_id;

    auto lang = QLocale::system().bcp47Name();
    title_ = readLocalizedKey(obj, JSON_KEY_TITLE, lang);
    title_latin_ = obj.value(JSON_KEY_TITLE).toString();
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

    // compatibility
    if (obj.contains(JSON_KEY_INPUT_COLUMNS)) {
        WARN() << "obsolete key:" << JSON_KEY_INPUT_COLUMNS;
        if (!logic_view_data_.setMaxInputColumnCount(obj[JSON_KEY_INPUT_COLUMNS].toInt(DEF_COL_COUNT)))
            WARN() << "can't set value:" << JSON_KEY_INPUT_COLUMNS;
    }

    // compatibility
    if (obj.contains(JSON_KEY_OUTPUT_COLUMNS)) {
        WARN() << "obsolete key:" << JSON_KEY_OUTPUT_COLUMNS;
        if (!logic_view_data_.setMaxOutputColumnCount(obj[JSON_KEY_OUTPUT_COLUMNS].toInt(DEF_COL_COUNT)))
            WARN() << "can't set value:" << JSON_KEY_OUTPUT_COLUMNS;
    }

    if (obj.contains(JSON_KEY_LOGIC_VIEW)) {
        auto logic_view = XletsLogicViewData::fromJson(obj[JSON_KEY_LOGIC_VIEW]);
        if (logic_view)
            logic_view_data_ = *logic_view;
    }

    user_view_data_.clear();
    if (obj.contains(JSON_KEY_USER_VIEW)) {
        auto arr = obj[JSON_KEY_USER_VIEW].toArray();
        for (const auto& x : arr) {
            auto user_view = XletsUserViewData::fromJson(x);
            if (user_view)
                user_view_data_.push_back(*user_view);
        }
    }

    if (obj.contains(JSON_KEY_SUBCAT)) {
        auto subcat = SubCategory::fromJson(obj[JSON_KEY_SUBCAT]);
        if (subcat)
            subcat_ = *subcat;
    }

    current_user_view_ = obj[current_user_view_].toString();

    mirror_ = imageMirrorFromJson(obj[JSON_KEY_IMAGE_MIRROR]);

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
    json[JSON_KEY_LOGIC_VIEW] = logic_view_data_.toJson();
    json[JSON_KEY_SUBCAT] = subcat_.toJson();
    json[JSON_KEY_IMAGE_MIRROR] = toJsonValue(mirror_);

    QJsonArray arr;
    for (auto& x : user_view_data_)
        arr.append(x.toJson());

    json[JSON_KEY_USER_VIEW] = arr;
    json[JSON_KEY_CURRENT_USER_VIEW] = current_user_view_;

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

bool XletsLogicViewData::setMaxInputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_input_column_count_ = n;
    return true;
}

bool XletsLogicViewData::setMaxOutputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_output_column_count_ = n;
    return true;
}

QJsonValue XletsLogicViewData::toJson() const
{
    QJsonObject json;
    json[JSON_KEY_INPUT_COLUMNS] = max_input_column_count_;
    json[JSON_KEY_OUTPUT_COLUMNS] = max_output_column_count_;
    json[JSON_KEY_NAME] = name_;
    json[JSON_KEY_TYPE] = JSON_STR_LOGIC;
    return json;
}

std::optional<XletsLogicViewData> XletsLogicViewData::fromJson(const QJsonValue& j)
{
    XletsLogicViewData res;
    if (!j.isObject())
        return res;

    auto obj = j.toObject();

    res.max_input_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_INPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);
    res.max_output_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_OUTPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);

    return res;
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
        WARN() << "json array expected, got:" << v;
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
    if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        QJsonArray arr;

        for (auto& cat : DEV_CATS) {
            if (*pval & std::get<0>(cat))
                arr << std::get<1>(cat);
        }

        if (arr.empty())
            return {};
        else if (arr.size() == 1)
            return arr.first();
        else
            return arr;

    } else if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        QJsonArray arr;

        for (auto& cat : INST_CATS) {
            if (*pval & std::get<0>(cat))
                arr << std::get<1>(cat);
        }

        if (arr.empty())
            return {};
        else if (arr.size() == 1)
            return arr.first();
        else
            return arr;
    } else
        return {};
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

QList<SubCategory> SubCategory::separate() const
{
    QList<SubCategory> res;

    if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        for (auto& cat : DEV_CATS) {
            if (*pval & std::get<0>(cat))
                res << SubCategory { DeviceCategoryFlags { std::get<0>(cat) } };
        }
    } else if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        for (auto& cat : INST_CATS) {
            if (*pval & std::get<0>(cat))
                res << SubCategory { InstrumentCategoryFlags { std::get<0>(cat) } };
        }
    }

    return res;
}

const char* SubCategory::title() const
{
    if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        for (auto& cat : DEV_CATS) {
            if (*pval & std::get<0>(cat))
                return std::get<2>(cat);
        }
    } else if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        for (auto& cat : INST_CATS) {
            if (*pval & std::get<0>(cat))
                return std::get<2>(cat);
        }
    }

    return "";
}

bool SubCategory::operator&(DeviceCategory cat) const
{
    if (auto pval = std::get_if<DeviceCategoryFlags>(this)) {
        return *pval & cat;
    } else
        return false;
}

bool SubCategory::operator&(InstrumentCategory cat) const
{
    if (auto pval = std::get_if<InstrumentCategoryFlags>(this)) {
        return *pval & cat;
    } else
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

        for (auto& cat : DEV_CATS) {
            if (std::get<1>(cat) == str)
                return SubCategory { std::get<0>(cat) };
        }

        for (auto& cat : INST_CATS) {
            if (std::get<1>(cat) == str)
                return SubCategory { std::get<0>(cat) };
        }

        return {};
    } else
        return {};
}

XletsUserViewData::XletsUserViewData(int row, int cols)
    : name_("User")
{
    setColumnCount(cols);
    setRowCount(row);
}

void XletsUserViewData::setColumnCount(int n)
{
    col_count_ = qBound(MIN_COL_COUNT, n, MAX_COL_COUNT);
    xlets_idx_.resize(cellCount(), XletViewIndex::null());
}

void XletsUserViewData::setRowCount(int n)
{
    row_count_ = qBound(MIN_ROW_COUNT, n, MAX_ROW_COUNT);
    xlets_idx_.resize(cellCount(), XletViewIndex::null());
}

int XletsUserViewData::cellCount() const
{
    return col_count_ * row_count_;
}

XletViewIndex XletsUserViewData::xletAt(int pos) const
{
    if (pos < 0 || pos >= xlets_idx_.size())
        return XletViewIndex::null();
    else
        return xlets_idx_[pos];
}

bool XletsUserViewData::insertXlet(CellIndex cellIdx, XletViewIndex vidx)
{
    if (cellIdx.row < 0
        || cellIdx.row >= row_count_
        || cellIdx.column < 0
        || cellIdx.column >= col_count_) //
    {
        return false;
    }

    auto idx = cellIdx.row * col_count_ + cellIdx.column;
    if (idx >= cellCount())
        return false;

    auto it = std::find(xlets_idx_.begin(), xlets_idx_.end(), vidx);
    if (it != xlets_idx_.end()) { // xlet exists
        *it = XletViewIndex::null();
    }

    if (xlets_idx_[idx].isNull()) { // insert into free cell
        xlets_idx_[idx] = vidx;
    } else {
        if (xlets_idx_.back().isNull()) { // insert before
            WARN() << "insert before";
            xlets_idx_.insert(xlets_idx_.begin() + idx, vidx);
            xlets_idx_.pop_back();
        } else { // insert info free space
            auto empty_it = std::find(xlets_idx_.begin(), xlets_idx_.end(), XletViewIndex::null());
            if (empty_it == xlets_idx_.end()) // no free space
                return false;

            *empty_it = xlets_idx_[idx];
            xlets_idx_[idx] = vidx;
        }
    }

    return true;
}

bool XletsUserViewData::operator==(const XletsUserViewData& vd) const
{
    if (this == &vd)
        return true;

    return col_count_ == vd.col_count_
        && row_count_ == vd.row_count_
        && name_ == vd.name_
        && xlets_idx_ == vd.xlets_idx_;
}

QJsonValue XletsUserViewData::toJson() const
{
    QJsonObject obj;

    obj[JSON_KEY_NUM_COLS] = col_count_;
    obj[JSON_KEY_NUM_ROWS] = row_count_;
    obj[JSON_KEY_NAME] = name_;

    if (!xlets_idx_.empty()) {
        QJsonArray arr;
        int count = 0;

        for (int i = 0; i < xlets_idx_.size(); i++) {
            auto& x = xlets_idx_[i];
            auto j = x.toJson();
            if (j.isEmpty())
                continue;

            j[JSON_KEY_DEST] = i;
            arr.append(j);
            count++;
        }

        if (count > 0)
            obj[JSON_KEY_INDEXES] = arr;
    }

    return obj;
}

std::optional<XletsUserViewData> XletsUserViewData::fromJson(const QJsonValue& v)
{
    if (!v.isObject())
        return {};

    XletsUserViewData res;
    auto obj = v.toObject();
    res.setColumnCount(obj[JSON_KEY_NUM_COLS].toInt(DEF_COL_COUNT));
    res.setRowCount(obj[JSON_KEY_NUM_ROWS].toInt(DEF_ROW_COUNT));

    res.name_ = obj[JSON_KEY_NAME].toString("User");

    auto idxs = obj[JSON_KEY_INDEXES].toArray();

    std::vector<XletViewIndex> indexes;
    indexes.assign(res.cellCount(), XletViewIndex { 0, XletType::None });

    for (const auto& item : idxs) {
        if (!item.isObject())
            continue;

        auto obj = item.toObject();
        auto idx = XletViewIndex::fromJson(obj);
        if (idx) {
            auto dest_idx = obj[JSON_KEY_DEST].toInt(-1);
            if (dest_idx >= 0 && dest_idx < indexes.size()) {
                indexes[dest_idx] = *idx;
            } else {
                WARN() << "invalid destination index:" << dest_idx;
            }
        }
    }

    res.xlets_idx_ = indexes;

    return res;
}
