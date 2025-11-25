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
#include "device_socket.h"
#include "user_item_types.h"
#include "xlet_info.h"

#include <QGraphicsRectItem>

class QContextMenuEvent;
class QGraphicsSvgItem;

namespace ceam {

class SceneItem;

class XletData {
    QString name_;
    ConnectorModel model_ { ConnectorModel::UNKNOWN };
    ConnectorType type_ { ConnectorType::socket_female };
    bool visible_ { true };
    bool phantom_power_ { false };
    PowerType power_type_ { PowerType::None };
    bool bidirect_ { false };

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

    bool supportsPhantomPower() const;
    bool isPhantomOn() const { return phantom_power_; }
    void setPhantom(bool on) { phantom_power_ = on; }

    PowerType powerType() const { return power_type_; }
    void setPowerType(PowerType type) { power_type_ = type; }

    bool isBidirect() const { return bidirect_; }
    void setBidirect(bool value) { bidirect_ = value; }

    QString modelString() const;
    QString iconPath() const;

    bool operator==(const XletData& data) const;
    bool operator!=(const XletData& data) const { return !operator==(data); };

    bool isSocket() const { return type_.isSocket(); }
    bool isPlug() const { return type_.isPlug(); }

    QJsonObject toJson() const;

public:
    static std::optional<XletData> fromJson(const QJsonValue& j);
    static XletData createSocket(ConnectorModel model, bool female = true);
    static QString defaultName(XletType type, XletIndex idx);
};

size_t qHash(const XletData& data);

class DeviceXlet : public QGraphicsObject {
    Q_OBJECT
public:
    enum { Type = UserItemTypeXlet };
    int type() const override { return Type; }

public:
    DeviceXlet(const XletData& data, const XletInfo& info, QGraphicsItem* parentItem);

    QRectF boundingRect() const final;

    const XletData& xletData() const;
    XletInfo xletInfo() const;
    const SceneItem* parentDevice() const;

    void setDragMode(bool value, bool selfDrag = false);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;

private:
    void updateTooltip();

private:
    XletData data_;
    QGraphicsSvgItem* icon_ { nullptr };
    XletInfo info_ { SCENE_ITEM_NULL_ID, XLET_INDEX_NONE, XletType::In };
    bool drag_mode_ { false }, self_drag_ { false };
};

} // namespace ceam

#endif // DEVICE_XLET_H
