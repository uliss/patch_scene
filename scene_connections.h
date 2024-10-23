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
#ifndef SCENE_CONNECTIONS_H
#define SCENE_CONNECTIONS_H

#include "connection.h"
#include "device_common.h"

#include <QHash>
#include <QObject>

#include <unordered_set>

namespace ceam {

class Connection;
class ConnectionEditor;
class SceneDevices;

class SceneConnections : public QObject {
    Q_OBJECT

public:
    SceneConnections(QGraphicsScene* scene, QObject* parent = nullptr);

    /**
     * adds new connection by given data
     * @param id - connection data
     * @return pointer to new Connection on nullptr on error
     * @emit added()
     */
    Connection* add(const ConnectionId& id);

    /**
     * @complexity O(N)
     */
    bool setViewData(const ConnectionId& id, const ConnectionViewData& viewData);

    /**
     * remove connection from/to specified input/output
     * @param xlet
     * @return true if connection was removed
     * @emit removed()
     */
    bool remove(const XletInfo& xlet);

    /**
     * remove all connections belonging to the given device id
     * @param id - target device id
     * @emit removed()
     */
    void removeAll(DeviceId id);

    /**
     * iterate all connections with given function
     */
    void foreachConn(std::function<void(const ConnectionId&, const ConnectionViewData&)> fn) const;

    /**
     * return all selected connections as list
     */

    QList<ConnectionInfo> selectedList() const;

    QList<DeviceConnectionData> infoList(const SceneDevices& devices) const;

    /**
     * find all incoming/outcoming connection to/from devices
     * @complexity Amort. O(1) (worst case: O(n))
     */
    QList<Connection*> findConnections(DeviceId id) const;

    /**
     * find all incoming/outcoming connection data to/from devices
     * @complexity Amort. O(1) (worst case: O(n))
     */
    QList<ConnectionInfo> findConnectionsData(DeviceId id) const;

    /**
     * find connection by specified xlet
     * @param xlet
     * @return connection pointer or null if not found
     * @complexity O(1)
     */
    Connection* findConnection(const XletInfo& xlet) const;

    /**
     * check if specified xlets can be connected
     * @param x0
     * @param x1
     * @return
     */
    bool checkConnection(const XletInfo& x0, const XletInfo& x1) const;

    /**
     * Set all connections visible
     * @complexity O(n)
     * @emit visibleChanged()
     */
    void setVisible(bool value);

    /**
     * Show/hide connection editor
     */
    void showEditor(bool value);

    /**
     * returns number of connections
     */
    size_t count() const;

    /**
     * @emit removed() for every connection
     */
    void clear();

signals:
    void added(ConnectionId);
    void removed(ConnectionId);
    void update(ConnectionId);
    void edit(ConnectionId);
    void visibleChanged(bool);

private:
    bool addConnection(Connection* c);
    bool removeConnection(Connection* c);

private:
    QGraphicsScene* scene_ { nullptr };
    ConnectionEditor* conn_edit_ { nullptr };
    std::unordered_set<Connection*> conn_;
    QHash<XletInfo, Connection*> conn_src_, conn_dest_;
    QHash<DeviceId, QList<Connection*>> conn_dev_;
};

}

#endif // SCENE_CONNECTIONS_H
