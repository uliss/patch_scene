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
#ifndef DEVICE_ITEM_H
#define DEVICE_ITEM_H

#include "device_xlet_view.h"
#include "scene_item.h"

class QGraphicsSvgItem;

namespace ceam {

class DeviceXlet;

class DeviceItem : public SceneItem {
    Q_OBJECT
public:
    DeviceItem();
    explicit DeviceItem(const SharedDeviceData& data);

    /**
     * return bounding rect in device coordinates
     */
    QRectF boundingRect() const final;

    /**
     * @return title bounding rect in device coordinates
     */
    QRectF titleRect() const;

    /**
     * @return xlet bounding rect in device coordinates
     */
    QRectF xletRect() const;

    bool setDeviceData(const SharedDeviceData& data) override;

    /**
     * @return connection point in device or scene coords
     * @param map - map to scene coordinates
     */
    std::optional<QPointF> connectionPoint(XletIndex i, XletType type, bool map) const final;

    DeviceXlets& xlets() { return xlets_; }
    const DeviceXlets& xlets() const { return xlets_; }

    QGraphicsTextItem* title() { return title_; }
    const QGraphicsTextItem* title() const { return title_; }

    QGraphicsSvgItem* image() { return image_; }
    const QGraphicsSvgItem* image() const { return image_; }

    bool mirrorImage(ImageMirrorType type);
    bool zoomImage(qreal k);

    /**
     * fill given menu with device actions
     * @note only for single context menu
     */
    void createContextMenu(QMenu& menu) override;

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void paintTitleBox(QPainter* painter);
    void paintStateIcons(QPainter* painter);

    int inletsYOff() const;

    int calcWidth() const;
    int calcHeight() const;

    qreal imageWidth() const;
    qreal imageHeight() const;

    void clearTitle();
    void clearImage();

    void createXlets();
    void createTitle(qreal wd);
    void createImage();

    void syncXletData();

    void updateTitlePos();
    void updateImagePos();
    void updateXletsPos();

    qreal centerAlignedLeftPos(qreal width) const
    {
        return rect_.left() + (rect_.width() - width) * 0.5;
    }

    void syncRect();

    void addMirrorAction(QMenu& menu);
    void addToFavoritesAct(QMenu& menu);
    void addTitleAction(QMenu& menu);
    void addViewSubMenu(QMenu& menu);

private:
    friend class DeviceXlet;

private:
    QGraphicsTextItem* title_;
    QGraphicsSvgItem* image_;
    QRectF rect_;
    DeviceXlets xlets_;
};

} // namespace ceam

#endif // DEVICE_ITEM_H
