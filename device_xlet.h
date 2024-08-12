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
#ifndef DEVICE_XLET_H
#define DEVICE_XLET_H

#include "socket.h"

#include <QGraphicsSvgItem>

struct XletData {
    QString name;
    ConnectorModel model { ConnectorModel::UNKNOWN };
    ConnectorType type { ConnectorType::Socket_Female };
    bool visible { true };
    bool phantom_power { false };
    QString level;
    QColor color_bg { Qt::white };

    QString modelString() const;
    QString typeString() const;

    QJsonObject toJson() const;

    static bool fromJson(const QJsonValue& j, XletData& data);
};

class DeviceXlet : public QGraphicsSvgItem {
public:
    DeviceXlet(const XletData& data, XletType type, QGraphicsItem* parentItem);

    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    const XletData& xletData() const;
    XletType zletType() const { return type_; }
    QString iconPath() const;
    void setConnectPoint(const QPointF& pos);

    bool isInlet() const { return type_ == XletType::In; }
    bool isOutlet() const { return type_ == XletType::Out; }

private:
    XletData data_;
    XletType type_ { XletType::In };
};

#endif // DEVICE_XLET_H
