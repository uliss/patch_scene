/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
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
#ifndef COMMENT_ITEM_H
#define COMMENT_ITEM_H

#include "scene_item.h"

namespace ceam {

class CommentItem : public SceneItem {
    Q_OBJECT

public:
    CommentItem();

    /**
     * return bounding rect in item coordinates
     */
    QRectF boundingRect() const final;

public:
    void createContextMenu(QMenu& menu) override;
    void showEditDialog() override;

protected:
    void addEditAct(QMenu& menu);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    // void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    QGraphicsTextItem* text_ { nullptr };
    enum State {
        NORMAL,
        RESIZE_LEFT_TOP,
        RESIZE_RIGHT_BOTTOM,
        RESIZE_RIGHT_TOP,
        RESIZE_LEFT_BOTTOM
    };

    State state_ { NORMAL };
    QPointF click_pos_;
    QRectF rect_;
};

} // namespace ceam

#endif // COMMENT_ITEM_H
