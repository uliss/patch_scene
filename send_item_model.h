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
#ifndef SEND_ITEM_MODEL_H
#define SEND_ITEM_MODEL_H

#include "connection.h"
#include "device_common.h"

#include <QStandardItemModel>

class SendItemModel : public QStandardItemModel {
    Q_OBJECT
public:
    SendItemModel(QObject* parent);

    bool addConnection(const ConnectionData& data, const SharedDeviceData& src, const SharedDeviceData& dest);

    /**
     * Remove connection from model
     * @complexity O(n)
     */
    bool removeConnection(const ConnectionData& data);

    /**
     * Update connected device title
     * @complexity O(n)
     */
    bool updateDeviceTitle(DeviceId id, const QString& title);

    void clearItems();

    std::optional<DeviceId> deviceId(const QModelIndex& idx) const;

private:
    bool updateDeviceTitle(const QModelIndex& idx, DeviceId id, const QString& title);
};

#endif // SEND_ITEM_MODEL_H
