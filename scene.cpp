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
#include "scene.h"
#include "comment.h"
#include "scene_item.h"
#include "logging.hpp"

#include <QGraphicsScene>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

using namespace ceam;

Scene::Scene()
    : scene_(nullptr)
{
}

bool Scene::operator==(const Scene& sc) const
{
    if (this == &sc)
        return true;

    if (items_.size() != sc.items_.size())
        return false;

    QSet<DeviceData> d0, d1;

    for (auto& kv : items_) {
        auto data = kv.second->deviceData();
        data->setId(SCENE_ITEM_NULL_ID);
        d0.insert(*data);
    }

    for (auto& kv : sc.items_) {
        auto data = kv.second->deviceData();
        data->setId(SCENE_ITEM_NULL_ID);
        d1.insert(*data);
    }

    return d0 == d1;
}

void Scene::setGraphicsScene(QGraphicsScene* scene)
{
    scene_ = scene;
}

size_t Scene::selectedCount() const
{
    size_t res = 0;

    for (auto& kv : items_)
        res += kv.second->isSelected();

    return res;
}

SceneItem* Scene::add(const SharedDeviceData& data)
{
    if (!scene_ || !data)
        return nullptr;

    SceneItem* dev = nullptr;
    if (data->category() == ItemCategory::Comment) {
        dev = new CommentItem();
    } else {
        dev = new SceneItem(data);
    }

    scene_->addItem(dev);

    auto id = dev->id();
    auto it = items_.find(id);
    if (items_.find(id) != items_.end()) {
        WARN() << "device already with id" << id << "already exists in scene";
        scene_->removeItem(it->second);
        delete it->second;
        it->second = dev;
    } else {
        items_.insert(it, { dev->id(), dev });
    }

    emit added(dev->deviceData());

    return dev;
}

CommentItem* Scene::addComment()
{
    if (!scene_)
        return nullptr;

    auto c = new CommentItem();
    scene_->addItem(c);

    auto id = c->id();
    auto it = items_.find(id);
    if (items_.find(id) != items_.end()) {
        WARN() << "device already with id" << id << "already exists in scene";
        scene_->removeItem(it->second);
        delete it->second;
        it->second = c;
    } else {
        items_.insert(it, { c->id(), c });
    }

    // emit added(c->deviceData());

    return c;
}

SharedDeviceData Scene::remove(SceneItemId id)
{
    if (!scene_)
        return {};

    auto it = items_.find(id);
    if (it == items_.end()) {
        WARN() << "device not found:" << id;
        return {};
    }

    auto data = it->second->deviceData();
    scene_->removeItem(it->second);
    delete it->second;
    items_.erase(it);

    emit removed(data);
    return data;
}

SceneItem* Scene::find(SceneItemId id)
{
    auto it = items_.find(id);
    return it == items_.end()
        ? nullptr
        : it->second;
}

const SceneItem* Scene::find(SceneItemId id) const
{
    auto it = items_.find(id);
    return it == items_.end()
        ? nullptr
        : it->second;
}

SharedDeviceData Scene::findData(SceneItemId id) const
{
    auto it = items_.find(id);
    return it == items_.end()
        ? SharedDeviceData {}
        : it->second->deviceData();
}

std::optional<DeviceConnectionData> Scene::connectionInfo(const ConnectionId& id) const
{
    std::optional<DeviceConnectionData> res = DeviceConnectionData();
    int count = 0;

    for (auto& kv : items_) {
        if (count == 2)
            break;

        const auto dev_id = kv.first;
        const auto dev = kv.second;

        if (dev_id == id.source()) {
            const auto data = dev->deviceData();
            if (data && id.sourceIndex() < data->outputs().size()) {
                res->src_out = data->outputAt(id.sourceIndex());
                res->src_data = data;
                res->src_out_idx = id.sourceIndex();
                count++;
            } else {
                WARN() << "invalid source outlet:" << (int)id.sourceIndex();
            }
        } else if (dev_id == id.destination()) {
            const auto data = dev->deviceData();
            if (data && id.destinationIndex() < data->inputs().size()) {
                res->dest_in = data->inputAt(id.destinationIndex());
                res->dest_data = data;
                res->dest_in_idx = id.destinationIndex();
                count++;
            } else {
                WARN() << "invalid dest inlet:" << (int)id.destinationIndex();
            }
        }
    }

    if (count == 2)
        return res;
    else
        return {};
}

std::optional<std::pair<QPointF, QPointF>> Scene::connectionPoints(const ConnectionId& id) const
{
    auto src_it = items_.find(id.source());
    if (src_it == items_.end())
        return {};

    auto dest_it = items_.find(id.destination());
    if (dest_it == items_.end())
        return {};

    auto p0 = src_it->second->connectionPoint(id.sourceIndex(), id.sourceType(), true);
    if (!p0)
        return {};

    auto p1 = dest_it->second->connectionPoint(id.destinationIndex(), id.destinationType(), true);
    if (!p1)
        return {};

    return std::pair { *p0, *p1 };
}

std::optional<ConnectorPair> Scene::connectionPair(const ConnectionId& id) const
{
    auto src_it = items_.find(id.source());
    if (src_it == items_.end())
        return {};

    auto dest_it = items_.find(id.destination());
    if (dest_it == items_.end())
        return {};

    if (id.sourceIndex() >= src_it->second->deviceData()->outputs().count())
        return {};

    if (id.destinationIndex() >= dest_it->second->deviceData()->inputs().count())
        return {};

    auto& d0 = src_it->second->deviceData()->outputAt(id.sourceIndex());
    auto& d1 = dest_it->second->deviceData()->inputAt(id.destinationIndex());

    return ConnectorPair {
        ConnectorJack { d0.connectorModel(), d0.connectorType().complement() },
        ConnectorJack { d1.connectorModel(), d1.connectorType().complement() },
    };
}

bool Scene::checkConnection(const ConnectionId& id) const
{
    if (!id.isValid())
        return false;

    auto src_it = items_.find(id.source());
    if (src_it == items_.end())
        return false;

    if (id.sourceIndex() >= src_it->second->deviceData()->outputs().size())
        return false;

    auto dest_it = items_.find(id.destination());
    if (dest_it == items_.end())
        return false;

    if (id.destinationIndex() >= dest_it->second->deviceData()->inputs().size())
        return false;

    return true;
}

bool Scene::hasSelected() const
{
    for (auto& kv : items_)
        if (kv.second->isSelected())
            return true;

    return false;
}

int Scene::setSelected(const QList<SceneItemId>& ids, bool value)
{
    QSignalBlocker sb(scene_);

    int count = 0;

    for (auto id : ids) {
        auto it = items_.find(id);
        if (it != items_.end()) {
            it->second->setSelected(value);
            count++;
        }
    }

    return count;
}

void Scene::toggleSelected(const QList<SceneItemId>& ids)
{
    QSignalBlocker sb(scene_);

    for (auto id : ids) {
        auto it = items_.find(id);
        if (it != items_.end())
            it->second->setSelected(!it->second->isSelected());
    }
}

void Scene::clear()
{
    if (scene_) {
        for (auto& kv : items_) {
            scene_->removeItem(kv.second);
            emit removed(kv.second->deviceData());
            delete kv.second;
        }
    }

    items_.clear();
}

QList<SceneItemId> Scene::idList() const
{
    QList<SceneItemId> res;
    res.reserve(items_.size());

    for (auto& kv : items_)
        res.push_back(kv.second->id());

    return res;
}

QList<SharedDeviceData> Scene::dataList() const
{
    QList<SharedDeviceData> res;
    res.reserve(items_.size());

    for (auto& kv : items_)
        res.push_back(kv.second->deviceData());

    return res;
}

QList<SceneItemId> Scene::selectedIdList() const
{
    QList<SceneItemId> res;

    foreachSelectedData([&res](const SharedDeviceData& data) {
        res.push_back(data->id());
    });

    return res;
}

QList<SharedDeviceData> Scene::selectedDataList() const
{
    QList<SharedDeviceData> res;
    foreachSelectedData([&res](const SharedDeviceData& data) {
        res.push_back(data);
    });
    return res;
}

QRectF Scene::boundingRect() const
{
    QRectF rect;
    int i = 0;
    for (auto& kv : items_) {
        auto dev = kv.second;
        auto item_rect = dev->mapRectToScene(dev->boundingRect());

        if (rect.isNull())
            rect = item_rect;
        else
            rect |= item_rect;
    }

    return rect;
}

QRectF Scene::boundingSelectRect() const
{
    QRectF rect;
    int i = 0;
    for (auto& kv : items_) {
        auto dev = kv.second;
        if (!dev->isSelected())
            continue;

        auto item_rect = dev->mapRectToScene(dev->boundingRect());

        if (rect.isNull())
            rect = item_rect;
        else
            rect |= item_rect;
    }

    return rect;
}

void Scene::foreachDevice(const std::function<void(SceneItem*)>& fn)
{
    if (!fn)
        return;

    for (auto& kv : items_)
        fn(kv.second);
}

void Scene::foreachSelectedDevice(const std::function<void(const SceneItem*)>& fn)
{
    if (!fn)
        return;

    for (auto& kv : items_) {
        if (kv.second->isSelected())
            fn(kv.second);
    }
}

void Scene::foreachData(const std::function<void(const SharedDeviceData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : items_)
        fn(kv.second->deviceData());
}

void Scene::foreachSelectedData(const std::function<void(const SharedDeviceData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : items_) {
        if (kv.second->isSelected())
            fn(kv.second->deviceData());
    }
}

QSet<SceneItemId> Scene::intersected(const QRectF& rect) const
{
    QSet<SceneItemId> res;

    for (auto& kv : items_) {
        auto dev = kv.second;
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (scene_bbox.intersects(rect))
            res.insert(kv.first);
    }

    return res;
}

QList<SceneItemId> Scene::intersectedList(const QRectF& rect) const
{
    QList<SceneItemId> res;

    for (auto& kv : items_) {
        auto dev = kv.second;
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (scene_bbox.intersects(rect))
            res << kv.first;
    }

    return res;
}

QJsonValue Scene::toJson() const
{
    QJsonArray res;

    for (auto& kv : items_)
        res << kv.second->toJson();

    return res;
}

bool Scene::moveBy(const QHash<SceneItemId, QPointF>& deltas)
{
    int count = 0;

    for (auto kv = deltas.begin(); kv != deltas.end(); ++kv) {
        auto it = items_.find(kv.key());
        if (it != items_.end() && !it->second->isLocked()) {
            it->second->moveBy(kv.value().x(), kv.value().y());
            count++;
        }
    }

    return count > 0;
}

bool Scene::moveSelectedBy(qreal dx, qreal dy)
{
    bool res = false;

    for (auto& kv : items_) {
        auto dev = kv.second;
        if (dev->isSelected() && !dev->isLocked()) {
            dev->moveBy(dx, dy);
            res = true;
        }
    }

    return res;
}

QDebug operator<<(QDebug debug, const ceam::Scene& sc)
{
    sc.foreachData([&debug](const SharedDeviceData& data) {
        if (data)
            debug << *data << "\n";
    });
    return debug;
}
