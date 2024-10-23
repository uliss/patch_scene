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

constexpr int NO_INDEX = -1;

QList<QPoint> SegmentPoints::makePointList(const QPoint& from, const QPoint& to, QList<int>* pointIndexes) const
{
    QList<QPoint> res;
    res << from;
    if (pointIndexes)
        pointIndexes->append(NO_INDEX);

    auto prev_pt = from;
    int idx = 0;
    for (auto& pt : points_) {
        if (prev_pt.y() != pt.y()) {
            res << QPoint { prev_pt.x(), pt.y() };
            if (pointIndexes)
                pointIndexes->append(idx);
        }

        if (prev_pt.x() != pt.x()) {
            res << QPoint { pt.x(), pt.y() };
            if (pointIndexes)
                pointIndexes->append(idx);
        }

        prev_pt = pt;
        idx++;
    }

    if (res.size() > 1) {
        auto N = res.size();
        if (betweenX(res[N - 2], prev_pt, to)) {
            if (prev_pt.x() != to.x())
                res << QPoint { to.x(), prev_pt.y() };

            if (prev_pt.y() != to.y())
                res << QPoint { to.x(), to.y() };

        } else {
            res << QPoint { prev_pt.x(), prev_pt.y() + 20 };
            res << QPoint { to.x(), prev_pt.y() + 20 };
            res << QPoint { to.x(), to.y() };
        }
    }

    return res;
}

bool SegmentPoints::splitAt(const QPoint& pos, const QPoint& from, const QPoint& to)
{
    QList<int> point_indexes;
    auto points = makePointList(from, to, &point_indexes);

    for (int i = 0; (i + 1) < points.size(); i++) {
        auto p0 = points[i];
        auto p1 = points[i + 1];

        auto segment = QRect(p0, p1).normalized().adjusted(-2, -2, 2, 2);
        if (segment.contains(pos)) {
            if (i < point_indexes.size()) {
                auto idx = point_indexes[i];
                if (idx >= 0 || idx < points_.size()) {
                    bool is_vertical = (i % 2 == 0);

                    points_.insert(idx + int(is_vertical), pos);
                    return true;
                }
            } else { // last segments
                points_.append(pos);
                return true;
            }

            return false;
        }
    }

    return false;
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
