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
#ifndef CONNECTION_ITEM_MODEL_H
#define CONNECTION_ITEM_MODEL_H

#include "connection.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace ceam {

class ConnectionId;
class ConnectionFullInfo;
class XletData;

class ConnectionItemModel : public QStandardItemModel {
    Q_OBJECT
public:
    ConnectionItemModel(QObject* parent);

    bool addConnection(const ConnectionFullInfo& info);

    /**
     * @complexity: O(n)
     */
    bool updateDeviceTitle(DeviceId id, const QString& title);

    /**
     * @brief removeConnection
     * @complexity: O(n)
     */
    bool removeConnection(const ConnectionId& data);

    std::optional<DeviceId> deviceId(const QModelIndex& idx) const;

    void clearItems();

    QSortFilterProxyModel* sortProxy() { return proxy_; }

    void setFullData(const QList<ConnectionFullInfo>& conn);

private:
    bool updateDeviceTitle(const QModelIndex& idx, DeviceId id, const QString& title);

private:
    QSortFilterProxyModel* proxy_;
};
}

#endif // CONNECTION_ITEM_MODEL_H
