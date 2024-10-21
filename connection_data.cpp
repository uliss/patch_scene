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

constexpr const char* KEY_BEZY0 = "bezy0";
constexpr const char* KEY_BEZY1 = "bezy1";
constexpr const char* KEY_SRC_PT = "src";
constexpr const char* KEY_DEST_PT = "dest";
constexpr const char* KEY_DEST = "dest";
constexpr const char* KEY_DEST_IN = "in";
constexpr const char* KEY_SRC = "src";
constexpr const char* KEY_SRC_OUT = "out";
constexpr const char* KEY_X = "x";
constexpr const char* KEY_Y = "y";
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

std::optional<ceam::ConnectionCordType> fromConnectionCordStr(const QString& str)
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

QJsonObject pointToJson(const QPoint& pt)
{
    QJsonObject res;
    res[KEY_X] = pt.x();
    res[KEY_Y] = pt.y();
    return res;
}

std::optional<QPoint> pointFromJson(const QJsonValue& v)
{
    if (!v.isObject())
        return {};

    auto js = v.toObject();
    return QPoint { v[KEY_X].toInt(), v[KEY_Y].toInt() };
}
}

namespace ceam {

bool ConnectionId::isValid() const
{
    return src_ != dest_;
}

QJsonObject ConnectionId::toJson() const
{
    QJsonObject j;

    j[KEY_SRC] = static_cast<int>(src_);
    j[KEY_DEST] = static_cast<int>(dest_);
    j[KEY_DEST_IN] = static_cast<int>(in_);
    j[KEY_SRC_OUT] = static_cast<int>(out_);

    return j;
}

bool ConnectionId::setEndPoint(const XletInfo& ep)
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

void ConnectionViewData::appendSegment(float seg)
{
    segs_.append(seg);
}

void ConnectionViewData::clearSegments()
{
    segs_.clear();
}

QJsonObject ConnectionViewData::toJson() const
{
    QJsonObject j;

    j[KEY_CONNECTION_CORD] = toString(cord_type_);
    j[KEY_BEZY0] = pointToJson(bezy0_);
    j[KEY_BEZY1] = pointToJson(bezy1_);
    j[KEY_SRC_PT] = pointToJson(pt0_);
    j[KEY_DEST_PT] = pointToJson(pt1_);

    return j;
}

std::optional<ConnectionViewData> ConnectionViewData::fromJson(const QJsonValue& j)
{
    if (!j.isObject())
        return {};

    auto obj = j.toObject();

    ConnectionViewData data;

    auto cord_type = fromConnectionCordStr(obj.value(KEY_CONNECTION_CORD).toString(JSON_STR_BEZIER));
    if (cord_type)
        data.cord_type_ = *cord_type;

    auto bezy0 = pointFromJson(obj.value(KEY_BEZY0));
    if (bezy0)
        data.setBezyCtlPoint0(*bezy0);

    auto bezy1 = pointFromJson(obj.value(KEY_BEZY1));
    if (bezy1)
        data.setBezyCtlPoint1(*bezy1);

    auto pt0 = pointFromJson(obj.value(KEY_SRC_PT));
    if (pt0)
        data.setSourcePoint(*pt0);

    auto pt1 = pointFromJson(obj.value(KEY_DEST_PT));
    if (pt1)
        data.setDestinationPoint(*pt1);

    return data;
}

std::optional<ConnectionId> ConnectionId::fromJson(const QJsonValue& j)
{
    if (!j.isObject())
        return {};

    ConnectionId data(0, 0, 0, 0);
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

    return data;
}

std::optional<ConnectionId> ConnectionId::fromXletPair(const XletInfo& x0, const XletInfo& x1)
{
    if (x0.type() == XletType::In && x1.type() == XletType::Out)
        return ConnectionId { x1.id(), x1.index(), x0.id(), x0.index() };
    else if (x0.type() == XletType::Out && x1.type() == XletType::In)
        return ConnectionId { x0.id(), x0.index(), x1.id(), x1.index() };
    else
        return {};
}

size_t qHash(const ConnectionId& key)
{
    return ::qHash(key.destination())
        ^ ::qHash(key.destinationInput())
        ^ ::qHash(key.source())
        ^ ::qHash(key.sourceOutput());
}

QDebug operator<<(QDebug debug, const ConnectionId& c)
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

std::optional<QPointF> SegmentData::pointAt(int idx, const QPointF& origin) const
{
    if (idx < 0 || idx >= segs_.size())
        return {};

    return (idx % 2 == 0)
        ? QPointF(idx == 0 ? 0 : segs_[idx - 1], segs_[idx]) + origin
        : QPointF(segs_[idx], segs_[idx - 1]) + origin;
}

void SegmentData::append(float seg)
{
    segs_ << seg;
}

} // namespace ceam
