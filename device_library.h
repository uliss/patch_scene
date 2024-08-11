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
#include "device_xlet.h"

#include <QJsonObject>
#include <QString>

class DeviceInfo {
    QString vendor_, model_, title_, image_;
    QList<XletData> inputs_, outputs_;
    qreal zoom_;
    ItemCategory category_;
    QColor bg_color_ { Qt::white };

public:
    DeviceInfo();

    qreal zoom() const { return zoom_; }
    void setZoom(qreal z);

    QString title() const;
    void setTitle(const QString& title) { title_ = title; }

    QString vendor() const { return vendor_; }
    void setVendor(const QString& vendor) { vendor_ = vendor; }

    QString model() const { return model_; }
    void setModel(const QString& model) { model_ = model; }

    QString image() const { return image_; }
    void setImage(const QString& image) { image_ = image; }

    ItemCategory category() const { return category_; }
    void setCategory(ItemCategory cat) { category_ = cat; }
    bool setCategory(const QString& cat);
    QString categoryName() const;

    const QColor& bgColor() const { return bg_color_; }

    bool setJson(const QJsonObject& obj);
    QJsonObject toJson() const;

    QList<XletData>& inputs() { return inputs_; }
    const QList<XletData>& inputs() const { return inputs_; }

    QList<XletData>& outputs() { return outputs_; }
    const QList<XletData>& outputs() const { return outputs_; }
};

class DeviceLibrary {
public:
    DeviceLibrary();

    bool readFile(const QString& filename);

    const QList<DeviceInfo>& devices() const { return devices_; }
    const QList<DeviceInfo>& instruments() const { return instruments_; }

    const QList<DeviceInfo>& sends() const { return sends_; }
    const QList<DeviceInfo>& returns() const { return returns_; }

private:
    bool readDevices(const QJsonValue& devs);
    bool readInstruments(const QJsonValue& instr);
    bool readSends(const QJsonValue& sends);
    bool readReturns(const QJsonValue& returns);

private:
    QList<DeviceInfo> devices_, instruments_, sends_, returns_;
};

#endif // DEVICE_LIBRARY_H
