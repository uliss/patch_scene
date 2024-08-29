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

#include "connector_type.h"
#include "socket.h"

#include <QGraphicsRectItem>

class QContextMenuEvent;
class QGraphicsSvgItem;

namespace ceam {

class XletData {
    QString name_;
    ConnectorModel model_ { ConnectorModel::UNKNOWN };
    ConnectorType type_ { ConnectorType::socket_female };
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

    const ConnectorType& connectorType() const { return type_; }
    void setConnectorType(ConnectorType type) { type_ = type; }

    bool isVisible() const { return visible_; }
    void setVisible(bool value) { visible_ = value; }

    bool supportsPhantomPower() const;
    bool isPhantomOn() const { return phantom_power_; }
    void setPhantom(bool on) { phantom_power_ = on; }

    PowerType powerType() const { return power_type_; }
    void setPowerType(PowerType type) { power_type_ = type; }

    QString modelString() const;
    QString iconPath() const;

    QJsonObject toJson() const;

    static std::optional<XletData> fromJson(const QJsonValue& j);

    bool operator==(const XletData& data) const;
    bool operator!=(const XletData& data) const { return !operator==(data); };

    bool isSocket() const { return type_.isSocket(); }
    bool isPlug() const { return type_.isPlug(); }
};

class DeviceXlet : public QGraphicsObject {
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

public:
    DeviceXlet(const XletData& data, XletType type, QGraphicsItem* parentItem);

    QRectF boundingRect() const final;

    const XletData& xletData() const;
    XletType xletType() const { return type_; }

    bool isInlet() const { return type_ == XletType::In; }
    bool isOutlet() const { return type_ == XletType::Out; }

    void setConnectPoint(const QPointF& pos);

    XletIndex index() const { return index_; }
    void setIndex(XletIndex idx);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;

private:
    void updateTooltip();

private:
    XletData data_;
    QGraphicsSvgItem* icon_ { nullptr };
    XletType type_ { XletType::In };
    XletIndex index_ { XLET_INDEX_NONE };
};

}

#endif // DEVICE_XLET_H
