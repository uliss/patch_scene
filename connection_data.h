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

class SegmentPoints {
    QList<QPoint> points_;

public:
    SegmentPoints() { }
    void clear() { points_.clear(); }
    const QPoint& pointAt(int idx) const { return points_[idx]; }
    void append(const QPoint& pt) { points_.append(pt); }
    bool isEmpty() const { return points_.empty(); }
    qsizetype size() const { return points_.size(); }
    bool setPoint(int idx, const QPoint& pt);
    bool removePoint(int idx);

    QList<std::pair<QPoint, bool>> makePointList(const QPoint& from, const QPoint& to) const;
    bool splitAt(const QPoint& pos, const QPoint& from, const QPoint& to);

    QJsonValue toJson() const;

public:
    static std::optional<SegmentPoints> fromJson(const QJsonValue& v);
};

class ConnectionId {
    DeviceId src_ { 0 }, dest_ { 0 };
    XletIndex src_idx_ { 0 }, dest_idx_ { 0 };
    XletType src_type_ { XletType::Out }, dest_type_ { XletType::In };

    ConnectionId(DeviceId src, XletType srcType, XletIndex srcIdx,
        DeviceId dest, XletType destType, XletIndex destIdx)
        : src_(src)
        , src_idx_(srcIdx)
        , dest_(dest)
        , dest_idx_(destIdx)
        , src_type_(srcType)
        , dest_type_(destType)
    {
    }

public:
    ConnectionId(DeviceId src, XletIndex srcIdx, DeviceId dest, XletIndex destIdx)
        : src_(src)
        , src_idx_(srcIdx)
        , dest_(dest)
        , dest_idx_(destIdx)
        , src_type_ { XletType::Out }
        , dest_type_ { XletType::In }
    {
    }

    ConnectionId(const XletInfo& xi0, const XletInfo& xi1);

    DeviceId source() const { return src_; }
    DeviceId destination() const { return dest_; }
    XletIndex sourceIndex() const { return src_idx_; }
    XletIndex destinationIndex() const { return dest_idx_; }

    XletInfo sourceInfo() const { return { src_, src_idx_, src_type_ }; }
    XletInfo destinationInfo() const { return { dest_, dest_idx_, dest_type_ }; }

    XletType sourceType() const { return src_type_; }
    XletType destinationType() const { return dest_type_; }

    const bool operator==(const ConnectionId& id) const
    {
        return id.src_ == src_
            && id.dest_ == dest_
            && id.dest_idx_ == dest_idx_
            && id.src_idx_ == src_idx_
            && id.src_type_ == src_type_
            && id.dest_type_ == dest_type_;
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
    static std::optional<ConnectionId> fromXletPair(
        const XletInfo& x0,
        const XletInfo& x1);
};

class ConnectionViewData {
    static const int BEZY_YOFF = 40;

private:
    SegmentPoints segs_;
    QPoint pt0_, pt1_;
    QPoint bezy0_ { 0, BEZY_YOFF }, bezy1_ { 0, -BEZY_YOFF };
    QColor color_;
    float pen_width_;
    ConnectionCordType cord_type_ { ConnectionCordType::Bezier };

public:
    ConnectionViewData();

    ConnectionCordType cordType() const { return cord_type_; }
    void setCordType(ConnectionCordType type) { cord_type_ = type; }

    const SegmentPoints& segments() const { return segs_; }
    void clearSegments();
    void setSegments(const SegmentPoints& segs) { segs_ = segs; }
    SegmentPoints makeSegments() const;
    bool setSegmentPoint(int idx, const QPoint& pos);
    bool removeSegmentPoint(int idx);
    void createSegments();
    bool splitSegment(const QPointF& pos);

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

using ConnectionInfo = std::pair<ConnectionId, ConnectionViewData>;

} // namespace ceam

Q_DECLARE_METATYPE(ceam::ConnectionId)

#endif // CONNECTION_DATA_H
