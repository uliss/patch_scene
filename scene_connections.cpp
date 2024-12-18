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
#include "connection_editor.h"
#include "logging.hpp"
#include "scene_devices.h"

#include <QGraphicsScene>

using namespace ceam;

SceneConnections::SceneConnections(QGraphicsScene* scene, QObject* parent)
    : QObject(parent)
    , scene_(scene)
{
    conn_edit_ = new ConnectionEditor(this);
    conn_edit_->setVisible(false);
    connect(conn_edit_, &ConnectionEditor::connectionUpdated, this,
        [this](const ConnectionId& id, const ConnectionViewData& viewData) {
            setViewData(id, viewData);
            emit update(id);
        });
    scene_->addItem(conn_edit_);
}

Connection* SceneConnections::add(const ConnectionId& id)
{
    if (!scene_ || !id.isValid())
        return nullptr;

    auto src_it = conn_xlets_.find(id.sourceInfo());
    if (src_it != conn_xlets_.end()) {
        qWarning() << "connection already exists";
        return nullptr;
    }

    auto dest_it = conn_xlets_.find(id.destinationInfo());
    if (dest_it != conn_xlets_.end()) {
        qWarning() << "connection already exists";
        return nullptr;
    }

    std::unique_ptr<Connection> conn(new Connection(id));
    if (addConnection(conn.get())) {
        return conn.release();
    } else {
        return nullptr;
    }
}

bool SceneConnections::setViewData(const ConnectionId& id, const ConnectionViewData& viewData)
{
    auto it = conn_dev_.find(id.source());
    if (it == conn_dev_.end())
        return false;

    for (auto c : *it) {
        if (c->connectionId() == id) {
            c->setViewData(viewData);
            return true;
        }
    }

    return false;
}

bool SceneConnections::remove(const ConnectionId& id)
{
    if (!scene_)
        return false;

    auto dest_it = conn_xlets_.find(id.sourceInfo());
    auto src_it = conn_xlets_.find(id.destinationInfo());

    if (dest_it == conn_xlets_.end() || src_it == conn_xlets_.end())
        return false;

    return removeConnection(src_it.value());
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

void SceneConnections::foreachConn(std::function<void(const ConnectionId&, const ConnectionViewData&)> fn) const
{
    if (!fn)
        return;

    for (auto c : conn_)
        fn(c->connectionId(), c->viewData());
}

QList<ConnectionInfo> SceneConnections::selectedList() const
{
    QList<ConnectionInfo> res;

    for (auto& kv : conn_) {
        if (kv->isSelected())
            res << ConnectionInfo { kv->connectionId(), kv->viewData() };
    }

    return res;
}

void SceneConnections::unselectAll()
{
    for (auto& kv : conn_) {
        if (kv->isSelected())
            kv->setSelected(false);
    }
}

QList<DeviceConnectionData> SceneConnections::infoList(const SceneDevices& devices) const
{
    QList<DeviceConnectionData> res;
    res.reserve(conn_.size());

    for (auto conn : conn_) {
        auto& data = conn->connectionId();
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

QList<ConnectionInfo> SceneConnections::findConnectionsData(DeviceId id) const
{
    auto dev_it = conn_dev_.find(id);
    if (dev_it != conn_dev_.end()) {
        QList<ConnectionInfo> res;
        res.reserve(dev_it.value().size());

        for (auto c : dev_it.value()) {
            res.append({ c->connectionId(), c->viewData() });
        }

        return res;
    } else
        return {};
}

Connection* SceneConnections::findByXlet(const XletInfo& xlet) const
{
    auto src_it = conn_xlets_.find(xlet);
    return (src_it != conn_xlets_.end())
        ? src_it.value()
        : nullptr;
}

bool SceneConnections::checkConnection(const std::pair<XletInfo, XletData>& x0, const std::pair<XletInfo, XletData>& x1) const
{
    if (x0.first.type() == XletType::None || x1.first.type() == XletType::None) {
        qWarning() << "none xlet type connection attempt";
        return false;
    }

    if (x0.first.id() == x1.first.id()) {
        qWarning() << "self connection attempt";
        return false;
    }

    if ((!x0.second.isBidirect() && !x1.second.isBidirect()) && x0.first.type() == x1.first.type()) {
        qWarning() << "same xlet type connection attempt";
        return false;
    }

    ConnectionId data(x0.first, x1.first);
    if (findByXlet(data.sourceInfo())) {
        qWarning() << "already connected from this source";
        return false;
    }

    if (findByXlet(data.destinationInfo())) {
        qWarning() << "already connected to this destination";
        return false;
    }

    return true;
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

void SceneConnections::showEditor(bool value)
{
    conn_edit_->setVisible(value);
}

size_t SceneConnections::count() const
{
    return conn_.size();
}

void SceneConnections::clear()
{
    for (auto c : conn_) {
        scene_->removeItem(c);
        emit removed(c->connectionId());
        delete c;
    }

    conn_.clear();
    conn_xlets_.clear();
    conn_dev_.clear();
}

bool SceneConnections::addConnection(Connection* c)
{
    if (!c || !scene_)
        return false;

    scene_->addItem(c);
    auto it = conn_.find(c);
    if (conn_.find(c) != conn_.end()) {
        WARN() << "connection already exists in the scene";
        scene_->removeItem(c);
        delete *it;
        conn_.insert(c);
    } else
        conn_.insert(c);

    conn_xlets_[c->sourceInfo()] = c;
    conn_xlets_[c->destinationInfo()] = c;
    conn_dev_[c->sourceInfo().id()] << c;
    conn_dev_[c->destinationInfo().id()] << c;

    connect(c, &Connection::changed, this, &SceneConnections::update);
    connect(c, &Connection::edited, this,
        [this](const ConnectionId& id, const ConnectionViewData& viewData) {
            conn_edit_->setConnectionData(id, viewData);
            emit edit(id);
        });

    connect(c, &Connection::selected, this,
        [this](const Connection* conn) {
            for (auto& c : conn_) {
                c->setSelected(c == conn);
            }
        });

    connect(c, &Connection::splited, this,
        [this](const ConnectionId& id, const QPointF& pos) {
            for (auto& c : conn_) {
                if (c->connectionId() == id) {
                    c->splitSegment(pos);
                    conn_edit_->setConnectionData(id, c->viewData());
                    emit edit(id);
                    emit update(id);
                    break;
                }
            }
        });

    connect(c, &Connection::reset, this,
        [this](ConnectionId id, ConnectionCordType cord) {
            for (auto& c : conn_) {
                if (c->connectionId() == id) {
                    c->resetCordPoints(cord);
                    conn_edit_->setConnectionData(id, c->viewData());
                    emit update(id);
                    break;
                }
            }
        });

    connect(
        c, &Connection::removeRequested, this,
        [this](ConnectionId id) { remove(id); },
        Qt::SingleShotConnection);

    emit added(c->connectionId());

    return true;
}

bool SceneConnections::removeConnection(Connection* c)
{
    if (!c || !scene_)
        return false;

    scene_->removeItem(c);
    conn_.erase(c);
    conn_xlets_.remove(c->sourceInfo());
    conn_xlets_.remove(c->destinationInfo());

    auto src_it = conn_dev_.find(c->sourceInfo().id());
    if (src_it != conn_dev_.end())
        src_it.value().removeAll(c);

    auto dest_it = conn_dev_.find(c->destinationInfo().id());
    if (dest_it != conn_dev_.end())
        dest_it.value().removeAll(c);

    emit removed(c->connectionId());

    delete c;

    return true;
}
