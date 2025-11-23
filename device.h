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
#ifndef DEVICE_H
#define DEVICE_H

#include "device_common.h"
#include "device_xlet_view.h"
#include "user_item_types.h"

#include <QAction>
#include <QGraphicsObject>

class QGraphicsSvgItem;

namespace ceam {

class DeviceXlet;

class Device : public QGraphicsObject {
    Q_OBJECT
public:
    enum { Type = UserItemTypeDevice };
    int type() const override { return Type; }

public:
    Device();
    explicit Device(const SharedDeviceData& data);
    ~Device();

    /**
     * @return device id
     */
    DeviceId id() const { return data_->id(); }

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

    /**
     * @return connection point in device or scene coords
     */
    std::optional<QPointF> connectionPoint(XletIndex i, XletType type, bool map = false) const;

    SharedDeviceData deviceData() const;
    void setDeviceData(const SharedDeviceData& data);

    /**
     * move device into random neighborhood within the specified delta
     */
    void randomizePos(qint64 delta);

    /**
     * export device state/data to json
     */
    QJsonObject toJson() const;

    DeviceXlets& xlets() { return xlets_; }
    const DeviceXlets& xlets() const { return xlets_; }

    bool isLocked() const { return data_ && data_->isLocked(); }
    void setLocked(bool value);

    bool mirrorImage(ImageMirrorType type);
    bool zoomImage(qreal k);

    /**
     * fill given menu with device actions
     * @note only for single context menu
     */
    virtual void createContextMenu(QMenu& menu);

    static SharedDeviceData defaultDeviceData();
    static SharedDeviceData dataFromJson(const QJsonValue& j);

signals:
    void addToFavorites(SharedDeviceData data);
    void alignHorizontal();
    void alignVertical();
    void distributeHorizontal();
    void distributeVertical();
    void duplicateDevice(SharedDeviceData data);
    void moveLower(const SharedDeviceData& data);
    void moveUpper(const SharedDeviceData& data);
    void placeInColumn();
    void placeInRow();
    void removeDevice(SharedDeviceData data);
    void updateDevice(SharedDeviceData data);

    void lockSelected();
    void unlockSelected();
    void lock(DeviceId id);
    void unlock(DeviceId id);

    void mirrorSelected();
    void mirror(DeviceId id);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void paintTitleBox(QPainter* painter);
    void paintStateIcons(QPainter* painter);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;

    int inletsYOff() const;

    void clearTitle();
    void clearImage();

    void createXlets();
    void createTitle(qreal wd);
    void createImage();

    void syncRect();
    void syncXletData();

    void updateTitlePos();
    void updateImagePos();
    void updateXletsPos();

    int calcWidth() const;
    int calcHeight() const;

    qreal imageWidth() const;
    qreal imageHeight() const;

    qreal centerAlignedLeftPos(qreal width) const
    {
        return rect_.left() + (rect_.width() - width) * 0.5;
    }

protected:
    // context menu actions
    void addDuplicateAct(QMenu& menu);
    void addLockAction(QMenu& menu);
    void addMirrorAction(QMenu& menu);
    void addPropertiesAct(QMenu& menu);
    void addRemoveAct(QMenu& menu);
    void addTitleAction(QMenu& menu);
    void addToFavoritesAct(QMenu& menu);
    void addZValueAction(QMenu& menu);
    void setMenuCaption(QMenu& menu);

    void addViewSubMenu(QMenu& menu);

private:
    friend class DeviceXlet;

private:
    QGraphicsTextItem* title_;
    QGraphicsSvgItem* image_;
    mutable SharedDeviceData data_;
    QRectF rect_;
    DeviceXlets xlets_;
};
} // namespace ceam

#endif // DEVICE_H
