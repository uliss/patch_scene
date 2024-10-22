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
#include "segment_editor_handle.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace {
constexpr int HANDLE_SIZE = 14;
constexpr int HANDLE_OFF = HANDLE_SIZE / 2;
constexpr int INDICATOR_W = 2;
constexpr int INDICATOR_H = ((HANDLE_SIZE * 2 / 3) / 2) * 2;
constexpr int INDICATOR_WOFF = INDICATOR_W / 2;
constexpr int INDICATOR_HOFF = INDICATOR_H / 2;
}

using namespace ceam;

SegmentEditorHandle::SegmentEditorHandle(const QPointF& pos,
    Direction dir,
    QGraphicsItem* parent,
    std::function<void(const QPointF& pos)> fn)
    : QGraphicsRectItem(-HANDLE_OFF, -HANDLE_OFF, HANDLE_SIZE, HANDLE_SIZE, parent)
    , fn_(fn)
    , dir_(dir)
{
    setToolTip("Segment editor point");
    setFlag(ItemIsMovable);

    setPos(pos);
    setPen(QPen(Qt::darkGray, 1));
    setBrush(Qt::green);
}

void SegmentEditorHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();

    switch (dir_) {
    case VERTICAL:
        setPos(pos().x(), event->scenePos().y());
        break;
    case HORIZONTAL:
        setPos(event->scenePos().x(), pos().y());
        break;
    case ANY:
        setPos(event->scenePos());
        break;
    default:
        return;
    }

    if (fn_)
        fn_(pos());
}

void SegmentEditorHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QGraphicsRectItem::paint(painter, option, widget);
    switch (dir_) {
    case VERTICAL:
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawRect(-INDICATOR_WOFF, -INDICATOR_HOFF, INDICATOR_W, INDICATOR_H);
        break;
    case HORIZONTAL:
        painter->setBrush(Qt::black);
        painter->setPen(Qt::NoPen);
        painter->drawRect(-INDICATOR_HOFF, -INDICATOR_WOFF, INDICATOR_H, INDICATOR_W);
        break;
    case ANY:
        break;
    default:
        break;
    }
}
