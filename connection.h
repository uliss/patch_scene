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

using DeviceId = std::uint32_t;
using XletIndex = std::uint8_t;

struct ConnectionData {
    DeviceId src { 0 }, dest { 0 };
    XletIndex out { 0 }, in { 0 };

    ConnectionData(DeviceId src_, XletIndex out_, DeviceId dest_, XletIndex in_)
        : src(src_)
        , out(out_)
        , dest(dest_)
        , in(in_)
    {
    }

    const bool operator==(const ConnectionData& data) const
    {
        return data.src == src
            && data.dest == dest
            && data.in == in
            && data.out == out;
    }

    bool isValid() const
    {
        return src != dest;
    }

    QJsonObject toJson() const;

    static bool fromJson(const QJsonValue& j, ConnectionData& data);
};

Q_DECLARE_METATYPE(ConnectionData)

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

    bool operator==(const ConnectionData& data) const;
    QRectF boundingRect() const override;

    DeviceId source() const { return data_.src; }
    DeviceId destination() const { return data_.dest; }
    XletIndex sourceOutput() const { return data_.out; }
    XletIndex destinationInput() const { return data_.in; }

    XletInfo sourceInfo() const;
    XletInfo destinationInfo() const;

    const ConnectionData& connectionData() const { return data_; }

    QJsonObject toJson() const;

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;

public:
    // QRectF boundingRect() const override;
    // void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    // void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    // void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    // void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)   override;

    bool relatesToId(DeviceId id) const
    {
        return data_.src == id || data_.dest == id;
    }

    bool updateCachedPos();

private:
    ConnectionData data_;
    QPainterPath line_;
};

#endif // CONNECTION_H
