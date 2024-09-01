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

#include <QGraphicsObject>

class QGraphicsSvgItem;

namespace ceam {

class DeviceXlet;

class Device : public QGraphicsObject {
    Q_OBJECT
public:
    enum { Type = QGraphicsItem::UserType + 1 };
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
     * @return inlets bounding rect in device coordinates
     */
    QRectF inletsRect() const;

    /**
     * @return outlets bounding rect in device coordinates
     */
    QRectF outletsRect() const;

    /**
     * @return title bounding rect in device coordinates
     */
    QRectF titleRect() const;

    /**
     * @return if device has inputs or outputs
     */
    bool hasXlets() const;

    /**
     * @return all inputs/outputs boundinge rect in device coords
     */
    QRectF xletRect() const;

    /**
     * @return input connection point in device or scene coords
     */
    std::optional<QPointF> inConnectionPoint(XletIndex i, bool map = false) const;

    /**
     * @return output connection point in device or scene coords
     */
    std::optional<QPointF> outConnectionPoint(XletIndex i, bool map = false) const;

    SharedDeviceData deviceData() const;
    void setDeviceData(const SharedDeviceData& data);

    /**
     * move device into random neighborhood within the specified delta
     */
    void randomizePos(qint64 delta);

    const DeviceXletView& inlets() const { return inputs_; }
    const DeviceXletView& outputs() const { return outputs_; }

    /**
     * export device state/data to json
     */
    QJsonObject toJson() const;

    static SharedDeviceData defaultDeviceData();
    static SharedDeviceData datafromJson(const QJsonValue& j);

signals:
    void addToFavorites(SharedDeviceData data);
    void alignHorizontal();
    void alignVertical();
    void distributeHorizontal();
    void distributeVertical();
    void duplicateDevice(SharedDeviceData data);
    void removeDevice(SharedDeviceData data);
    void updateDevice(SharedDeviceData data);

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) final;
    void paintTitleBox(QPainter* painter);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) final;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;

    QJsonArray xletToJson(const QList<XletData>& xlets) const;

    int inletsYOff() const;
    int outletsYOff() const;

    void clearInlets();
    void clearOutlets();
    void clearXlets();
    void clearTitle();
    void clearImage();

    void createXlets();
    void createTitle(qreal wd);
    void createImage();

    void syncRect();
    void syncXletData();

    void updateTitlePos();
    void updateImagePos(const QRectF& bbox);
    void updateXletsPos(const QRectF& bbox);

    int calcWidth() const;
    int calcHeight() const;

    qreal centerAlignedLeftPos(qreal width) const
    {
        return rect_.left() + (rect_.width() - width) * 0.5;
    }

private:
    friend class DeviceXlet;

private:
    QGraphicsTextItem* title_;
    QGraphicsSvgItem* image_;
    mutable SharedDeviceData data_;
    QRectF rect_;
    DeviceXletView inputs_, outputs_;
};
}

#endif // DEVICE_H
