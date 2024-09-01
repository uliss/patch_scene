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
#include "device_xlet.h"
#include "device.h"
#include "svg_render_factory.h"

#include <QContextMenuEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSvgItem>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace ceam;

namespace {
constexpr const char* KEY_NAME = "name";
constexpr const char* KEY_PHANTOM = "phantom";
constexpr const char* KEY_VISIBLE = "visible";
constexpr const char* KEY_SOCKET = "socket";
constexpr const char* KEY_MODEL = "model";
constexpr const char* KEY_POWER_TYPE = "power";

constexpr int ICON_W = 16;
constexpr int ICON_H = 16;

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;

constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 3;
}

XletData::XletData(ConnectorModel model)
    : model_(model)
{
}

XletData::XletData(const QString& name, ConnectorModel model)
    : name_(name)
    , model_(model)
{
}

bool XletData::supportsPhantomPower() const
{
    return power_type_ == PowerType::Phantom;
}

QString XletData::modelString() const
{
    return connectorName(model_);
}

QString XletData::iconPath() const
{
    return QString(":/connectors/%1_%2.svg").arg(connectorSvgName(model_), type_.toJsonString());
}

QJsonObject XletData::toJson() const
{
    QJsonObject j;

    j[KEY_NAME] = name_;
    j[KEY_PHANTOM] = phantom_power_;
    j[KEY_VISIBLE] = visible_;
    j[KEY_MODEL] = connectorJsonName(model_);
    j[KEY_SOCKET] = type_.toJson();
    j[KEY_POWER_TYPE] = powerTypeToString(power_type_);

    return j;
}

std::optional<XletData> XletData::fromJson(const QJsonValue& j)
{
    if (!j.isObject()) {
        qWarning() << __FILE__ << __FUNCTION__ << "json object expected, got:" << j;
        return {};
    }

    XletData data;
    auto obj = j.toObject();

    data.name_ = obj.value(KEY_NAME).toString();
    data.visible_ = obj.value(KEY_VISIBLE).toBool(true);
    data.phantom_power_ = obj.value(KEY_PHANTOM).toBool(false);
    data.model_ = findConnectorByJsonName(obj.value(KEY_MODEL).toString());

    auto conn_type = ConnectorType::fromJson(obj.value(KEY_SOCKET));
    if (conn_type)
        data.type_ = conn_type.value();

    auto power_type = powerTypeFromString(obj.value(KEY_POWER_TYPE).toString({}));
    if (power_type)
        data.power_type_ = power_type.value();

    return data;
}

XletData XletData::createSocket(ConnectorModel model, bool female)
{
    XletData res { model };
    res.setConnectorType(female ? ConnectorType::socket_female : ConnectorType::socket_male);
    return res;
}

bool XletData::operator==(const XletData& data) const
{
    return name_ == data.name_
        && model_ == data.model_
        && type_ == data.type_
        && visible_ == data.visible_
        && phantom_power_ == data.phantom_power_
        && power_type_ == data.power_type_;
}

DeviceXlet::DeviceXlet(const XletData& data, XletType type, XletIndex idx, QGraphicsItem* parentItem)
    : QGraphicsObject(parentItem)
    , data_ { data }
    , type_ { type }
    , index_(idx)
    , icon_(new QGraphicsSvgItem(this))
{
    icon_->setPos((XLET_W - ICON_W) * 0.5, 2);
    icon_->setSharedRenderer(SvgRenderFactory::instance().getRender(data_.iconPath()));

    if (data.isPlug() && type == XletType::In)
        icon_->setTransform(QTransform().scale(1, -1).translate(0, -ICON_H));

    updateTooltip();
}

QRectF DeviceXlet::boundingRect() const
{
    return { 0, 0, XLET_W, XLET_H };
}

const XletData& DeviceXlet::xletData() const
{
    return data_;
}

void DeviceXlet::setConnectPoint(const QPointF& pos)
{
    const auto x = pos.x() - (XLET_W / 2);

    if (type_ == XletType::In)
        setPos(x, pos.y());
    else
        setPos(x, pos.y() - XLET_H);
}

void DeviceXlet::setIndex(XletIndex idx)
{
    index_ = idx;
    updateTooltip();
}

const Device* DeviceXlet::parentDevice() const
{
    return qgraphicsitem_cast<Device*>(parentItem());
}

void DeviceXlet::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    if (data_.supportsPhantomPower()) {
        QMenu menu;

        auto phantom_on = new QAction(tr("Phantom"), &menu);
        phantom_on->setCheckable(true);
        phantom_on->setChecked(data_.isPhantomOn());

        connect(phantom_on, &QAction::triggered, this, [this](bool checked) {
            data_.setPhantom(checked);
            update(boundingRect());
            auto dev = qgraphicsitem_cast<Device*>(parentItem());
            if (dev)
                dev->syncXletData();
        });

        menu.addAction(phantom_on);
        menu.exec(event->screenPos());
        event->accept();
    } else
        event->ignore();
}

void DeviceXlet::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    const auto bbox = boundingRect();
    painter->setPen(Qt::NoPen);

    switch (data_.powerType()) {
    case PowerType::DC_Positive:
        painter->setBrush(Qt::red);
        painter->drawRect(bbox);
        break;
    case PowerType::DC_Negative:
        painter->setBrush(Qt::blue);
        painter->drawRect(bbox);
        break;
    case PowerType::AC:
        painter->setBrush(QColor(255, 127, 0));
        painter->drawRect(bbox);
        break;
    case PowerType::AC_DC:
        painter->setBrush(Qt::darkMagenta);
        painter->drawRect(bbox);
        break;
    case PowerType::Phantom: {
        painter->setBrush(Qt::NoBrush);
        QPen pen(Qt::darkRed, 1);
        pen.setJoinStyle(Qt::MiterJoin);
        painter->setPen(pen);
        painter->drawRect(bbox.adjusted(1, 1, -1, -1));
    } break;
    case PowerType::None:
    default:
        break;
    }

    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform());
    if (lod >= 0.5) {
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);

        switch (type_) {
        case XletType::In:
            if (data_.isPhantomOn())
                painter->setBrush(Qt::red);

            painter->drawRect((bbox.width() - XLET_BOX_W) * 0.5, 0, XLET_BOX_W, XLET_BOX_H);
            break;
        case XletType::Out:
            painter->drawRect((bbox.width() - XLET_BOX_W) * 0.5, bbox.bottom() - XLET_BOX_H, XLET_BOX_W, XLET_BOX_H);
            break;
        default:
            break;
        }
    }
}

void DeviceXlet::updateTooltip()
{
    QString prefix;
    if (index_ != XLET_INDEX_NONE)
        prefix = QString("[%1] ").arg(index_ + 1);

    if (!data_.name().isEmpty())
        setToolTip(QString("%1%2: %3").arg(prefix, data_.modelString(), data_.name()));
    else
        setToolTip(prefix + data_.modelString());
}

size_t ceam::qHash(const XletData& data)
{
    return ::qHash(data.name())
        ^ ::qHash((int)data.connectorModel())
        ^ ::qHash(data.connectorType().toInt())
        ^ ::qHash(data.isVisible())
        ^ ::qHash(data.isPhantomOn())
        ^ ::qHash((int)data.powerType());
}
