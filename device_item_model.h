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
#ifndef DEVICE_ITEM_MODEL_H
#define DEVICE_ITEM_MODEL_H

#include "device_common.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace ceam {

class DeviceItemModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit DeviceItemModel(QObject* parent);

    int deviceCount() const { return rowCount(); }

    bool addDevice(const SharedDeviceData& data);
    bool removeDevice(const SharedDeviceData& data);
    void clearItems();

    void setDeviceData(const QList<SharedDeviceData>& data);

    QStandardItem* deviceTitle(int idx);
    QStandardItem* deviceVendor(int idx);
    QStandardItem* deviceModel(int idx);

    std::optional<DeviceId> deviceId(const QStandardItem* item) const;
    std::optional<DeviceId> deviceId(int idx) const;

    SharedDeviceData updateDeviceData(const QStandardItem* item, const SharedDeviceData& data);

    QSortFilterProxyModel* sortProxy() { return proxy_; }

public:
    static QList<QString> headerLabels();

private:
    QSortFilterProxyModel* proxy_;
};

}  // namespace ceam

#endif // DEVICE_ITEM_MODEL_H
