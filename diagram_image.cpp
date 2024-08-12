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
#include "diagram_image.h"

DiagramImage::DiagramImage(const QString& path, QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setAcceptDrops(false);
    setAcceptHoverEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);

    setImagePath(path);
}

QRectF DiagramImage::boundingRect() const
{
    if (svg_)
        return svg_->boundingRect();
    else if (pixmap_)
        return pixmap_->boundingRect();
    else
        return {};
}

void DiagramImage::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (svg_)
        svg_->paint(painter, option, widget);

    if (pixmap_)
        pixmap_->paint(painter, option, widget);
}

bool DiagramImage::setImagePath(const QString& path)
{
    clearImage();

    if (path.isEmpty())
        return !empty_;

    if (path.endsWith(".svg", Qt::CaseInsensitive)) {
        svg_ = new QGraphicsSvgItem(path, this);
        qWarning() << svg_->boundingRect();
        return isEmpty();
    } else if (path.endsWith(".png", Qt::CaseInsensitive)
        || path.endsWith(".jpg", Qt::CaseInsensitive)
        || path.endsWith(".jpeg", Qt::CaseInsensitive)) {

        QPixmap pixmap(path);
        if (!pixmap) {
            empty_ = true;
            return !empty_;
        }

        pixmap_ = new QGraphicsPixmapItem(pixmap, this);
        empty_ = false;
        return !empty_;
    } else {
        qWarning() << "unknown image format:" << path;
        empty_ = true;
        return !empty_;
    }
}

void DiagramImage::clearImage()
{
    delete pixmap_;
    delete svg_;

    pixmap_ = nullptr;
    svg_ = nullptr;

    empty_ = true;
}
