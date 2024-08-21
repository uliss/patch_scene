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

#include <QJsonArray>
#include <QSharedData>
#include <QString>

#include <functional>

#include "connection.h"
#include "device_xlet.h"

namespace ceam {

enum class ItemCategory : std::uint8_t {
    Device,
    Instrument,
    Human,
    Furniture,
    Send,
    Return,
    MaxCategory,
    // when add new value - update toQString, fromQString, foreachItemCategory functions!
};

const char* toString(ItemCategory cat);
std::optional<ItemCategory> fromQString(const QString& str);
void foreachItemCategory(std::function<void(const char*, int)> fn);

enum class BatteryType : std::uint8_t {
    None,
    AA,
    AAA,
    AAAA,
    PP3_Krona,
    B,
    C,
    A23,
    A27,
    MaxBattery_,
};

const char* toString(BatteryType type);
const char* toJsonString(BatteryType type);
BatteryType fromJsonString(const QString& str);
void foreachBatteryType(std::function<void(const char*, int)> fn);

class BatteryChange {
    BatteryType typeA_ { BatteryType::None }, typeB_ { BatteryType::None };
    int countA_ { 0 }, countB_ { 0 };

public:
    BatteryChange() { }
    BatteryChange(BatteryType typeA, int countA, BatteryType typeB, int countB);

    int typeADelta() const { return countA_; }
    int typeBDelta() const { return countB_; }
    BatteryType typeA() const { return typeA_; }
    BatteryType typeB() const { return typeB_; }

    operator bool() const;
};

class DeviceData : public QSharedData {
public:
    constexpr static const qreal MIN_ZOOM = 0.25;
    constexpr static const qreal MAX_ZOOM = 4;

public:
    explicit DeviceData(DeviceId id);

    bool isNull() const { return id_ == DEV_NULL_ID; }
    DeviceId id() const { return id_; }
    void setId(DeviceId id) { id_ = id; }

    size_t visInputCount() const;
    size_t visOutputCount() const;
    bool hasVisInputs() const;
    bool hasVisOutputs() const;

    bool showInDeviceCategory() const;

    qreal zoom() const { return zoom_; }
    void setZoom(qreal z);

    QString title() const;
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
    void foreachVisInput(std::function<void(XletIndex, const XletData&)> fn);
    const XletData& inputAt(XletIndex n) const;
    void appendInput(const XletData& x) { inputs_.append(x); }

    QList<XletData>& outputs() { return outputs_; }
    const QList<XletData>& outputs() const { return outputs_; }
    void foreachVisOutput(std::function<void(XletIndex, const XletData&)> fn);
    const XletData& outputAt(XletIndex n) const { return outputs_.at(n); }
    void appendOutput(const XletData& x) { outputs_.append(x); }

    const QPointF& pos() const { return pos_; }
    void setPos(const QPointF& pos) { pos_ = pos; }

    int batteryCount() const { return battery_count_; }
    void setBatteryCount(int v);

    BatteryType batteryType() const { return battery_type_; }
    void setBatteryType(int type);
    BatteryChange calcBatteryChange(const DeviceData& data) const;

    size_t calcModelId() const;

    bool showTitle() const { return show_title_; }
    void setShowTitle(bool value) { show_title_ = value; }

private:
    static QJsonArray xletToJson(const QList<XletData>& xlets);
    static bool setXletJson(const QJsonValue& v, QList<XletData>& xlets);

private:
    QList<XletData> inputs_;
    QList<XletData> outputs_;
    QString model_, vendor_, title_;
    QString image_;
    QPointF pos_;
    float zoom_ = { 1 };
    float zvalue_ = { 1 };
    DeviceId id_ { 0 };
    int battery_count_ { 0 };
    ItemCategory category_ { ItemCategory::Device };
    BatteryType battery_type_ { BatteryType::None };
    bool show_title_ { true };
};

using SharedDeviceData = QSharedDataPointer<DeviceData>;

}

QDebug operator<<(QDebug debug, const ceam::DeviceData& data);

#endif // DEVICE_COMMON_H
