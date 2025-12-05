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
#include "undo_commands.h"
#include "device_item.h"
#include "diagram.h"
#include "diagram_updates_blocker.h"
#include "logging.hpp"

namespace {
constexpr int MoveDeviceId = 1000;
}

using namespace ceam;

CreateDevice::CreateDevice(Diagram* doc, const QPointF& pos)
    : doc_(doc)
    , pos_(pos)
{
}

void CreateDevice::undo()
{
    if (doc_)
        doc_->removeItem(id_);
}

void CreateDevice::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->addItem(SceneItem::defaultDeviceData());
    if (dev) {
        dev->setPos(pos_);
        id_ = dev->id();
    }
}

AddToSelected::AddToSelected(Diagram* doc, const QList<SceneItemId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void AddToSelected::undo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->itemScene().setSelected(ids_, false);
    }
}

void AddToSelected::redo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->itemScene().setSelected(ids_, true);
    }
}

ConnectDevices::ConnectDevices(Diagram* doc, const ConnectionId& id)
    : doc_(doc)
    , id_(id)
{
}

void ConnectDevices::undo()
{
    if (doc_)
        doc_->disconnectDevices(id_);
}

void ConnectDevices::redo()
{
    if (doc_)
        doc_->connectDevices(id_, std::nullopt);
}

DisconnectXlet::DisconnectXlet(Diagram* doc, const XletInfo& xi)
    : doc_(doc)
    , id_(0, 0, 0, 0)
{
    auto conn = doc->connections()->findByXlet(xi);
    if (conn) {
        id_ = conn->connectionId();
        view_data_ = conn->viewData();
    }
}

void DisconnectXlet::undo()
{
    if (doc_ && id_.isValid())
        doc_->connectDevices(id_, view_data_);
    else
        WARN() << "can't connect:" << id_;
}

void DisconnectXlet::redo()
{
    if (doc_ && id_.isValid())
        doc_->disconnectDevices(id_);
    else
        WARN() << "can't disconnect:" << id_;
}

RemoveItem::RemoveItem(Diagram* doc, const SharedItemData& data)
    : doc_(doc)
    , data_(data)
{
    if (doc_ && data_)
        conn_info_ = doc_->connections()->findConnectionsData(data->id());
}

void RemoveItem::undo()
{
    if (doc_ && data_) {
        doc_->addItem(data_);

        // TODO(uliss): device/comment

        for (auto& conn : conn_info_)
            doc_->connectDevices(conn.first, conn.second);
    }
}

void RemoveItem::redo()
{
    if (doc_ && data_)
        doc_->removeItem(data_->id());
}

RemoveSelected::RemoveSelected(Diagram* doc)
    : doc_(doc)
{
    if (!doc_)
        return;

    data_ = doc_->itemScene().selectedDataList();
    conn_data_ = doc_->findSelectedConnections();

    // append selected connections
    auto conn_selected = doc_->connections()->selectedList();
    for (auto it = conn_selected.begin(); it != conn_selected.end(); ++it)
        conn_data_[it->first] = it->second;
}

void RemoveSelected::undo()
{
    if (!doc_)
        return;

    for (const auto& x : data_) {
        auto dev = doc_->addItem(x);
        if (dev)
            dev->setSelected(true);
    }

    for (auto it = conn_data_.begin(); it != conn_data_.end(); ++it)
        doc_->connectDevices(it.key(), it.value());
}

void RemoveSelected::redo()
{
    if (!doc_)
        return;

    for (const auto& data : data_)
        doc_->removeItem(data->id());

    // only connection is selected
    if (data_.isEmpty()) {
        for (auto it = conn_data_.begin(); it != conn_data_.end(); ++it)
            doc_->disconnectDevices(it.key());
    }
}

DuplicateSelected::DuplicateSelected(Diagram* doc)
    : doc_(doc)
{
    sel_devs_ = doc_->itemScene().selectedIdList();
}

void DuplicateSelected::undo()
{
    if (!doc_)
        return;

    {
        // remove duplicated devices
        DiagramUpdatesBlocker ub(doc_);
        for (auto id : new_devs_)
            doc_->removeItem(id);

        // restore selection
        for (auto id : sel_devs_) {
            auto dev = doc_->itemScene().find(id);
            if (dev)
                dev->setSelected(true);
        }
    }

    emit doc_->sceneFullUpdate();
}

void DuplicateSelected::redo()
{
    if (!doc_)
        return;

    {
        Diagram::DuplicatePolicy policy;
        policy.select_new = true;
        policy.unselect_origin = true;

        DiagramUpdatesBlocker ub(doc_);
        new_devs_ = doc_->duplicateSelected(policy);
    }

    emit doc_->sceneFullUpdate();
}

DuplicateItem::DuplicateItem(Diagram* doc, const SharedItemData& data)
    : doc_(doc)
    , src_data_(data)
    , new_id_(SCENE_ITEM_NULL_ID)
{
}

void DuplicateItem::undo()
{
    if (!doc_ || new_id_ == SCENE_ITEM_NULL_ID)
        return;

    doc_->removeItem(new_id_);
    new_id_ = SCENE_ITEM_NULL_ID;
}

void DuplicateItem::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->addItem(src_data_);
    dev->moveBy(20, 20);
    new_id_ = dev->id();
}

ToggleSelected::ToggleSelected(Diagram* doc, const QList<SceneItemId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void ToggleSelected::undo()
{
    if (doc_)
        doc_->itemScene().toggleSelected(ids_);
}

void ToggleSelected::redo()
{
    if (doc_)
        doc_->itemScene().toggleSelected(ids_);
}

SetSelected::SetSelected(Diagram* doc, const QSet<SceneItemId>& new_sel)
    : doc_(doc)
    , new_sel_(new_sel)
{
    if (doc_) {
        doc_->itemScene().foreachSelectedData([this](const SharedItemData& data) {
            prev_sel_.insert(data->id());
        });
    }
}

void SetSelected::undo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->itemScene().foreachItem([this](SceneItem* dev) {
            dev->setSelected(prev_sel_.contains(dev->id()));
        });
    }
}

void SetSelected::redo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->itemScene().foreachItem([this](SceneItem* dev) {
            dev->setSelected(new_sel_.contains(dev->id()));
        });
    }
}

MoveSelected::MoveSelected(Diagram* doc, qreal dx, qreal dy)
    : doc_(doc)
    , dx_(dx)
    , dy_(dy)
{
}

void MoveSelected::undo()
{
    if (!doc_)
        return;

    doc_->moveSelectedItemsBy(-dx_, -dy_);
}

void MoveSelected::redo()
{
    if (!doc_)
        return;

    doc_->moveSelectedItemsBy(dx_, dy_);
}

MoveByItems::MoveByItems(Diagram* doc, const QHash<SceneItemId, QPointF>& deltas)
    : doc_(doc)
    , deltas_(deltas)
{
}

void MoveByItems::undo()
{
    if (!doc_)
        return;

    doc_->moveItemsBy(negate(deltas_));
}

void MoveByItems::redo()
{
    if (!doc_)
        return;

    doc_->moveItemsBy(deltas_);
}

QHash<SceneItemId, QPointF> MoveByItems::negate(const QHash<SceneItemId, QPointF>& map)
{
    auto res = map;

    for (auto& pt : res) {
        pt.rx() *= -1;
        pt.ry() *= -1;
    }

    return res;
}

CutSelected::CutSelected(Diagram* doc)
    : doc_(doc)
{
    if (doc_)
        data_ = doc_->itemScene().selectedDataList();
}

void CutSelected::undo()
{
    if (!doc_ || data_.empty())
        return;

    // restore clip buffer
    doc_->setClipBuffer(prev_clip_buf_);

    // restore selected
    for (const auto& data : data_) {
        auto dev = doc_->addItem(data);
        if (dev)
            dev->setSelected(true);
    }
}

void CutSelected::redo()
{
    if (!doc_ || data_.empty())
        return;

    prev_clip_buf_ = doc_->clipBuffer(); // save old clip buffer
    doc_->setClipBuffer(data_);

    // remove selected
    for (const auto& data : data_)
        doc_->removeItem(data->id());
}

PasteFromClipBuffer::PasteFromClipBuffer(Diagram* doc)
    : doc_(doc)
{
    if (doc_)
        data_ = doc_->clipBuffer();
}

void PasteFromClipBuffer::undo()
{
    if (!doc_)
        return;

    for (auto id : added_)
        doc_->removeItem(id);
}

void PasteFromClipBuffer::redo()
{
    if (!doc_)
        return;

    added_.clear();
    for (const auto& data : data_) {

        auto dev = doc_->addItem(data);
        if (dev) {
            dev->randomizePos(50);
            added_.push_back(dev->id());
        }
    }
}

CopySelected::CopySelected(Diagram* doc)
    : doc_(doc)
{
    if (doc_)
        old_clip_buf_ = doc_->clipBuffer();
}

void CopySelected::undo()
{
    if (!doc_)
        return;

    doc_->setClipBuffer(old_clip_buf_);
}

void CopySelected::redo()
{
    if (!doc_)
        return;

    auto data = doc_->itemScene().selectedDataList();
    if (!data.isEmpty())
        doc_->setClipBuffer(data);
}

UpdateDeviceData::UpdateDeviceData(Diagram* doc, const SharedItemData& data)
    : doc_(doc)
    , new_data_(data)
{
    if (doc_ && data)
        old_data_ = doc_->itemScene().findData(data->id());
}

void UpdateDeviceData::undo()
{
    if (doc_)
        doc_->setItemData(old_data_);
}

void UpdateDeviceData::redo()
{
    if (doc_)
        doc_->setItemData(new_data_);
}

ReconnectDevice::ReconnectDevice(Diagram* doc, const ConnectionInfo& old_conn, const ConnectionInfo& new_conn)
    : doc_(doc)
    , old_conn_(old_conn)
    , new_conn_(new_conn)
{
}

void ReconnectDevice::undo()
{
    if (doc_) {
        doc_->disconnectDevices(new_conn_.first);
        doc_->connectDevices(old_conn_.first, old_conn_.second);
    }
}

void ReconnectDevice::redo()
{
    if (doc_) {
        doc_->disconnectDevices(old_conn_.first);
        doc_->connectDevices(new_conn_.first, new_conn_.second);
    }
}

LockSelected::LockSelected(Diagram* doc)
    : BaseLockSelected(doc, false)
{
}

void LockSelected::undo()
{
    setLocked(false);
}

void LockSelected::redo()
{
    setLocked(true);
}

UnlockSelected::UnlockSelected(Diagram* doc)
    : BaseLockSelected(doc, true)
{
}

void UnlockSelected::undo()
{
    setLocked(true);
}

void UnlockSelected::redo()
{
    setLocked(false);
}

BaseLockSelected::BaseLockSelected(Diagram* doc, bool lockState)
    : BaseLockItems(doc, {})
{
    for (const auto& data : doc->itemScene().selectedDataList()) {
        if (data->isLocked() == lockState)
            devs_.push_back(data->id());
    }
}

void BaseLockItems::setLocked(bool value)
{
    if (!doc_)
        return;

    for (auto id : devs_) {
        auto dev = doc_->itemScene().find(id);
        if (dev)
            dev->setLocked(value);
    }
}

BaseLockItems::BaseLockItems(Diagram* doc, const QList<SceneItemId>& devs)
    : doc_(doc)
    , devs_(devs)
{
}

LockItems::LockItems(Diagram* doc, const QList<SceneItemId>& devs)
    : BaseLockItems(doc, devs)
{
}

void LockItems::undo()
{
    setLocked(false);
}

void LockItems::redo()
{
    setLocked(true);
}

UnlockItems::UnlockItems(Diagram* doc, const QList<SceneItemId>& devs)
    : BaseLockItems(doc, devs)
{
}

void UnlockItems::undo()
{
    setLocked(true);
}

void UnlockItems::redo()
{
    setLocked(false);
}

MirrorSelected::MirrorSelected(Diagram* doc, ImageMirrorType type)
    : doc_(doc)
    , type_(type)
{
}

void MirrorSelected::undo()
{
    MirrorSelected::redo();
}

void MirrorSelected::redo()
{
    if (!doc_)
        return;

    doc_->itemScene().foreachItem([this](SceneItem* item) {
        if (item->isSelected()) {
            auto dev = dynamic_cast<DeviceItem*>(item);
            if (dev)
                dev->mirrorImage(type_);
        }
    });
}

MirrorDevice::MirrorDevice(Diagram* doc, SceneItemId id, ImageMirrorType type)
    : doc_(doc)
    , id_(id)
    , type_(type)
{
}

void MirrorDevice::undo()
{
    MirrorDevice::redo();
}

void MirrorDevice::redo()
{
    if (!doc_)
        return;

    auto dev = dynamic_cast<DeviceItem*>(doc_->itemScene().find(id_));
    if (!dev)
        return;

    dev->mirrorImage(type_);
}

ZoomSelected::ZoomSelected(Diagram* doc, qreal k)
    : doc_(doc)
    , k_(k)
{
}

void ZoomSelected::undo()
{
    if (!doc_)
        return;

    doc_->itemScene().foreachItem([this](SceneItem* item) {
        if (item->isSelected()) {
            auto dev = dynamic_cast<DeviceItem*>(item);
            if (dev)
                dev->zoomImage(1 / k_);
        }
    });
}

void ZoomSelected::redo()
{
    if (!doc_)
        return;

    doc_->itemScene().foreachItem([this](SceneItem* item) {
        if (item->isSelected()) {
            auto dev = dynamic_cast<DeviceItem*>(item);
            if (dev)
                dev->zoomImage(k_);
        }
    });
}

CreateComment::CreateComment(Diagram* doc, const QPointF& pos, const QString& txt)
    : doc_(doc)
    , pos_(pos)
    , txt_(txt)
{
}

void CreateComment::undo()
{
    if (doc_)
        doc_->removeItem(id_);
}

void CreateComment::redo()
{
    if (!doc_)
        return;

    auto comment = doc_->addItem(ItemData::makeComment(txt_));
    if (comment) {
        comment->setPos(pos_);
        id_ = comment->id();
    }
}

MoveLower::MoveLower(Diagram* doc, SceneItemId id)
    : doc_(doc)
    , id_(id)
    , old_z_(0)
{
}

void MoveLower::undo()
{
    if (!doc_)
        return;

    auto dev = doc_->itemScene().find(id_);
    if (!dev)
        return;

    dev->setZValue(old_z_);
}

void MoveLower::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->itemScene().find(id_);
    if (!dev) {
        WARN() << "device not found: " << id_;
        return;
    }

    old_z_ = dev->zValue();

    const SceneItem* lower_item = nullptr;
    for (auto it : dev->collidingItems()) {
        auto x = qgraphicsitem_cast<const SceneItem*>(it);
        if (x && x->zValue() <= old_z_)
            lower_item = x;
    }

    if (!lower_item) {
        WARN() << "LOWER NOT FOUND";
        return;
    }

    // TODO(uliss): check this for big reals!
    auto z = lower_item->zValue() - 1;
    dev->setZValue(z);
}

MoveUpper::MoveUpper(Diagram* doc, SceneItemId id)
    : doc_(doc)
    , id_(id)
    , old_z_(0)
{
}

void MoveUpper::undo()
{
    if (!doc_)
        return;

    auto dev = doc_->itemScene().find(id_);
    if (!dev)
        return;

    dev->setZValue(old_z_);
}

void MoveUpper::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->itemScene().find(id_);
    if (!dev) {
        WARN() << "device not found: " << id_;
        return;
    }

    old_z_ = dev->zValue();

    const SceneItem* upper_item = nullptr;
    for (auto it : dev->collidingItems()) {
        auto x = qgraphicsitem_cast<const SceneItem*>(it);
        if (x && x->zValue() >= old_z_)
            upper_item = x;
    }

    if (!upper_item) {
        WARN() << "UPPER NOT FOUND";
        return;
    }

    // TODO(uliss): check this for big reals!
    auto z = upper_item->zValue() + 0.5;
    dev->setZValue(z);
}
