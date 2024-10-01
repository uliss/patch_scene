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
#include "bezier_editor_handle.h"

#include <QGraphicsSceneMoveEvent>
#include <QPainter>

namespace ceam {

BezierEditorHandle::BezierEditorHandle(const QPoint& srcPoint, const QPoint& bezyOffset, QGraphicsItem* parent, std::function<void(const QPointF&)> fn)
    : QGraphicsItem(parent)
    , fn_(fn)
    , src_pos_(srcPoint)
    , ellipse_ { -5, -5, 10, 10 }
{
    setToolTip("Segment point");
    setFlag(ItemIsMovable);
    setHandlePos(src_pos_ + bezyOffset);
}

QRectF BezierEditorHandle::handleRect() const
{
    return ellipse_.translated(pos());
}

void BezierEditorHandle::setHandlePos(const QPointF& pos)
{
    prepareGeometryChange();
    shape_.clear();
    shape_.addEllipse(ellipse_);
    shape_.addPolygon({ mapFromScene(src_pos_), mapFromScene(pos) });
    setPos(pos);
}

void BezierEditorHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();
    setHandlePos(event->scenePos());

    if (fn_)
        fn_(event->scenePos());

    update(boundingRect());
}

QRectF BezierEditorHandle::boundingRect() const
{
    return shape_.boundingRect();
}

void BezierEditorHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setPen(QPen(Qt::darkGray, 1));
    painter->drawLine({ mapFromScene(src_pos_), mapFromScene(pos()) });

    painter->setPen(QPen(Qt::darkGray, 1));
    painter->setBrush(Qt::green);
    painter->drawEllipse(ellipse_);
}

} // namespace ceam
