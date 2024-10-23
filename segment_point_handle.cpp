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
#include "segment_point_handle.h"

#include <QGraphicsSceneMouseEvent>
#include <QPen>

using namespace ceam;

SegmentPointHandle::SegmentPointHandle(const QPointF& pos,
    std::function<void(const QPointF& pos)> moveCallback, std::function<void()> removeCallback,
    QGraphicsItem* parent)
    : QGraphicsEllipseItem(QRect(-5, -5, 10, 10), parent)
    , remove_callback_(removeCallback)
    , move_callback_(moveCallback)
{
    setToolTip("Segment editor point");
    setFlag(ItemIsMovable);

    setPos(pos);
    setPen(QPen(Qt::darkGray, 1));
    setBrush(QColor(0xFFAA00));
}

void SegmentPointHandle::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers().testFlags(Qt::AltModifier)) {
        event->accept();
        if (remove_callback_)
            remove_callback_();

    } else
        event->ignore();
}

void SegmentPointHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();
    setPos(event->scenePos());

    if (move_callback_)
        move_callback_(pos());
}
