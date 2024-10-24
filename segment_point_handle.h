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
#ifndef SEGMENT_POINT_HANDLE_H
#define SEGMENT_POINT_HANDLE_H

#include <QGraphicsEllipseItem>

namespace ceam {

class SegmentPointHandle : public QGraphicsEllipseItem {
public:
    enum { Type = QGraphicsItem::UserType + 6 };
    int type() const override { return Type; }

public:
    SegmentPointHandle(const QPointF& pos,
        std::function<void(const QPointF& pos)> moveCallback,
        std::function<void()> removeCallback,
        QGraphicsItem* parent = nullptr);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;

private:
    void setHandlePos(const QPointF& pos);

private:
    std::function<void(const QPointF& pos)> move_callback_;
    std::function<void()> remove_callback_;
};

} // namespace ceam

#endif // SEGMENT_POINT_HANDLE_H
