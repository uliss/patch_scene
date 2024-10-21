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

#include "connection_data.h"
#include "connection_database.h"

#include <QGraphicsItem>

namespace ceam {

constexpr qreal ZVALUE_CONN = 100;
constexpr qreal ZVALUE_BACKGROUND = -200;
constexpr qreal ZVALUE_LIVE_CONN = 16000;
constexpr qreal ZVALUE_SELECTION = 32000;

class Connection : public QGraphicsObject {
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

public:
    explicit Connection(const ConnectionId& id);

    QRectF boundingRect() const final;

    bool operator==(const ConnectionId& id) const;

    void setConnectionId(const ConnectionId& id);
    const ConnectionId& connectionId() const { return id_; }

    const ConnectionViewData& viewData() const { return view_data_; }
    void setViewData(const ConnectionViewData& data);

    XletInfo sourceInfo() const;
    XletInfo destinationInfo() const;

    void setPoints(const QPointF& p0, const QPointF& p1);
    void setStyle(ConnectionStyle style);

    void toggleSelection();

    void resetCordPoints(ConnectionCordType cord);

signals:
    void changed(ConnectionId);
    void selected(const Connection*, bool);
    void edited(ConnectionId, ConnectionViewData);
    void reset(ConnectionId, ConnectionCordType);

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    QPainterPath shape() const override;
    void updateShape();

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) final;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) final;

    /** @emit changed() */
    void setCordType(ConnectionCordType type);

private:
    ConnectionId id_;
    ConnectionViewData view_data_;
    QPainterPath line_;
    bool hover_ { false };
};

}

#endif // CONNECTION_H
