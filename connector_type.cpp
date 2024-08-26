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
#include "connector_type.h"

namespace {
constexpr const char* SOCKET_MALE_OLD = "male";
constexpr const char* SOCKET_FEMALE_OLD = "female";
constexpr const char* SOCKET_MALE = "socket_male";
constexpr const char* SOCKET_FEMALE = "socket_female";
constexpr const char* PLUG_MALE = "plug_male";
constexpr const char* PLUG_FEMALE = "plug_female";
}

namespace ceam {

const ConnectorType ConnectorType::socket_female(ConnectorType::SocketFemale);
const ConnectorType ConnectorType::socket_male(ConnectorType::SocketMale);
const ConnectorType ConnectorType::plug_female(ConnectorType::PlugFemale);
const ConnectorType ConnectorType::plug_male(ConnectorType::PlugMale);

QString ConnectorType::iconPath() const
{
    switch (type_) {
    case SocketMale:
    case SocketFemale:
        return ":/connectors/conn_socket.svg";
    case PlugMale:
    case PlugFemale:
        return ":/connectors/conn_plug.svg";
    default:
        return {};
    }
}

QString ConnectorType::localizedName() const
{
    switch (type_) {
    case SocketFemale:
        return tr("Female");
    case PlugMale:
        return tr("Male");
    case ConnectorType::PlugFemale:
        return tr("Female");
    case ConnectorType::SocketMale:
    default:
        return tr("Male");
    }
}

QString ConnectorType::toJsonString() const
{
    switch (type_) {
    case SocketMale:
        return SOCKET_MALE;
    case SocketFemale:
        return SOCKET_FEMALE;
    case PlugMale:
        return PLUG_MALE;
    case PlugFemale:
        return PLUG_FEMALE;
    default:
        return {};
    }
}

QJsonValue ConnectorType::toJson() const
{
    return toJsonString();
}

int ConnectorType::toInt() const
{
    return type_;
}

bool ConnectorType::isSocket() const
{
    return type_ == SocketFemale || type_ == SocketMale;
}

bool ConnectorType::isPlug() const
{
    return type_ == PlugFemale || type_ == PlugMale;
}

ConnectorType ConnectorType::complement() const
{
    switch (type_) {
    case ConnectorType::SocketMale:
        return ConnectorType(PlugFemale);
    case ConnectorType::SocketFemale:
        return ConnectorType(PlugMale);
    case ConnectorType::PlugMale:
        return ConnectorType(SocketFemale);
    case ConnectorType::PlugFemale:
        return ConnectorType(SocketMale);
    default:
        return ConnectorType(SocketFemale);
    }
}

std::optional<ConnectorType> ConnectorType::fromJson(const QJsonValue& val)
{
    if (!val.isString())
        return {};

    const auto str = val.toString().toLower();

    if (str == SOCKET_MALE || str == SOCKET_MALE_OLD)
        return ConnectorType(SocketMale);
    else if (str == SOCKET_FEMALE || str == SOCKET_FEMALE_OLD)
        return ConnectorType(SocketFemale);
    else if (str == PLUG_FEMALE)
        return ConnectorType(PlugFemale);
    else if (str == PLUG_MALE)
        return ConnectorType(PlugMale);
    else
        return {};
}

std::optional<ConnectorType> ConnectorType::fromInt(int val)
{
    if (val < SocketMale || val >= MaxConnectorType)
        return {};

    return ConnectorType(static_cast<Type>(val));
}

void ConnectorType::foreachType(const std::function<void(const ConnectorType&)> fn)
{
    for (int i = SocketMale; i < MaxConnectorType; i++)
        fn(ConnectorType(static_cast<Type>(i)));
}

} // namespace ceam
