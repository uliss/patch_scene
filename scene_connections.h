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

#include <QHash>
#include <QObject>

#include <unordered_set>

namespace ceam {

class Connection;

class SceneConnections : public QObject {
    Q_OBJECT
public:
    SceneConnections(QObject* parent = nullptr);

    Connection* add(const ConnectionData& connData);
    bool remove(const XletInfo& xlet);
    bool removeAll(DeviceId id);

    void foreachData(std::function<void(const ConnectionData& data)> fn) const;

    /**
     * find all incoming/outcoming connection to/from devices
     * @complexity Amort. O(1) (worst case: O(n))
     */
    QList<Connection*> findConnections(DeviceId id) const;

    /**
     * find all incoming/outcoming connection data to/from devices
     * @complexity Amort. O(1) (worst case: O(n))
     */
    QList<ConnectionData> findConnectionsData(DeviceId id) const;

    /**
     * find connection by specified xlet
     * @param xlet
     * @return connection data or null if not found
     * @complexity O(1)
     */
    std::optional<ConnectionData> findConnection(const XletInfo& xlet) const;

    /**
     * Set connection visible
     * @complexity O(n)
     */
    void setVisible(bool value);

    void setScene(QGraphicsScene* scene);
    size_t count() const;

    void clear();

private:
    bool addConnection(Connection* c);
    bool removeConnection(Connection* c);

private:
    QGraphicsScene* scene_ { nullptr };
    std::unordered_set<Connection*> conn_;
    QHash<XletInfo, Connection*> conn_src_, conn_dest_;
    QHash<DeviceId, QList<Connection*>> conn_dev_;
};

}

#endif // SCENE_CONNECTIONS_H
