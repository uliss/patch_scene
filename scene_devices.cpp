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
#include "scene_devices.h"
#include "comment.h"
#include "device.h"
#include "logging.hpp"

#include <QGraphicsScene>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>

using namespace ceam;

SceneDevices::SceneDevices()
    : scene_(nullptr)
{
}

bool SceneDevices::operator==(const SceneDevices& sc) const
{
    if (this == &sc)
        return true;

    if (devices_.size() != sc.devices_.size())
        return false;

    QSet<DeviceData> d0, d1;

    for (auto& kv : devices_) {
        auto data = kv.second->deviceData();
        data->setId(DEV_NULL_ID);
        d0.insert(*data);
    }

    for (auto& kv : sc.devices_) {
        auto data = kv.second->deviceData();
        data->setId(DEV_NULL_ID);
        d1.insert(*data);
    }

    return d0 == d1;
}

void SceneDevices::setScene(QGraphicsScene* scene)
{
    scene_ = scene;
}

size_t SceneDevices::selectedCount() const
{
    size_t res = 0;

    for (auto& kv : devices_)
        res += kv.second->isSelected();

    return res;
}

Device* SceneDevices::add(const SharedDeviceData& data)
{
    if (!scene_ || !data)
        return nullptr;

    Device* dev = nullptr;
    if (data->category() == ItemCategory::Comment) {
        dev = new Comment();
    } else {
        dev = new Device(data);
    }

    scene_->addItem(dev);

    auto id = dev->id();
    auto it = devices_.find(id);
    if (devices_.find(id) != devices_.end()) {
        WARN() << "device already with id" << id << "already exists in scene";
        scene_->removeItem(it->second);
        delete it->second;
        it->second = dev;
    } else {
        devices_.insert(it, { dev->id(), dev });
    }

    emit added(dev->deviceData());

    return dev;
}

Comment* SceneDevices::addComment()
{
    if (!scene_)
        return nullptr;

    auto c = new Comment();
    scene_->addItem(c);

    auto id = c->id();
    auto it = devices_.find(id);
    if (devices_.find(id) != devices_.end()) {
        WARN() << "device already with id" << id << "already exists in scene";
        scene_->removeItem(it->second);
        delete it->second;
        it->second = c;
    } else {
        devices_.insert(it, { c->id(), c });
    }

    // emit added(c->deviceData());

    return c;
}

SharedDeviceData SceneDevices::remove(DeviceId id)
{
    if (!scene_)
        return {};

    auto it = devices_.find(id);
    if (it == devices_.end()) {
        WARN() << "device not found:" << id;
        return {};
    }

    auto data = it->second->deviceData();
    scene_->removeItem(it->second);
    delete it->second;
    devices_.erase(it);

    emit removed(data);
    return data;
}

QList<DeviceId> SceneDevices::duplicateSelected(DuplicatePolicy policy)
{
    QList<Device*> dup_list;
    for (auto& kv : devices_) {
        auto src_dev = kv.second;
        if (!src_dev->isSelected())
            continue;

        dup_list << src_dev;
    }

    QList<DeviceId> res;
    if (dup_list.empty())
        return res;

    for (auto src_dev : dup_list) {
        auto new_dev = add(src_dev->deviceData());
        if (new_dev) {
            switch (new_dev->deviceData()->category()) {
            case ItemCategory::Furniture:
                new_dev->moveBy(50, 0);
                break;
            default:
                new_dev->moveBy(20, 20);
                break;
            }

            res.push_back(new_dev->id());

            if (policy.select_new)
                new_dev->setSelected(true);

            if (policy.unselect_origin)
                src_dev->setSelected(false);
        }
    }

    return res;
}

Device* SceneDevices::find(DeviceId id)
{
    auto it = devices_.find(id);
    return it == devices_.end()
        ? nullptr
        : it->second;
}

const Device* SceneDevices::find(DeviceId id) const
{
    auto it = devices_.find(id);
    return it == devices_.end()
        ? nullptr
        : it->second;
}

SharedDeviceData SceneDevices::findData(DeviceId id) const
{
    auto it = devices_.find(id);
    return it == devices_.end()
        ? SharedDeviceData {}
        : it->second->deviceData();
}

std::optional<DeviceConnectionData> SceneDevices::connectionInfo(const ConnectionId& id) const
{
    std::optional<DeviceConnectionData> res = DeviceConnectionData();
    int count = 0;

    for (auto& kv : devices_) {
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

std::optional<std::pair<QPointF, QPointF>> SceneDevices::connectionPoints(const ConnectionId& id) const
{
    auto src_it = devices_.find(id.source());
    if (src_it == devices_.end())
        return {};

    auto dest_it = devices_.find(id.destination());
    if (dest_it == devices_.end())
        return {};

    auto p0 = src_it->second->connectionPoint(id.sourceIndex(), id.sourceType(), true);
    if (!p0)
        return {};

    auto p1 = dest_it->second->connectionPoint(id.destinationIndex(), id.destinationType(), true);
    if (!p1)
        return {};

    return std::pair { *p0, *p1 };
}

std::optional<ConnectorPair> SceneDevices::connectionPair(const ConnectionId& id) const
{
    auto src_it = devices_.find(id.source());
    if (src_it == devices_.end())
        return {};

    auto dest_it = devices_.find(id.destination());
    if (dest_it == devices_.end())
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

bool SceneDevices::checkConnection(const ConnectionId& id) const
{
    if (!id.isValid())
        return false;

    auto src_it = devices_.find(id.source());
    if (src_it == devices_.end())
        return false;

    if (id.sourceIndex() >= src_it->second->deviceData()->outputs().size())
        return false;

    auto dest_it = devices_.find(id.destination());
    if (dest_it == devices_.end())
        return false;

    if (id.destinationIndex() >= dest_it->second->deviceData()->inputs().size())
        return false;

    return true;
}

bool SceneDevices::hasSelected() const
{
    for (auto& kv : devices_)
        if (kv.second->isSelected())
            return true;

    return false;
}

int SceneDevices::setSelected(const QList<DeviceId>& ids, bool value)
{
    QSignalBlocker sb(scene_);

    int count = 0;

    for (auto id : ids) {
        auto it = devices_.find(id);
        if (it != devices_.end()) {
            it->second->setSelected(value);
            count++;
        }
    }

    return count;
}

void SceneDevices::toggleSelected(const QList<DeviceId>& ids)
{
    QSignalBlocker sb(scene_);

    for (auto id : ids) {
        auto it = devices_.find(id);
        if (it != devices_.end())
            it->second->setSelected(!it->second->isSelected());
    }
}

void SceneDevices::clear()
{
    if (scene_) {
        for (auto& kv : devices_) {
            scene_->removeItem(kv.second);
            emit removed(kv.second->deviceData());
            delete kv.second;
        }
    }

    devices_.clear();
}

QList<DeviceId> SceneDevices::idList() const
{
    QList<DeviceId> res;
    res.reserve(devices_.size());

    for (auto& kv : devices_)
        res.push_back(kv.second->id());

    return res;
}

QList<SharedDeviceData> SceneDevices::dataList() const
{
    QList<SharedDeviceData> res;
    res.reserve(devices_.size());

    for (auto& kv : devices_)
        res.push_back(kv.second->deviceData());

    return res;
}

QList<DeviceId> SceneDevices::selectedIdList() const
{
    QList<DeviceId> res;

    foreachSelectedData([&res](const SharedDeviceData& data) {
        res.push_back(data->id());
    });

    return res;
}

QList<SharedDeviceData> SceneDevices::selectedDataList() const
{
    QList<SharedDeviceData> res;
    foreachSelectedData([&res](const SharedDeviceData& data) {
        res.push_back(data);
    });
    return res;
}

QRectF SceneDevices::boundingRect() const
{
    QRectF rect;
    int i = 0;
    for (auto& kv : devices_) {
        auto dev = kv.second;
        auto item_rect = dev->mapRectToScene(dev->boundingRect());

        if (rect.isNull())
            rect = item_rect;
        else
            rect |= item_rect;
    }

    return rect;
}

QRectF SceneDevices::boundingSelectRect() const
{
    QRectF rect;
    int i = 0;
    for (auto& kv : devices_) {
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

void SceneDevices::foreachDevice(const std::function<void(Device*)>& fn)
{
    if (!fn)
        return;

    for (auto& kv : devices_)
        fn(kv.second);
}

void SceneDevices::foreachSelectedDevice(const std::function<void(const Device*)>& fn)
{
    if (!fn)
        return;

    for (auto& kv : devices_) {
        if (kv.second->isSelected())
            fn(kv.second);
    }
}

void SceneDevices::foreachData(const std::function<void(const SharedDeviceData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : devices_)
        fn(kv.second->deviceData());
}

void SceneDevices::foreachSelectedData(const std::function<void(const SharedDeviceData&)>& fn) const
{
    if (!fn)
        return;

    for (auto& kv : devices_) {
        if (kv.second->isSelected())
            fn(kv.second->deviceData());
    }
}

QSet<DeviceId> SceneDevices::intersected(const QRectF& rect) const
{
    QSet<DeviceId> res;

    for (auto& kv : devices_) {
        auto dev = kv.second;
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (scene_bbox.intersects(rect))
            res.insert(kv.first);
    }

    return res;
}

QList<DeviceId> SceneDevices::intersectedList(const QRectF& rect) const
{
    QList<DeviceId> res;

    for (auto& kv : devices_) {
        auto dev = kv.second;
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (scene_bbox.intersects(rect))
            res << kv.first;
    }

    return res;
}

QJsonValue SceneDevices::toJson() const
{
    QJsonArray res;

    for (auto& kv : devices_)
        res << kv.second->toJson();

    return res;
}

bool SceneDevices::moveBy(const QHash<DeviceId, QPointF>& deltas)
{
    int count = 0;

    for (auto kv = deltas.begin(); kv != deltas.end(); ++kv) {
        auto it = devices_.find(kv.key());
        if (it != devices_.end() && !it->second->isLocked()) {
            it->second->moveBy(kv.value().x(), kv.value().y());
            count++;
        }
    }

    return count > 0;
}

bool SceneDevices::moveSelectedBy(qreal dx, qreal dy)
{
    bool res = false;

    for (auto& kv : devices_) {
        auto dev = kv.second;
        if (dev->isSelected() && !dev->isLocked()) {
            dev->moveBy(dx, dy);
            res = true;
        }
    }

    return res;
}

QDebug operator<<(QDebug debug, const ceam::SceneDevices& sc)
{
    sc.foreachData([&debug](const SharedDeviceData& data) {
        if (data)
            debug << *data << "\n";
    });
    return debug;
}
