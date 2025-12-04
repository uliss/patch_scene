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
#include "logging.hpp"

#include <QKeyEvent>
#include <QPainter>
#include <QPrinter>

using namespace ceam;

DiagramScene::DiagramScene(int w, int h, QObject* parent)
    : QGraphicsScene { parent }
{
    setSceneRect(-w / 2, -h / 2, w, h);
}

void DiagramScene::setGridVisible(bool value)
{
    grid_visible_ = value;
    update();
}

void DiagramScene::setCacheMode(QGraphicsItem::CacheMode mode)
{
    for (auto x : items())
        x->setCacheMode(mode);
}

void DiagramScene::renderDiagram(QPainter* painter, const QRect& rect)
{
    QSignalBlocker db(this);

    const auto old_rect = sceneRect();
    const auto new_rect = rect.isNull() ? bestFitRect() : rect;

    auto selected = selectedItems();
    clearSelection();

    setSceneRect(new_rect);
    setCacheMode(QGraphicsItem::NoCache);

    render(painter);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setSceneRect(old_rect);

    for (auto& x : selected)
        x->setSelected(true);
}

QImage DiagramScene::renderToImage(qreal scale)
{
    auto rect = bestFitRect();

    QImage image(rect.size().toSize() * scale, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    renderDiagram(&painter, rect.toRect());
    painter.end();
    return image;
}

void DiagramScene::printDiagram(QPrinter* printer)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    renderDiagram(&painter);
    painter.end();
}

QRectF DiagramScene::bestFitRect() const
{
    return itemsBoundingRect();
}

void DiagramScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (!grid_visible_)
        return;

    // axis
    QPen pen(QColor(100, 100, 100));
    pen.setWidth(0);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->drawLine(QLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0)));
    painter->drawLine(QLine(QPoint(0, rect.top()), QPoint(0, rect.bottom())));

    auto p0 = QPoint(0, rect.top());
    auto p1 = QPoint(0, rect.bottom());

    pen.setColor(QColor(100, 100, 100, 100));
    painter->setPen(pen);

    // horizontal grid lines
    for (int i = 0; i <= qCeil(rect.width() / 50); i++) {
        auto x = 50 * (static_cast<int>(rect.left() + i * 50) / 50);
        p0.rx() = x;
        p1.rx() = x;
        painter->drawLine(QLine(p0, p1));
    }

    // vertical grid lines
    p0.rx() = rect.left();
    p1.rx() = rect.right();
    for (int i = 0; i <= qCeil(rect.height() / 50); i++) {
        auto y = 50 * (static_cast<int>(rect.top() + i * 50) / 50);
        p0.ry() = y;
        p1.ry() = y;
        painter->drawLine(QLine(p0, p1));
    }
}

void DiagramScene::keyPressEvent(QKeyEvent* event)
{
    // WARN() << "key: " << event->isAccepted();
    return QGraphicsScene::keyPressEvent(event);
    // if (event->isAccepted()) {
    //     WARN() << "accepted";
    //     return;
    // }

    // auto mods = event->modifiers();
    // int MOVE_STEP = 2;
    // if (mods.testFlags(Qt::ControlModifier))
    //     MOVE_STEP = 50;
    // else if (mods.testFlags(Qt::ShiftModifier))
    //     MOVE_STEP = 10;

    // if (event->key() == Qt::Key_Backspace && event->modifiers().testFlag(Qt::ControlModifier)) {
    //     WARN() << "remove";
    //     event->accept();
    //     // cmdRemoveSelected();
    // } else if (event->key() == Qt::Key_Down) {
    //     WARN() << "move down";
    //     event->accept();
    //     // cmdMoveSelectedItemsBy(0, MOVE_STEP);
    // } else if (event->key() == Qt::Key_Up) {
    //     WARN() << "move up";
    //     event->accept();
    //     // cmdMoveSelectedItemsBy(0, -MOVE_STEP);
    // } else if (event->key() == Qt::Key_Left) {
    //     WARN() << "move left";
    //     event->accept();
    //     // cmdMoveSelectedItemsBy(-MOVE_STEP, 0);
    // } else if (event->key() == Qt::Key_Right) {
    //     WARN() << "move right";
    //     event->accept();
    //     // cmdMoveSelectedItemsBy(MOVE_STEP, 0);
    // }

    // // WARN() << "key";
}
