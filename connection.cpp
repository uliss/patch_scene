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
#include <QStyleOptionGraphicsItem>

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
    setFlag(QGraphicsItem::ItemIsSelectable);
}

bool Connection::operator==(const ConnectionData& data) const
{
    return data_ == data;
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setPen(Qt::NoPen);

    if (isSelected()) {
        painter->setBrush(Qt::blue);
        painter->drawPath(line_);
    } else {
        painter->setBrush(color_);
        painter->drawPath(line_);
    }

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QPainterPath Connection::shape() const
{
    return line_;
}

void Connection::updateShape()
{
    prepareGeometryChange();

    switch (data_.cordType()) {
    case ConnectionCordType::Linear: {
        line_.clear();
        line_.moveTo(pt0_);
        line_.lineTo(pt1_);

        QPainterPathStroker stroker;
        stroker.setWidth(pen_width_);
        stroker.setCapStyle(Qt::RoundCap);
        line_ = stroker.createStroke(line_);
    } break;
    case ConnectionCordType::Bezier: {
        auto bezy = (std::abs(pt0_.x() - pt1_.x()) < 20) ? 20 : 40;
        ctl_pt0_ = pt0_ + QPointF(0, bezy);
        ctl_pt1_ = pt1_ + QPointF(0, -bezy);
        line_.clear();
        line_.moveTo(pt0_);
        line_.cubicTo(ctl_pt0_, ctl_pt1_, pt1_);
        line_.cubicTo(ctl_pt1_, ctl_pt0_, pt0_);
        line_.closeSubpath();

        QPainterPathStroker stroker;
        stroker.setWidth(pen_width_);
        line_ = stroker.createStroke(line_);
    } break;
    case ConnectionCordType::Segmented:
    default:
        break;
    }

    update(boundingRect());
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
    pt0_ = p0;
    pt1_ = p1;

    updateShape();
}

void Connection::setStyle(ConnectionStyle style)
{
    pen_width_ = ConnectionStyleDatabase::instance().penWidth(style, 1.5);
    color_ = ConnectionStyleDatabase::instance().color(style, Qt::darkGray);

    updateShape();
}

void Connection::setCordType(ConnectionCordType type)
{
    data_.setCordType(type);
    updateShape();
}

void Connection::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    auto dia_scene = qobject_cast<DiagramScene*>(scene());
    if (!dia_scene)
        return;

    QMenu menu;

    auto act_del = menu.addAction(QAction::tr("Delete"));
    QAction::connect(act_del, &QAction::triggered, dia_scene,
        [this, dia_scene]() {
            emit dia_scene->removeConnection(data_);
        });

    auto menu_ct = menu.addMenu(QAction::tr("Cord type"));

    auto act_bezier = menu_ct->addAction(QAction::tr("Bezier"));
    auto act_linear = menu_ct->addAction(QAction::tr("Linear"));

    act_bezier->setCheckable(true);
    act_bezier->setChecked(data_.cordType() == ConnectionCordType::Bezier);
    QAction::connect(act_bezier, &QAction::triggered, dia_scene, [this]() {
        setCordType(ConnectionCordType::Bezier);
    });

    act_linear->setCheckable(true);
    act_linear->setChecked(data_.cordType() == ConnectionCordType::Linear);
    QAction::connect(act_linear, &QAction::triggered, dia_scene, [this]() {
        setCordType(ConnectionCordType::Linear);
    });

    menu.exec(event->screenPos());
    event->accept();
}
