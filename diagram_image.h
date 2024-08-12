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
#ifndef DIAGRAM_IMAGE_H
#define DIAGRAM_IMAGE_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>

class DiagramImage : public QGraphicsItem {
public:
    DiagramImage(const QString& path, QGraphicsItem* parent = nullptr);

    enum { Type = QGraphicsItem::UserType + 4 };
    int type() const override { return Type; }

    QRectF boundingRect() const final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) final;

    void clearImage();
    bool setImagePath(const QString& path);

    bool isEmpty() const;

    QJsonValue toJson() const;
    static std::unique_ptr<DiagramImage> fromJson(const QJsonValue& v);

private:
    void setPixmap();

private:
    QGraphicsPixmapItem* pixmap_ { nullptr };
    QGraphicsSvgItem* svg_ { nullptr };
    QString svg_content_;
};

#endif // DIAGRAM_IMAGE_H
