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
#ifndef DEVICE_COMMON_H
#define DEVICE_COMMON_H

#include <QFlags>
#include <QJsonArray>
#include <QSharedData>
#include <QString>

#include <functional>

#include "battery.h"
#include "connection_data.h"
#include "device_xlet.h"
#include "xlets_logic_view_data.h"
#include "xlets_user_view_data.h"

namespace ceam {

enum class ItemCategory : std::uint8_t {
    Device,
    Instrument,
    Human,
    Furniture,
    Send,
    Return,
    Comment,
    MaxCategory,
    // when add new value - update toQString, fromQString, foreachItemCategory functions!
};

const char* toString(ItemCategory cat);
std::optional<ItemCategory> fromQString(const QString& str);

// fn params: category enum, category name, category index
void foreachItemCategory(const std::function<void(ItemCategory, const char*, int)>& fn);

enum class DeviceCategory {
    // clang-format off
    Amplifier       = (1 << 0),
    SoundCard       = (1 << 1),
    Computer        = (1 << 2),
    Microphone      = (1 << 3),
    Midi            = (1 << 4),
    Misc            = (1 << 5),
    Mixer           = (1 << 6),
    Network         = (1 << 7),
    Phones          = (1 << 8),
    Photo           = (1 << 9),
    Speaker         = (1 << 10),
    Synth           = (1 << 11),
    Video           = (1 << 12),
    Power           = (1 << 13),
    Guitar          = (1 << 14),
    Light           = (1 << 15),
    Radio           = (1 << 16),
    Recorder        = (1 << 17),
    Player          = (1 << 18),
    // clang-format on
};

Q_DECLARE_FLAGS(DeviceCategoryFlags, DeviceCategory)
Q_DECLARE_OPERATORS_FOR_FLAGS(DeviceCategoryFlags)

enum class InstrumentCategory {
    // clang-format off
    WindsWood       = (1 << 0),
    WindsBrass      = (1 << 1),
    WindsReed       = (1 << 2),
    StringsBowed    = (1 << 3),
    StringsPlucked  = (1 << 4),
    PercussionTonal = (1 << 5),
    PercussionNoise = (1 << 6),
    Keyboard        = (1 << 7),
    // clang-format on
};

Q_DECLARE_FLAGS(InstrumentCategoryFlags, InstrumentCategory)
Q_DECLARE_OPERATORS_FOR_FLAGS(InstrumentCategoryFlags)

class SubCategory : public std::variant<std::monostate,
                        DeviceCategoryFlags,
                        InstrumentCategoryFlags> {
public:
    bool isValid() const { return index() != 0; }
    QJsonValue toJson() const;
    SubCategory& operator|=(const SubCategory& cat);
    SubCategory& operator|=(DeviceCategory cat);
    SubCategory& operator|=(InstrumentCategory cat);

    QList<SubCategory> separate() const;

    const char* title() const;

    bool operator&(DeviceCategory cat) const;
    bool operator&(InstrumentCategory cat) const;
    static std::optional<SubCategory> fromJson(const QJsonValue& val);
};

enum class ImageMirrorType : std::uint8_t {
    None,
    Horizontal
};

using DeviceDataInfo = QList<std::pair<QString, QString>>;

class DeviceData : public QSharedData {
public:
    constexpr static const qreal MIN_ZOOM = 0.25;
    constexpr static const qreal MAX_ZOOM = 4;
    constexpr static int MAX_COL_COUNT = 24;
    constexpr static int MIN_COL_COUNT = 2;
    constexpr static int DEF_COL_COUNT = 8;

public:
    explicit DeviceData(DeviceId id);

    bool operator==(const DeviceData& data) const;
    bool operator!=(const DeviceData& data) const
    {
        return !operator==(data);
    }

    bool isNull() const { return id_ == DEV_NULL_ID; }
    DeviceId id() const { return id_; }
    void setId(DeviceId id) { id_ = id; }

    bool showInDeviceCategory() const;

    qreal zoom() const { return zoom_; }
    void setZoom(qreal z);

    QString title() const;
    const QString& titleLatin() const { return title_latin_; }
    void setTitle(const QString& title);

    const QString& vendor() const { return vendor_; }
    void setVendor(const QString& vendor);

    const QString& model() const { return model_; }
    void setModel(const QString& model);

    QString modelVendor() const;

    QString image() const { return image_; }
    void setImage(const QString& image);
    QString imageIconPath() const;

    ItemCategory category() const { return category_; }
    void setCategory(ItemCategory cat) { category_ = cat; }

    int categoryIndex() const { return static_cast<int>(category_); }
    bool setCategoryIndex(int idx);

    bool setJson(const QJsonValue& v);
    bool setJson(const QByteArray& json);
    QJsonObject toJson() const;

    QList<XletData>& inputs() { return inputs_; }
    const QList<XletData>& inputs() const { return inputs_; }
    const XletData& inputAt(XletIndex n) const;
    void appendInput(const XletData& x) { inputs_.append(x); }

    QList<XletData>& outputs() { return outputs_; }
    const QList<XletData>& outputs() const { return outputs_; }
    const XletData& outputAt(XletIndex n) const { return outputs_.at(n); }
    void appendOutput(const XletData& x) { outputs_.append(x); }

    bool hasAnyXput() const;

    const QPointF& pos() const { return pos_; }
    void setPos(const QPointF& pos) { pos_ = pos; }

    int batteryCount() const { return battery_count_; }
    void setBatteryCount(int v);

    /**
     * battery capacity in minutes
     */
    int batteryCapacity() const { return battery_capacity_; }
    void setBatteryCapacity(int v);

    BatteryType batteryType() const { return battery_type_; }
    void setBatteryType(int type);
    BatteryChange calcBatteryChange(const DeviceData& data) const;

    size_t calcModelId() const;

    bool showTitle() const { return show_title_; }
    void setShowTitle(bool value) { show_title_ = value; }

    bool isLocked() const { return locked_; }
    void setLocked(bool value) { locked_ = value; }

    SubCategory subCategory() const { return subcat_; }
    void setSubCategory(SubCategory subcat) { subcat_ = subcat; }

    ImageMirrorType imageMirror() const { return mirror_; }
    void setImageMirror(ImageMirrorType type) { mirror_ = type; }

    XletsLogicViewData& logicViewData() { return logic_view_data_; }
    const XletsLogicViewData& logicViewData() const { return logic_view_data_; }

    QList<XletsUserViewData>& userViewData() { return user_view_data_; }
    const QList<XletsUserViewData>& userViewData() const { return user_view_data_; }

    const QString& currentUserView() const { return current_user_view_; }
    void setCurrentUserView(const QString& name) { current_user_view_ = name; }

    QString verboseInfo() const;

    DeviceDataInfo& info() { return info_; }
    const DeviceDataInfo& info() const { return info_; }

    qreal weight() const { return weight_; }
    void setWeight(qreal w);

    qreal volume() const { return volume_; }
    void setVolume(qreal vol);

    // view width in pixels (for comments etc.)
    int viewWidth() const { return view_width_; }
    void setViewWidth(int w);

    // view height in pixels (for comments etc.)
    int viewHeight() const { return view_height_; }
    void setViewHeight(int h);

    // border width in pixels (for comments etc.)
    int borderWidth() const { return border_width_; }
    void setBorderWidth(int px);

    QColor borderColor() const { return border_color_; }
    void setBorderColor(const QColor& color) { border_color_ = color; }

    QColor backgroundColor() const { return background_color_; }
    void setBackgroundColor(const QColor& color) { background_color_ = color; }

    QColor textColor() const { return text_color_; }
    void setTextColor(const QColor& color) { text_color_ = color; }

private:
    static QJsonArray xletToJson(const QList<XletData>& xlets);
    static bool setXletJson(const QJsonValue& v, QList<XletData>& xlets);

private:
    QList<XletData> inputs_;
    QList<XletData> outputs_;
    QString model_, vendor_, title_, title_latin_;
    QString image_;
    QPointF pos_;
    float zoom_ = { 1 };
    float zvalue_ = { 1 };
    DeviceId id_ { 0 };
    int battery_count_ { 0 };
    int battery_capacity_ { 0 };
    SubCategory subcat_;
    ItemCategory category_ { ItemCategory::Device };
    BatteryType battery_type_ { BatteryType::None };
    bool show_title_ { true };
    bool locked_ { false };
    ImageMirrorType mirror_ { ImageMirrorType::None };
    XletsLogicViewData logic_view_data_;
    QList<XletsUserViewData> user_view_data_;
    QString current_user_view_;
    DeviceDataInfo info_;
    qreal weight_ { 0 };
    qreal volume_ { 0 };
    int border_width_ { 0 };
    int view_width_ { 0 };
    int view_height_ { 0 };
    QColor border_color_, background_color_, text_color_;
};

using SharedDeviceData = QSharedDataPointer<DeviceData>;

class DeviceConnectionData {
public:
    SharedDeviceData src_data, dest_data;
    XletData src_out, dest_in;
    XletIndex src_out_idx, dest_in_idx;

    bool isValid() const
    {
        return src_data && dest_data;
    }

    ConnectionId connectionId() const
    {
        return { src_data->id(), src_out_idx, dest_data->id(), dest_in_idx };
    }
};

size_t qHash(const DeviceData& data);

} // namespace ceam

QDebug operator<<(QDebug debug, const ceam::DeviceData& data);

#endif // DEVICE_COMMON_H
