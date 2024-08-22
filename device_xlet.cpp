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

#include <QJsonObject>
#include <QPainter>

using namespace ceam;

namespace {
constexpr const char* KEY_LEVEL = "level";
constexpr const char* KEY_NAME = "name";
constexpr const char* KEY_PHANTOM = "phantom";
constexpr const char* KEY_VISIBLE = "visible";
constexpr const char* KEY_SOCKET = "socket";
constexpr const char* KEY_MODEL = "model";
constexpr const char* KEY_POWER_TYPE = "power";

constexpr const char* SOCKET_MALE = "male";
constexpr const char* SOCKET_FEMALE = "female";
constexpr const char* PLUG_MALE = "plug_male";
constexpr const char* PLUG_FEMALE = "plug_female";

QString xlet_icon_path(ConnectorModel model, ConnectorType type)
{
    switch (type) {
    case ConnectorType::Socket_Male:
        return QString(":/connectors/%1_socket_male.svg").arg(connectorSvgName(model));
    case ConnectorType::Socket_Female:
        return QString(":/connectors/%1_socket.svg").arg(connectorSvgName(model));
    case ConnectorType::Plug_Male:
        return QString(":/connectors/%1_plug_male.svg").arg(connectorSvgName(model));
    case ConnectorType::Plug_Female:
    default:
        return {};
    }
}

ConnectorType connector_type(const QString& type)
{
    if (type == SOCKET_MALE) {
        return ConnectorType::Socket_Male;
    } else if (type == PLUG_MALE) {
        return ConnectorType::Plug_Male;
    } else if (type == PLUG_FEMALE) {
        return ConnectorType::Plug_Female;
    } else {
        return ConnectorType::Socket_Female;
    }
}
}

QString XletData::modelString() const
{
    return connectorName(model);
}

QString XletData::typeString() const
{
    switch (type) {
    case ConnectorType::Socket_Male:
        return SOCKET_MALE;
    case ConnectorType::Socket_Female:
        return SOCKET_FEMALE;
    case ConnectorType::Plug_Male:
        return PLUG_MALE;
    case ConnectorType::Plug_Female:
        return PLUG_FEMALE;
    default:
        return "";
    }
}

QJsonObject XletData::toJson() const
{
    QJsonObject j;

    j[KEY_NAME] = name;
    j[KEY_LEVEL] = level;
    j[KEY_PHANTOM] = phantom_power;
    j[KEY_VISIBLE] = visible;
    j[KEY_MODEL] = connectorJsonName(model);
    j[KEY_SOCKET] = typeString();
    j[KEY_POWER_TYPE] = powerTypeToString(power_type);

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

    data.name = obj.value(KEY_NAME).toString();
    data.visible = obj.value(KEY_VISIBLE).toBool(true);
    data.phantom_power = obj.value(KEY_PHANTOM).toBool(false);
    data.level = obj.value(KEY_LEVEL).toString();
    data.model = findConnectorByJsonName(obj.value(KEY_MODEL).toString());
    data.type = connector_type(obj.value(KEY_SOCKET).toString(""));
    auto power_type = powerTypeFromString(obj.value(KEY_POWER_TYPE).toString({}));
    if (power_type)
        data.power_type = power_type.value();

    return data;
}

bool XletData::isSocket() const
{
    return connectorIsSocket(type);
}

bool XletData::isPlug() const
{
    return connectorIsPlug(type);
}

DeviceXlet::DeviceXlet(const XletData& data, XletType type, QGraphicsItem* parentItem)
    : QGraphicsSvgItem(xlet_icon_path(data.model, data.type), parentItem)
    , data_ { data }
    , type_ { type }
{
    if (!data.name.isEmpty())
        setToolTip(connectorName(data.model) + ": " + data.name);
    else
        setToolTip(connectorName(data.model));
}

const XletData& DeviceXlet::xletData() const
{
    return data_;
}

QString DeviceXlet::iconPath() const
{
    return xlet_icon_path(data_.model, data_.type);
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
