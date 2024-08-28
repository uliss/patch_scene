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
#ifndef DIAGRAM_SCENE_H
#define DIAGRAM_SCENE_H

#include <QGraphicsItem>
#include <QGraphicsScene>

class QGraphicsItemGroup;
class QPrinter;

namespace ceam {

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    explicit DiagramScene(int w, int h, QObject* parent = nullptr);

    void initGrid();

    bool gridVisible() const;
    void setGridVisible(bool value);

    void setCacheMode(QGraphicsItem::CacheMode mode);

    void renderDiagram(QPainter* painter, const QRect& rect = {});
    QImage renderToImage(qreal scale);
    void printDiagram(QPrinter* printer);

    QRectF bestFitRect();
    QGraphicsItemGroup* grid() { return grid_; }

private:
    QGraphicsItemGroup* grid_ { nullptr };
};

} // namespace ceam

#endif // DIAGRAM_SCENE_H
