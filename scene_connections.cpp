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
#include "scene_connections.h"
#include "connection.h"
#include "scene_devices.h"

#include <QGraphicsScene>

using namespace ceam;

#define WARN() qWarning() << metaObject()->className() << __FUNCTION__

SceneConnections::SceneConnections(QObject* parent)
    : QObject(parent)
{
}

Connection* SceneConnections::add(const ConnectionData& connData)
{
    if (!scene_ || !connData.isValid())
        return nullptr;

    auto src_it = conn_src_.find(connData.sourceInfo());
    if (src_it != conn_src_.end()) {
        qWarning() << "connection already exists";
        return nullptr;
    }

    auto dest_it = conn_dest_.find(connData.destinationInfo());
    if (dest_it != conn_dest_.end()) {
        qWarning() << "connection already exists";
        return nullptr;
    }

    std::unique_ptr<Connection> conn(new Connection(connData));
    if (addConnection(conn.get())) {
        return conn.release();
    } else {
        return nullptr;
    }
}

bool SceneConnections::remove(const XletInfo& xlet)
{
    if (!scene_)
        return false;

    switch (xlet.type()) {
    case XletType::In: {
        auto dest_it = conn_dest_.find(xlet);
        if (dest_it != conn_dest_.end())
            return removeConnection(dest_it.value());
    } break;
    case XletType::Out: {
        auto src_it = conn_src_.find(xlet);
        if (src_it != conn_src_.end())
            return removeConnection(src_it.value());
    } break;
    case XletType::None:
    default:
        break;
    }

    return false;
}

void SceneConnections::removeAll(DeviceId id)
{
    auto dev_it = conn_dev_.find(id);
    if (dev_it != conn_dev_.end()) {
        // NB: QList copy! to prevent conn_dev_ iterator invalidations
        const auto connections = dev_it.value();
        for (auto conn : connections)
            removeConnection(conn);
    }
}

void SceneConnections::foreachData(std::function<void(const ConnectionData&)> fn) const
{
    if (!fn)
        return;

    for (auto c : conn_)
        fn(c->connectionData());
}

QList<ConnectionData> SceneConnections::dataList() const
{
    QList<ConnectionData> res;
    res.reserve(conn_.size());

    for (auto& kv : conn_)
        res.append(kv->connectionData());

    return res;
}

QList<ConnectionFullInfo> SceneConnections::infoList(const SceneDevices& devices) const
{
    QList<ConnectionFullInfo> res;
    res.reserve(conn_.size());

    for (auto conn : conn_) {
        auto& data = conn->connectionData();
        auto info = devices.connectionInfo(data);
        if (info)
            res.append(info.value());
    }

    return res;
}

QList<Connection*> SceneConnections::findConnections(DeviceId id) const
{
    auto dev_it = conn_dev_.find(id);
    if (dev_it != conn_dev_.end()) {
        return dev_it.value();
    } else
        return {};
}

QList<ConnectionData> SceneConnections::findConnectionsData(DeviceId id) const
{
    auto dev_it = conn_dev_.find(id);
    if (dev_it != conn_dev_.end()) {
        QList<ConnectionData> res;
        res.reserve(dev_it.value().size());

        for (auto c : dev_it.value()) {
            res.append(c->connectionData());
        }

        return res;
    } else
        return {};
}

std::optional<ConnectionData> SceneConnections::findConnection(const XletInfo& xlet) const
{
    switch (xlet.type()) {
    case XletType::In: {
        auto dest_it = conn_dest_.find(xlet);
        if (dest_it != conn_dest_.end())
            return dest_it.value()->connectionData();
    } break;
    case XletType::Out: {
        auto src_it = conn_src_.find(xlet);
        if (src_it != conn_src_.end())
            return src_it.value()->connectionData();
    } break;
    case XletType::None:
    default:
        break;
    }

    return {};
}

void SceneConnections::setVisible(bool value)
{
    int counter = 0;

    for (auto c : conn_) {
        if (c->isVisible() != value) {
            c->setVisible(value);
            counter++;
        }
    }

    if (counter > 0)
        emit visibleChanged(value);
}

void SceneConnections::setScene(QGraphicsScene* scene)
{
    scene_ = scene;
}

size_t SceneConnections::count() const
{
    return conn_.size();
}

void SceneConnections::clear()
{
    for (auto c : conn_) {
        scene_->removeItem(c);
        emit removed(c->connectionData());
        delete c;
    }

    conn_.clear();
    conn_src_.clear();
    conn_dest_.clear();
    conn_dev_.clear();
}

bool SceneConnections::addConnection(Connection* c)
{
    if (!c || !scene_)
        return false;

    scene_->addItem(c);
    auto it = conn_.find(c);
    if (conn_.find(c) != conn_.end()) {
        qWarning() << "connection already exists in the scene";
        scene_->removeItem(c);
        delete *it;
        conn_.insert(c);
    } else
        conn_.insert(c);

    conn_src_[c->sourceInfo()] = c;
    conn_dest_[c->destinationInfo()] = c;
    conn_dev_[c->sourceInfo().id()] << c;
    conn_dev_[c->destinationInfo().id()] << c;

    emit added(c->connectionData());

    return true;
}

bool SceneConnections::removeConnection(Connection* c)
{
    if (!c || !scene_)
        return false;

    scene_->removeItem(c);
    conn_.erase(c);
    conn_src_.remove(c->sourceInfo());
    conn_dest_.remove(c->destinationInfo());

    auto src_it = conn_dev_.find(c->sourceInfo().id());
    if (src_it != conn_dev_.end())
        src_it.value().removeAll(c);

    auto dest_it = conn_dev_.find(c->destinationInfo().id());
    if (dest_it != conn_dev_.end())
        dest_it.value().removeAll(c);

    emit removed(c->connectionData());

    delete c;

    return true;
}
