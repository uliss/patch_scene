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

#include <QJsonArray>
#include <QJsonObject>
#include <QRectF>

namespace {

constexpr int SEGMENT_SRC_CONN_YPAD = 5;
constexpr int SEGMENT_DEST_CONN_YPAD = 25;

constexpr qreal PEN_WIDTH_MIN = 1;
constexpr qreal PEN_WIDTH_DEF = 1.5;
constexpr qreal PEN_WIDTH_MAX = 5;

constexpr const char* KEY_BEZY0 = "bezy0";
constexpr const char* KEY_BEZY1 = "bezy1";
constexpr const char* KEY_SRC_PT = "src";
constexpr const char* KEY_DEST_PT = "dest";
constexpr const char* KEY_DEST = "dest";
constexpr const char* KEY_DEST_IN = "in";
constexpr const char* KEY_SRC = "src";
constexpr const char* KEY_SRC_OUT = "out";
constexpr const char* KEY_COLOR = "color";
constexpr const char* KEY_X = "x";
constexpr const char* KEY_Y = "y";
constexpr const char* KEY_WIDTH = "width";
constexpr const char* KEY_SEGMENTS = "segments";
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

bool betweenX(const QPoint& a, const QPoint& pt, const QPoint& b)
{
    if (a.x() <= b.x())
        return pt.x() >= a.x() && pt.x() <= b.x();
    else
        return pt.x() >= b.x() && pt.x() <= a.x();
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

ConnectionViewData::ConnectionViewData()
    : color_(Qt::black), pen_width_(PEN_WIDTH_DEF)
{
}

void ConnectionViewData::clearSegments()
{
    segs_.clear();
}

SegmentPoints ConnectionViewData::makeSegments() const
{
    SegmentPoints res;

    if (pt0_.y() + SEGMENT_DEST_CONN_YPAD <= pt1_.y()) {
        res.append({ pt1_.x(), (pt1_.y() + pt0_.y()) / 2 });
    } else {
        res.append(QPoint((pt1_.x() + pt0_.x()) / 2, pt0_.y() + SEGMENT_SRC_CONN_YPAD));
        res.append(QPoint((pt1_.x() + pt0_.x()) / 2, pt1_.y() - SEGMENT_DEST_CONN_YPAD));
    }

    return res;
}

bool ConnectionViewData::setSegmentPoint(int idx, const QPoint& pt)
{
    return segs_.setPoint(idx, pt);
}

bool ConnectionViewData::removeSegmentPoint(int idx)
{
    return segs_.removePoint(idx);
}

void ConnectionViewData::createSegments()
{
    segs_ = makeSegments();
}

bool ConnectionViewData::splitSegment(const QPointF& pos)
{
    if (cord_type_ != ConnectionCordType::Segmented)
        return false;

    if (segs_.isEmpty())
        createSegments();

    return segs_.splitAt(pos.toPoint(), pt0_, pt1_);
}

void ConnectionViewData::resetPoints(ConnectionCordType cord)
{
    switch (cord) {
    case ConnectionCordType::Bezier:
        bezy0_ = { 0, BEZY_YOFF };
        bezy1_ = { 0, -BEZY_YOFF };
        break;
    case ConnectionCordType::Segmented:
        clearSegments();
        break;
    default:
        break;
    }
}

QJsonObject ConnectionViewData::toJson() const
{
    QJsonObject j;

    j[KEY_CONNECTION_CORD] = toString(cord_type_);
    j[KEY_BEZY0] = pointToJson(bezy0_);
    j[KEY_BEZY1] = pointToJson(bezy1_);
    j[KEY_SRC_PT] = pointToJson(pt0_);
    j[KEY_DEST_PT] = pointToJson(pt1_);
    j[KEY_SEGMENTS] = segs_.toJson();
    j[KEY_COLOR] = color_.name(QColor::HexRgb);
    j[KEY_WIDTH] = pen_width_;

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

    auto segs = SegmentPoints::fromJson(obj.value(KEY_SEGMENTS));
    if (segs)
        data.setSegments(*segs);

    auto color = QColor::fromString(obj.value(KEY_COLOR).toString("#000"));
    if (color.isValid())
        data.setColor(color);

    auto pen_wd = qBound<qreal>(PEN_WIDTH_MIN, obj.value(KEY_WIDTH).toDouble(PEN_WIDTH_DEF), PEN_WIDTH_MAX);
    data.setPenWidth(pen_wd);

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

bool SegmentPoints::setPoint(int idx, const QPoint& pt)
{
    if (idx < 0 || idx >= points_.size())
        return false;

    points_[idx] = pt;
    return true;
}

bool SegmentPoints::removePoint(int idx)
{
    if (idx < 0 || idx >= points_.size())
        return false;

    points_.remove(idx);
    return true;
}

QList<std::pair<QPoint, bool>> SegmentPoints::makePointList(const QPoint& from, const QPoint& to) const
{
    QList<std::pair<QPoint, bool>> res;
    res << std::make_pair(from, false);

    auto prev_pt = from;
    for (auto& pt : points_) {
        if (prev_pt.y() != pt.y()) {
            auto new_pt = QPoint { prev_pt.x(), pt.y() };
            res << std::make_pair(new_pt, new_pt == pt);
        }

        if (prev_pt.x() != pt.x()) {
            auto new_pt = QPoint { pt.x(), pt.y() };
            res << std::make_pair(new_pt, new_pt == pt);
        }

        prev_pt = pt;
    }

    if (res.size() > 1) {
        auto N = res.size();
        if (betweenX(res[N - 2].first, prev_pt, to)) {
            if (prev_pt.x() != to.x())
                res << std::make_pair(QPoint { to.x(), prev_pt.y() }, false);

            if (prev_pt.y() != to.y())
                res << std::make_pair(QPoint { to.x(), to.y() }, false);

        } else {
            res << std::make_pair(QPoint { prev_pt.x(), prev_pt.y() + 20 }, false);
            res << std::make_pair(QPoint { to.x(), prev_pt.y() + 20 }, false);
            res << std::make_pair(QPoint { to.x(), to.y() }, false);
        }
    }

    return res;
}

bool SegmentPoints::splitAt(const QPoint& pos, const QPoint& from, const QPoint& to)
{
    auto points = makePointList(from, to);

    QList<QPoint> new_points;
    for (int i = 0; (i + 1) < points.size(); i++) {
        auto p0 = points[i];
        auto p1 = points[i + 1];

        auto segment = QRect(p0.first, p1.first).normalized().adjusted(-2, -2, 2, 2);
        if (p0.second)
            new_points.append(p0.first);

        if (segment.contains(pos)) {
            auto norm_pos = (p0.first.x() == p1.first.x())
                ? QPoint { p0.first.x(), pos.y() }
                : QPoint { pos.x(), p0.first.y() };

            new_points.append(norm_pos);
        }
    }

    if (new_points.isEmpty())
        return false;

    points_ = new_points;
    return true;
}

QJsonValue SegmentPoints::toJson() const
{
    if (points_.empty())
        return {};

    QJsonArray arr;
    for (auto v : points_)
        arr << pointToJson(v);

    return arr;
}

std::optional<SegmentPoints> SegmentPoints::fromJson(const QJsonValue& v)
{
    if (v.isNull())
        return {};

    if (!v.isArray()) {
        WARN() << "array expected";
        return {};
    }

    SegmentPoints res;

    for (auto&& x : v.toArray()) {
        if (x.isObject()) {
            auto pt = pointFromJson(x);
            if (pt)
                res.append(*pt);
            else
                WARN() << "point expected, got:" << x;
        } else {
            WARN() << "object expected, got:" << x;
        }
    }

    return res;
}

} // namespace ceam
