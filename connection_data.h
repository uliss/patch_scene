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
#ifndef CONNECTION_DATA_H
#define CONNECTION_DATA_H

#include "xlet_info.h"

#include <QColor>
#include <QPoint>

namespace ceam {

enum class ConnectionCordType : std::uint8_t {
    Linear,
    Bezier,
    Segmented
};

class SegmentData {
    QList<float> segs_;

public:
    SegmentData() { }
    void clear() { segs_.clear(); }
    float at(int pos) const { return segs_[pos]; }
    std::optional<QPointF> pointAt(int idx, const QPointF& origin) const;
    std::optional<QPointF> midPointAt(int idx, const QPointF& origin) const;
    void append(float seg);
    bool isEmpty() const { return segs_.empty(); }
    qsizetype size() const { return segs_.size(); }
    bool setPos(int idx, const QPointF& pos);

    QJsonValue toJson() const;

public:
    static std::optional<SegmentData> fromJson(const QJsonValue& v);
};

class ConnectionId {
    DeviceId src_ { 0 }, dest_ { 0 };
    XletIndex out_ { 0 }, in_ { 0 };

public:
    ConnectionId(DeviceId src, XletIndex out, DeviceId dest, XletIndex in)
        : src_(src)
        , out_(out)
        , dest_(dest)
        , in_(in)
    {
    }

    DeviceId source() const { return src_; }
    DeviceId destination() const { return dest_; }
    XletIndex sourceOutput() const { return out_; }
    XletIndex destinationInput() const { return in_; }

    XletInfo sourceInfo() const { return { src_, out_, XletType::Out }; }
    XletInfo destinationInfo() const { return { dest_, in_, XletType::In }; }

    const bool operator==(const ConnectionId& id) const
    {
        return id.src_ == src_
            && id.dest_ == dest_
            && id.in_ == in_
            && id.out_ == out_;
    }

    bool operator!=(const ConnectionId& id) const { return !operator==(id); }

    bool isValid() const;

    /**
     * Set connection source or destination point, according to given XletInfo
     * @return true on success, false on error
     */
    bool setEndPoint(const XletInfo& ep);

    /**
     * converts to Json object
     */
    QJsonObject toJson() const;

public:
    static std::optional<ConnectionId> fromJson(const QJsonValue& j);
    static std::optional<ConnectionId> fromXletPair(const XletInfo& x0, const XletInfo& x1);
};

class ConnectionViewData {
    static const int BEZY_YOFF = 40;

private:
    SegmentData segs_;
    QPoint pt0_, pt1_;
    QPoint bezy0_ { 0, BEZY_YOFF }, bezy1_ { 0, -BEZY_YOFF };
    QColor color_ { Qt::black };
    float pen_width_ { 1.5 };
    ConnectionCordType cord_type_ { ConnectionCordType::Bezier };

public:
    ConnectionViewData() { }

    ConnectionCordType cordType() const { return cord_type_; }
    void setCordType(ConnectionCordType type) { cord_type_ = type; }

    const SegmentData& segments() const { return segs_; }
    void appendSegment(float seg);
    void clearSegments();
    void setSegments(const SegmentData& segs) { segs_ = segs; }
    SegmentData makeSegments() const;
    bool setSegmentPos(int idx, const QPointF& pos);
    bool adjustSegmentLastPos();
    void createSegments();

    const QPoint& sourcePoint() const { return pt0_; }
    const QPoint& destinationPoint() const { return pt1_; }

    void setSourcePoint(const QPointF& pt) { pt0_ = pt.toPoint(); }
    void setDestinationPoint(const QPointF& pt) { pt1_ = pt.toPoint(); }

    const QPoint& bezyCtlPoint0() const { return bezy0_; }
    const QPoint& bezyCtlPoint1() const { return bezy1_; }

    void setBezyCtlPoint0(const QPointF& pt) { bezy0_ = pt.toPoint(); }
    void setBezyCtlPoint1(const QPointF& pt) { bezy1_ = pt.toPoint(); }

    const QColor& color() const { return color_; }
    void setColor(const QColor& color) { color_ = color; }

    qreal penWidth() const { return pen_width_; }
    void setPenWidth(qreal w) { pen_width_ = w; }

    void resetPoints(ConnectionCordType cord);

    /**
     * converts to Json object
     */
    QJsonObject toJson() const;

public:
    static std::optional<ConnectionViewData> fromJson(const QJsonValue& j);
};

QDebug operator<<(QDebug debug, const ConnectionId& c);
size_t qHash(const ConnectionId& key);

} // namespace ceam

Q_DECLARE_METATYPE(ceam::ConnectionId)

#endif // CONNECTION_DATA_H
