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

void SceneConnections::setScene(QGraphicsScene* scene)
{
    scene_ = scene;
}

size_t SceneConnections::count() const
{
    return conn_.size();
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
    delete c;

    return true;
}
