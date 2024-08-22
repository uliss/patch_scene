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
#ifndef CONNECTION_H
#define CONNECTION_H

#include "socket.h"

#include <QGraphicsLineItem>

namespace ceam {

using DeviceId = std::uint32_t;
using XletIndex = std::uint8_t;

class Device;

constexpr DeviceId DEV_NULL_ID = 0;
constexpr qreal ZVALUE_CONN = 100;
constexpr qreal ZVALUE_BACKGROUND = -200;
constexpr qreal ZVALUE_LIVE_CONN = 16000;
constexpr qreal ZVALUE_SELECTION = 32000;

class ConnectionData {
    DeviceId src_ { 0 }, dest_ { 0 };
    XletIndex out_ { 0 }, in_ { 0 };

public:
    ConnectionData(DeviceId src, XletIndex out, DeviceId dest, XletIndex in)
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

    const bool operator==(const ConnectionData& data) const
    {
        return data.src_ == src_
            && data.dest_ == dest_
            && data.in_ == in_
            && data.out_ == out_;
    }

    bool relatesToId(DeviceId id) const
    {
        return src_ == id || dest_ == id;
    }

    bool isValid() const
    {
        return src_ != dest_;
    }

    bool isSameSource(const ConnectionData& conn) const
    {
        return src_ == conn.src_ && out_ == conn.out_;
    }

    bool isSameDestimation(const ConnectionData& conn) const
    {
        return dest_ == conn.dest_ && in_ == conn.in_;
    }

    /**
     * converts to Json object
     */
    QJsonObject toJson() const;

    static bool fromJson(const QJsonValue& j, ConnectionData& data);
};

struct XletInfo {
    DeviceId id { 0 };
    XletType type { XletType::None };

    int index { -1 };

    bool operator==(const XletInfo& xi) const
    {
        return xi.id == id && xi.type == type && xi.index == index;
    }

    bool operator!=(const XletInfo& xi) const { return !operator==(xi); }

    static XletInfo none() { return { 0, XletType::None, -1 }; }
};

class Connection : public QGraphicsItem {
public:
    explicit Connection(const ConnectionData& data);

    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }
    QRectF boundingRect() const override;

    bool operator==(const ConnectionData& data) const;
    const ConnectionData& connectionData() const { return data_; }
    XletInfo sourceInfo() const;
    XletInfo destinationInfo() const;
    bool relatesToDevice(DeviceId id) const { return data_.relatesToId(id); }

    /**
     * find connected elements on the graphic scene
     * @complexity O(n)
     * @return pair of valid pointers to connected element or null optional
     */
    std::optional<std::pair<Device*, Device*>> findConnectedElements() const;

    /**
     * check if connected elements are exists on the scene
     * @complexity O(n)
     * @return true if connected source and destination are exists
     */
    bool checkConnectedElements() const;

    /**
     * updates connection positions
     * @complexity O(n)
     * @return true on success
     */
    bool updateCachedPos();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

private:
    ConnectionData data_;
    QPainterPath line_;
};

uint qHash(const ConnectionData& key);

}

Q_DECLARE_METATYPE(ceam::ConnectionData)

#endif // CONNECTION_H
