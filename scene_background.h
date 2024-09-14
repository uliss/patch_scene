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
#ifndef SCENE_BACKGROUND_H
#define SCENE_BACKGROUND_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSvgItem>
#include <QObject>
#include <QSvgRenderer>

namespace ceam {

class SceneBackground : public QObject {
    Q_OBJECT

public:
    SceneBackground();

    void setScene(QGraphicsScene* scene);

    /**
     * checks if no image content
     */
    bool isEmpty() const;

    /**
     * clear background image
     * @emit backgroundChanged()
     */
    void clear();

    /**
     * try load background image from file
     * @param path
     * @return true on success, false on error
     * @emit backgroundChanged()
     */
    bool loadImage(const QString& path);

    /**
     * set background visible
     */
    void setVisible(bool value);

    /**
     * @return background bounding rect
     */
    QRectF boundingRect() const;

    /**
     * set background position on the scene
     */
    void setPos(const QPointF& pos);

    void setSize(const QSizeF& size);

    QJsonValue toJson() const;
    bool setFromJson(const QJsonValue& v);

    /**
     * append background QActions to given menu
     */
    void addToContextMenu(QMenu& menu);

    /**
     * @return pointer to background graphics item
     */
    QGraphicsItem* sceneItem();

    /**
     * @return pointer to background graphics item
     */
    const QGraphicsItem* sceneItem() const;

signals:
    void backgroundChanged();
    void requestBackgroundChange();

private:
    bool setPixmap(const QPixmap& pixmap);
    bool setSvg(const QString& path);
    bool setSvg(const QByteArray& contents);

private:
    QGraphicsScene* scene_ { nullptr };
    QGraphicsPixmapItem* pixmap_ { nullptr };
    QGraphicsSvgItem* svg_ { nullptr };
    QByteArray svg_bin_content_;
    QSvgRenderer svg_renderer_;
};

}

#endif // SCENE_BACKGROUND_H
