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

Connection::Connection(const ConnectionId& id)
    : id_(id)
{
    setZValue(ZVALUE_CONN);
    setCacheMode(DeviceCoordinateCache);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setToolTip(QString("In(%1) -> Out(%2)").arg((int)id.sourceOutput()).arg((int)id.destinationInput()));
    setAcceptHoverEvents(true);
}

bool Connection::operator==(const ConnectionId& id) const
{
    return id_ == id;
}

void Connection::setConnectionId(const ConnectionId& id)
{
    id_ = id;
    updateShape();
}

void Connection::setViewData(const ConnectionViewData& data)
{
    view_data_ = data;
    updateShape();
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setPen(Qt::NoPen);

    if (isSelected()) {
        painter->setBrush(Qt::blue);
        painter->drawPath(line_);
    } else if (hover_) {
        painter->setBrush(view_data_.color().lighter(220));
        painter->drawPath(line_);
    } else {
        painter->setBrush(view_data_.color());
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

    switch (view_data_.cordType()) {
    case ConnectionCordType::Linear: {
        line_.clear();
        line_.moveTo(view_data_.sourcePoint());
        line_.lineTo(view_data_.destinationPoint());

        QPainterPathStroker stroker;
        stroker.setWidth(view_data_.penWidth());
        stroker.setCapStyle(Qt::RoundCap);
        line_ = stroker.createStroke(line_);
    } break;
    case ConnectionCordType::Bezier: {
        auto ctl_pt0 = view_data_.bezyCtlPoint0() + view_data_.sourcePoint();
        auto ctl_pt1 = view_data_.bezyCtlPoint1() + view_data_.destinationPoint();

        line_.clear();
        line_.moveTo(view_data_.sourcePoint());
        line_.cubicTo(ctl_pt0, ctl_pt1, view_data_.destinationPoint());
        line_.cubicTo(ctl_pt1, ctl_pt0, view_data_.sourcePoint());
        line_.closeSubpath();

        QPainterPathStroker stroker;
        stroker.setWidth(view_data_.penWidth());
        line_ = stroker.createStroke(line_);
    } break;
    case ConnectionCordType::Segmented: {
        view_data_.clearSegments();

        // if (data_.segmentPoints().isEmpty()) {
        // if (pt1_.y() > pt0_.y()) {
        //     auto yoff = (pt1_.y() + pt0_.y()) * 0.5;
        //     auto p0 = QPointF(pt0_.x(), yoff);
        //     auto p1 = QPointF(pt1_.x(), yoff);

        //     line_.clear();
        //     line_.moveTo(pt0_);
        //     line_.lineTo(p0);
        //     line_.lineTo(p1);
        //     line_.lineTo(pt1_);

        //     data_.appendSegment(pt0_.x());
        //     data_.appendSegment(yoff);
        // } else {
        //     constexpr int pad = 8;
        //     auto xoff = (pt1_.x() + pt0_.x()) * 0.5;

        //     line_.clear();
        //     line_.moveTo(pt0_);
        //     line_.lineTo(pt0_.x(), pt0_.y() + pad);
        //     line_.lineTo(xoff, pt0_.y() + pad);
        //     line_.lineTo(xoff, pt1_.y() - pad);
        //     line_.lineTo(pt1_.x(), pt1_.y() - pad);
        //     line_.lineTo(pt1_.x(), pt1_.y());

        //     // data_.appendSegPoint(pt0_.toPoint());
        //     // data_.appendSegPoint(pt1_.toPoint());
        // }
        // }

        QPainterPathStroker stroker;
        stroker.setWidth(view_data_.penWidth() + 1);
        stroker.setCapStyle(Qt::RoundCap);
        line_ = stroker.createStroke(line_);

    } break;
    default:
        break;
    }

    update(boundingRect());
}

void Connection::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    hover_ = true;
    update(boundingRect());
}

void Connection::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    hover_ = false;
    update(boundingRect());
}

QRectF Connection::boundingRect() const
{
    return line_.controlPointRect();
}

XletInfo Connection::destinationInfo() const
{
    return { id_.destination(), id_.destinationInput(), XletType::In };
}

XletInfo Connection::sourceInfo() const
{
    return { id_.source(), id_.sourceOutput(), XletType::Out };
}

void Connection::setPoints(const QPointF& p0, const QPointF& p1)
{
    view_data_.setSourcePoint(p0);
    view_data_.setDestinationPoint(p1);

    updateShape();
}

void Connection::setStyle(ConnectionStyle style)
{
    view_data_.setPenWidth(ConnectionStyleDatabase::instance().penWidth(style, 1.5));
    view_data_.setColor(ConnectionStyleDatabase::instance().color(style, Qt::darkGray));

    updateShape();
}

void Connection::toggleSelection()
{
    auto value = !isSelected();
    setSelected(value);

    emit selected(this, value);
}

void Connection::setCordType(ConnectionCordType type)
{
    view_data_.setCordType(type);
    updateShape();
    emit changed(id_);
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
            emit dia_scene->removeConnection(id_);
        });

    auto menu_ct = menu.addMenu(QAction::tr("Cord type"));

    auto act_bezier = menu_ct->addAction(QAction::tr("Bezier"));
    auto act_linear = menu_ct->addAction(QAction::tr("Linear"));
    auto act_segment = menu_ct->addAction(QAction::tr("Segment"));

    act_bezier->setCheckable(true);
    act_bezier->setChecked(view_data_.cordType() == ConnectionCordType::Bezier);
    QAction::connect(act_bezier, &QAction::triggered, dia_scene, [this]() {
        setCordType(ConnectionCordType::Bezier);
    });

    act_linear->setCheckable(true);
    act_linear->setChecked(view_data_.cordType() == ConnectionCordType::Linear);
    QAction::connect(act_linear, &QAction::triggered, dia_scene, [this]() {
        setCordType(ConnectionCordType::Linear);
    });

    act_segment->setCheckable(true);
    act_segment->setChecked(view_data_.cordType() == ConnectionCordType::Segmented);
    QAction::connect(act_segment, &QAction::triggered, dia_scene, [this]() {
        setCordType(ConnectionCordType::Segmented);
    });

    auto act_edit = menu.addAction(QAction::tr("Edit"));
    QAction::connect(act_edit, &QAction::triggered, dia_scene, [this]() {
        emit edited(id_, view_data_);
    });

    menu.exec(event->screenPos());
    event->accept();
}
