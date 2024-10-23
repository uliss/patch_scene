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
#include "logging.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

using namespace ceam;

namespace {

QPainterPath makeSegmentsPath(const QPoint& from, const QPoint& to, const SegmentPoints& segs)
{
    QPainterPath line;

    auto points = segs.makePointList(from, to);

    if (points.isEmpty())
        return line;

    line.moveTo(points.front());
    // iterate forward
    for (int i = 1; i < points.size(); i++) {
        line.lineTo(points[i]);
    }

    // iterate backward
    for (int i = points.size() - 2; i > 0; i--) {
        line.lineTo(points[i]);
    }

    line.closeSubpath();

    return line;
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
        QColor hover_color = (view_data_.color() == Qt::black)
            ? Qt::darkGray
            : view_data_.color().lighter(200);
        painter->setBrush(hover_color);
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
    return shape_;
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
        line_.clear();

        if (view_data_.segments().isEmpty()) { // auto segment path
            line_ = makeSegmentsPath(view_data_.sourcePoint(), view_data_.destinationPoint(), view_data_.makeSegments());
        } else {
            line_ = makeSegmentsPath(view_data_.sourcePoint(), view_data_.destinationPoint(), view_data_.segments());
        }

        if (line_.isEmpty())
            return;

        QPainterPathStroker stroker;
        stroker.setWidth(view_data_.penWidth());
        stroker.setCapStyle(Qt::RoundCap);
        line_ = stroker.createStroke(line_);

        stroker.setWidth(5);
        shape_ = stroker.createStroke(line_);
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

void Connection::resetCordPoints(ConnectionCordType cord)
{
    view_data_.resetPoints(cord);
    updateShape();
}

bool Connection::splitSegment(const QPointF& pos)
{
    return view_data_.splitSegment(pos);
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

    setSelected(true);
    emit selected(this, true);
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

    if (view_data_.cordType() == ConnectionCordType::Segmented) {
        auto act_edit = menu.addAction(QAction::tr("Edit"));
        QAction::connect(act_edit, &QAction::triggered, dia_scene, [this]() {
            emit edited(id_, view_data_);
        });

        auto pos = event->scenePos();
        auto act_split = menu.addAction(QAction::tr("Add split point"));
        QAction::connect(act_split, &QAction::triggered, dia_scene, [this, pos]() {
            emit splited(id_, pos);
        });

        if (!view_data_.segments().isEmpty()) {
            auto act_reset = menu.addAction(QAction::tr("Reset"));
            QAction::connect(act_reset, &QAction::triggered, dia_scene, [this]() {
                emit reset(id_, view_data_.cordType());
            });
        }
    } else if (view_data_.cordType() == ConnectionCordType::Bezier) {
        auto act_edit = menu.addAction(QAction::tr("Edit"));
        QAction::connect(act_edit, &QAction::triggered, dia_scene, [this]() {
            emit edited(id_, view_data_);
        });

        auto act_reset = menu.addAction(QAction::tr("Reset"));
        QAction::connect(act_reset, &QAction::triggered, dia_scene, [this]() {
            emit reset(id_, view_data_.cordType());
        });
    }

    menu.exec(event->screenPos());
    event->accept();
}
