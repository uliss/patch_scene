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
#include "connection.h"
#include "connection_style.h"
#include "diagram_scene.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>

using namespace ceam;

namespace {

ConnectionDatabase& conn_db()
{
    static ConnectionDatabase db_;
    return db_;
}

}

Connection::Connection(const ConnectionData& data)
    : data_(data)
{
    setZValue(ZVALUE_CONN);
    setCacheMode(DeviceCoordinateCache);
}

bool Connection::operator==(const ConnectionData& data) const
{
    return data_ == data;
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QPen p(color_);
    p.setWidthF(pen_width_);
    painter->setPen(p);
    painter->drawPath(line_);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qWarning() << __FUNCTION__;
}

QPainterPath Connection::shape() const
{
    return line_;
}

QRectF Connection::boundingRect() const
{
    return line_.controlPointRect();
}

XletInfo Connection::destinationInfo() const
{
    return { data_.destination(), data_.destinationInput(), XletType::In };
}

XletInfo Connection::sourceInfo() const
{
    return { data_.source(), data_.sourceOutput(), XletType::Out };
}

void Connection::setPoints(const QPointF& p0, const QPointF& p1)
{
    prepareGeometryChange();

    line_.clear();
    line_.moveTo(p0);

    auto bezy = (std::abs(p0.x() - p1.x()) < 20) ? 20 : 40;

    line_.cubicTo(p0 + QPointF(0, bezy), p1 + QPointF(0, -bezy), p1);

    update(boundingRect());
}

void Connection::setStyle(ConnectionStyle style)
{
    pen_width_ = ConnectionStyleDatabase::instance().penWidth(style, 1.5);
    color_ = ConnectionStyleDatabase::instance().color(style, Qt::darkGray);
    update(boundingRect());
}

void Connection::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    auto dia_scene = qobject_cast<DiagramScene*>(scene());
    if (!dia_scene)
        return;

    QMenu menu;
    auto act = menu.addAction(QAction::tr("Delete"));
    QAction::connect(act, &QAction::triggered, dia_scene,
        [this, dia_scene]() {
            emit dia_scene->removeConnection(data_);
        });
    menu.exec(event->screenPos());
    event->accept();
}
