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
#include "comment_item.h"
#include "device_item.h"
#include "logging.hpp"
#include "scene_item.h"

#include <QGraphicsScene>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

using namespace ceam;

Scene::Scene()
    : scene_(nullptr)
    , edited_comment_id_ { SCENE_ITEM_NULL_ID }
{
}

bool Scene::operator==(const Scene& sc) const
{
    if (this == &sc)
        return true;

    if (items_.size() != sc.items_.size())
        return false;

    QSet<ItemData> d0, d1;

    for (auto& kv : items_) {
        auto data = kv.second->itemData();
        data->setId(SCENE_ITEM_NULL_ID);
        d0.insert(*data);
    }

    for (auto& kv : sc.items_) {
        auto data = kv.second->itemData();
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

SceneItem* Scene::add(const SharedItemData& data)
{
    if (!scene_ || !data)
        return nullptr;

    SceneItem* item = nullptr;
    if (data->category() == ItemCategory::Comment) {
        auto x = new CommentItem();
        connect(x, &CommentItem::editComment, this, [this](SceneItemId id) {
            edited_comment_id_ = id;
            emit showCommentEditor(id == SCENE_ITEM_NULL_ID);
        });
        item = x;
    } else {
        item = new DeviceItem(data);
    }

    scene_->addItem(item);

    auto id = item->id();
    auto it = items_.find(id);
    if (items_.find(id) != items_.end()) {
        WARN() << "item already with id" << id << "already exists in scene";
        scene_->removeItem(it->second);
        delete it->second;
        it->second = item;
    } else {
        items_.insert(it, { item->id(), item });
    }

    emit added(item->itemData());

    return item;
}

SharedItemData Scene::remove(SceneItemId id)
{
    if (!scene_)
        return {};

    auto it = items_.find(id);
    if (it == items_.end()) {
        WARN() << "device not found:" << id;
        return {};
    }

    auto data = it->second->itemData();
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

SharedItemData Scene::findData(SceneItemId id) const
{
    auto it = items_.find(id);
    return it == items_.end()
        ? SharedItemData {}
        : it->second->itemData();
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
            const auto data = dev->itemData();
            if (data && id.sourceIndex() < data->outputs().size()) {
                res->src_out = data->outputAt(id.sourceIndex());
                res->src_data = data;
                res->src_out_idx = id.sourceIndex();
                count++;
            } else {
                WARN() << "invalid source outlet:" << (int)id.sourceIndex();
            }
        } else if (dev_id == id.destination()) {
            const auto data = dev->itemData();
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
    if (src_it == items_.end()) {
        // WARN() << "source not found: " << id.source();
        return {};
    }

    auto dest_it = items_.find(id.destination());
    if (dest_it == items_.end()) {
        // WARN() << "destination not found: " << id.source();
        return {};
    }

    if (id.sourceIndex() >= src_it->second->itemData()->outputs().count()) {
        // WARN() << "invalid source index: " << id.sourceIndex();
        return {};
    }

    if (id.destinationIndex() >= dest_it->second->itemData()->inputs().count()) {
        // WARN() << "invalid destination index: " << id.destinationIndex();
        return {};
    }

    auto& d0 = src_it->second->itemData()->outputAt(id.sourceIndex());
    auto& d1 = dest_it->second->itemData()->inputAt(id.destinationIndex());

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

    if (id.sourceIndex() >= src_it->second->itemData()->outputs().size())
        return false;

    auto dest_it = items_.find(id.destination());
    if (dest_it == items_.end())
        return false;

    if (id.destinationIndex() >= dest_it->second->itemData()->inputs().size())
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
            emit removed(kv.second->itemData());
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

QList<SharedItemData> Scene::dataList() const
{
    QList<SharedItemData> res;
    res.reserve(items_.size());

    for (auto& kv : items_)
        res.push_back(kv.second->itemData());

    return res;
}

QList<SceneItemId> Scene::selectedIdList() const
{
    QList<SceneItemId> res;

    foreachSelectedData([&res](const SharedItemData& data) {
        res.push_back(data->id());
    });

    return res;
}

QList<SharedItemData> Scene::selectedDataList() const
{
    QList<SharedItemData> res;
    foreachSelectedData([&res](const SharedItemData& data) {
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

void Scene::foreachItem(const std::function<void(SceneItem*)>& fn)
{
    if (!fn)
        return;

    for (auto& kv : items_)
        fn(kv.second);
}

void Scene::foreachSelectedItem(const std::function<void(const SceneItem*)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : items_) {
        if (kv.second->isSelected())
            fn(kv.second);
    }
}

void Scene::foreachData(const std::function<void(const SharedItemData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : items_)
        fn(kv.second->itemData());
}

void Scene::foreachSelectedData(const std::function<void(const SharedItemData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : items_) {
        if (kv.second->isSelected())
            fn(kv.second->itemData());
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

void Scene::doneCommentEditor()
{
    if (edited_comment_id_ == SCENE_ITEM_NULL_ID)
        return;

    auto comment = dynamic_cast<CommentItem*>(find(edited_comment_id_));
    if (!comment)
        return;

    comment->setEditable(false);
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
    sc.foreachData([&debug](const SharedItemData& data) {
        if (data)
            debug << *data << "\n";
    });
    return debug;
}
