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

#include <QContextMenuEvent>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>

using namespace ceam;

namespace {
constexpr const char* KEY_NAME = "name";
constexpr const char* KEY_PHANTOM = "phantom";
constexpr const char* KEY_VISIBLE = "visible";
constexpr const char* KEY_SOCKET = "socket";
constexpr const char* KEY_MODEL = "model";
constexpr const char* KEY_POWER_TYPE = "power";
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

bool XletData::operator==(const XletData& data) const
{
    return name_ == data.name_
        && model_ == data.model_
        && type_ == data.type_
        && visible_ == data.visible_
        && phantom_power_ == data.phantom_power_
        && power_type_ == data.power_type_;
}

DeviceXlet::DeviceXlet(const XletData& data, XletType type, QGraphicsItem* parentItem)
    : QGraphicsSvgItem(data.iconPath(), parentItem)
    , data_ { data }
    , type_ { type }
{
    if (!data.name().isEmpty())
        setToolTip(data.modelString() + ": " + data.name());
    else
        setToolTip(data.modelString());

    if (data.isPlug() && type == XletType::In)
        setTransform(QTransform().scale(1, -1).translate(0, -16));
}

const XletData& DeviceXlet::xletData() const
{
    return data_;
}

QString DeviceXlet::iconPath() const
{
    return data_.iconPath();
}

void DeviceXlet::setConnectPoint(const QPointF& pos)
{
    constexpr auto PAD = 2;
    auto wd = 16;
    auto ht = 16;
    auto bbox = boundingRect();

    if (type_ == XletType::In)
        setPos(pos + QPointF(-wd / 2, PAD));
    else
        setPos(pos + QPointF(-wd / 2, -(bbox.height() + PAD)));
}

void DeviceXlet::contextMenuEvent(QContextMenuEvent* event, const QPoint& globalPos)
{
    QMenu menu;
    menu.addAction(new QAction("Context"));
    menu.exec(globalPos);
}
