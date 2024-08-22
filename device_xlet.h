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
#ifndef DEVICE_XLET_H
#define DEVICE_XLET_H

#include "socket.h"

#include <QGraphicsSvgItem>

namespace ceam {

class XletData {
    QString name_;
    ConnectorModel model_ { ConnectorModel::UNKNOWN };
    ConnectorType type_ { ConnectorType::Socket_Female };
    bool visible_ { true };
    bool phantom_power_ { false };
    PowerType power_type_ { PowerType::None };

public:
    XletData() { }
    explicit XletData(ConnectorModel model);
    XletData(const QString& name, ConnectorModel model);

    const QString& name() const { return name_; }
    void setName(const QString& name) { name_ = name; }

    ConnectorModel connectorModel() const { return model_; }
    void setConnectorModel(ConnectorModel model) { model_ = model; }

    ConnectorType connectorType() const { return type_; }
    void setConnectorType(ConnectorType type) { type_ = type; }

    bool isVisible() const { return visible_; }
    void setVisible(bool value) { visible_ = value; }

    bool supportsPhantomPower() const;
    bool isPhantomOn() const { return phantom_power_; }
    void setPhantom(bool on) { phantom_power_ = on; }

    PowerType powerType() const { return power_type_; }

    QString modelString() const;
    QString typeString() const;
    QString iconPath() const;

    QJsonObject toJson() const;

    static std::optional<XletData> fromJson(const QJsonValue& j);

    bool operator==(const XletData& data) const;
    bool operator!=(const XletData& data) const { return !operator==(data); };

    bool isSocket() const;
    bool isPlug() const;
};

class DeviceXlet : public QGraphicsSvgItem {
public:
    DeviceXlet(const XletData& data, XletType type, QGraphicsItem* parentItem);

    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    const XletData& xletData() const;
    XletType xletType() const { return type_; }
    QString iconPath() const;
    void setConnectPoint(const QPointF& pos);

    bool isInlet() const { return type_ == XletType::In; }
    bool isOutlet() const { return type_ == XletType::Out; }

private:
    XletData data_;
    XletType type_ { XletType::In };
};

}

#endif // DEVICE_XLET_H
