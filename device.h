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

#include "connection.h"
#include "device_common.h"
#include "device_xlet.h"

#include <QGraphicsItem>

enum class BatteryType {
    None,
    AA,
    AAA,
    Crona
};

class DeviceData : public QSharedData {
public:
    QList<XletData> inlets;
    QList<XletData> outlets;
    QString name;
    QString image;
    QPointF pos;
    DeviceId id { 0 };
    qreal zvalue = { 1 };
    ItemCategory category { ItemCategory::Device };
    BatteryType battery_ { BatteryType::None };
    qreal zoom = { 1 };

    size_t visInletCount() const;
    size_t visOutletCount() const;
};

using SharedDeviceData = QSharedDataPointer<DeviceData>;

class Device : public QGraphicsRectItem {
public:
    explicit Device();
    explicit Device(const SharedDeviceData& data);
    ~Device();

    QPointF inletPos(int i, bool map = false) const;
    QPointF outletPos(int i, bool map = false) const;
    QRect inletRect(int i) const;
    QRect outletRect(int i) const;

    DeviceId id() const { return data_->id; }

    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    void incrementName();

    QJsonObject toJson() const;
    static bool fromJson(const QJsonValue& j, Device& dev);

    int inletAt(const QPointF& pt) const;
    int outletAt(const QPointF& pt) const;

    SharedDeviceData deviceData() const;
    void setDeviceData(const SharedDeviceData& data);

    static constexpr DeviceId NullID { 0 };

private:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void paintTitleBox(QPainter* painter);
    void paintInlets(QPainter* painter);
    void paintOutlets(QPainter* painter);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    size_t inletCount() const;
    size_t outletCount() const;
    bool noXlets() const;

    QJsonArray xletToJson(const QList<XletData>& xlets) const;

    void clearInlets();
    void clearOutlets();
    void clearXlets();
    void clearTitle();
    void clearImage();

    void createXlets();
    void createTitle(qreal wd);
    void createImage(qreal wd);

    void syncRect();

    void updateTitlePos();
    void updateImagePos();
    void updateXletsPos();

    QRectF titleRect() const;
    QRectF xletRect() const;

    qreal centerXOff() const;
    int calcWidth() const;
    int calcHeight() const;

    QGraphicsTextItem* title_;
    QGraphicsSvgItem* image_;
    mutable SharedDeviceData data_;
};

#endif // DEVICE_H
