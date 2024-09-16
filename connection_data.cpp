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
#include "connection_data.h"
#include "logging.hpp"

#include <QJsonObject>

namespace {
constexpr const char* KEY_SRC = "src";
constexpr const char* KEY_DEST = "dest";
constexpr const char* KEY_SRC_OUT = "out";
constexpr const char* KEY_DEST_IN = "in";
constexpr const char* KEY_CONNECTION_CORD = "cord";
constexpr const char* JSON_STR_LINE = "line";
constexpr const char* JSON_STR_BEZIER = "bezier";
constexpr const char* JSON_STR_SEGMENTED = "segment";

const char* toString(ceam::ConnectionCordType type)
{
    switch (type) {
    case ceam::ConnectionCordType::Linear:
        return JSON_STR_LINE;
    case ceam::ConnectionCordType::Segmented:
        return JSON_STR_SEGMENTED;
    case ceam::ConnectionCordType::Bezier:
    default:
        return JSON_STR_BEZIER;
    }
}

std::optional<ceam::ConnectionCordType> fromConnectionCordstr(const QString& str)
{
    if (str == JSON_STR_BEZIER || str.isEmpty())
        return ceam::ConnectionCordType::Bezier;
    else if (str == JSON_STR_LINE)
        return ceam::ConnectionCordType::Linear;
    else if (str == JSON_STR_SEGMENTED)
        return ceam::ConnectionCordType::Segmented;
    else {
        WARN() << "unknown cord type:" << str;
        return ceam::ConnectionCordType::Bezier;
    }
}
}

namespace ceam {

QJsonObject ConnectionData::toJson() const
{
    QJsonObject j;

    j[KEY_SRC] = static_cast<int>(src_);
    j[KEY_DEST] = static_cast<int>(dest_);
    j[KEY_DEST_IN] = static_cast<int>(in_);
    j[KEY_SRC_OUT] = static_cast<int>(out_);
    j[KEY_CONNECTION_CORD] = toString(cord_type_);

    return j;
}

bool ConnectionData::setEndPoint(const XletInfo& ep)
{
    switch (ep.type()) {
    case XletType::In:
        in_ = ep.index();
        dest_ = ep.id();
        return true;
    case XletType::Out:
        out_ = ep.index();
        src_ = ep.id();
        return true;
    default:
        return false;
    }
}

std::optional<ConnectionData> ConnectionData::fromJson(const QJsonValue& j)
{
    if (!j.isObject())
        return {};

    ConnectionData data(0, 0, 0, 0);
    auto obj = j.toObject();
    auto src = obj.value(KEY_SRC).toInt(-1);
    if (src >= 0)
        data.src_ = src;

    auto dest = obj.value(KEY_DEST).toInt(-1);
    if (dest >= 0)
        data.dest_ = dest;

    auto in = obj.value(KEY_DEST_IN).toInt(-1);
    if (in >= 0)
        data.in_ = in;

    auto out = obj.value(KEY_SRC_OUT).toInt(-1);
    if (out >= 0)
        data.out_ = out;

    auto cord_type = fromConnectionCordstr(obj.value(KEY_CONNECTION_CORD).toString(JSON_STR_BEZIER));
    if (cord_type)
        data.cord_type_ = *cord_type;

    return data;
}

std::optional<ConnectionData> ConnectionData::fromXletPair(const XletInfo& x0, const XletInfo& x1)
{
    if (x0.type() == XletType::In && x1.type() == XletType::Out)
        return ConnectionData { x1.id(), x1.index(), x0.id(), x0.index() };
    else if (x0.type() == XletType::Out && x1.type() == XletType::In)
        return ConnectionData { x0.id(), x0.index(), x1.id(), x1.index() };
    else
        return {};
}

size_t qHash(const ConnectionData& key)
{
    return ::qHash(key.destination())
        ^ ::qHash(key.destinationInput())
        ^ ::qHash(key.source())
        ^ ::qHash(key.sourceOutput());
}

QDebug operator<<(QDebug debug, const ConnectionData& c)
{
    QDebugStateSaver saver(debug);
    debug.nospace()
        << '['
        << c.source() << ':' << (int)c.sourceOutput()
        << "->"
        << c.destination() << ':' << (int)c.destinationInput()
        << ']';

    return debug;
}

} // namespace ceam
