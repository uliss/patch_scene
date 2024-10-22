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
#ifndef SEGMENT_EDITOR_HANDLE_H
#define SEGMENT_EDITOR_HANDLE_H

#include <QGraphicsRectItem>

namespace ceam {

class SegmentEditorHandle : public QGraphicsRectItem {
public:
    enum Direction {
        NONE,
        VERTICAL,
        HORIZONTAL,
        ANY
    };

public:
    SegmentEditorHandle(const QPointF& pos, Direction dir, QGraphicsItem* parent, std::function<void(const QPointF& pos)> fn);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) final;

private:
    std::function<void(const QPointF& pos)> fn_;
    Direction dir_ { NONE };
};

}

#endif // SEGMENT_EDITOR_HANDLE_H
