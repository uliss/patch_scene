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
#ifndef BEZIER_EDITOR_HANDLE_H
#define BEZIER_EDITOR_HANDLE_H

#include <QGraphicsItem>
#include <functional>

namespace ceam {

class BezierEditorHandle : public QGraphicsItem {
    std::function<void(const QPointF& pos)> fn_;
    QPointF src_pos_;
    QPainterPath shape_;
    QRectF ellipse_;

public:
    BezierEditorHandle(const QPoint& srcPoint, const QPoint& bezyOffset,
        QGraphicsItem* parent, const std::function<void(const QPointF& pos)>& fn);

    void setHandlePos(const QPointF& pos);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    QRectF boundingRect() const final;
    QPainterPath shape() const final { return shape_; }
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) final;
};

} // namespace ceam

#endif // BEZIER_EDITOR_HANDLE_H
