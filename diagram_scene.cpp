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
#include "diagram_scene.h"

#include <QGraphicsItemGroup>

namespace ceam {

DiagramScene::DiagramScene(int w, int h, QObject* parent)
    : QGraphicsScene { parent }
{
    setSceneRect(-w / 2, -h / 2, w, h);
}

void DiagramScene::initGrid()
{
    if (!grid_) {
        grid_ = new QGraphicsItemGroup;
        addItem(grid_);
    }

    auto rect = sceneRect();

    // axis
    QPen pen(QColor(100, 100, 100));
    pen.setWidth(0);

    auto x_axis = new QGraphicsLineItem;
    x_axis->setPen(pen);
    x_axis->setLine(QLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0)));
    grid_->addToGroup(x_axis);

    auto y_axis = new QGraphicsLineItem;
    y_axis->setPen(pen);
    y_axis->setLine(QLine(QPoint(0, rect.top()), QPoint(0, rect.bottom())));
    grid_->addToGroup(y_axis);

    // grid lines
    pen.setColor(QColor(100, 100, 100, 100));
    pen.setWidth(0);

    for (int i = 0; i < rect.width() / 50; i++) {
        auto x = 50 * (int(rect.left() + i * 50) / 50);
        auto p0 = QPoint(x, rect.top());
        auto p1 = QPoint(x, rect.bottom());
        auto line = new QGraphicsLineItem;
        line->setPen(pen);
        line->setLine(QLine(p0, p1));
        grid_->addToGroup(line);
    }

    for (int i = 0; i < rect.height() / 50; i++) {
        auto y = 50 * (int(rect.top() + i * 50) / 50);
        auto p0 = QPoint(rect.left(), y);
        auto p1 = QPoint(rect.right(), y);
        auto line = new QGraphicsLineItem;
        line->setPen(pen);
        line->setLine(QLine(p0, p1));
        grid_->addToGroup(line);
    }
}

bool DiagramScene::gridVisible() const
{
    return grid_ && grid_->isVisible();
}

void DiagramScene::setGridVisible(bool value)
{
    if (grid_)
        grid_->setVisible(value);
}

} // namespace ceam
