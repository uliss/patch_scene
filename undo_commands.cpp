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
        doc_->removeDevice(id_);
}

void CreateDevice::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->addDevice(Device::defaultDeviceData());
    if (dev) {
        dev->setPos(pos_);
        id_ = dev->id();
    }
}

AddDeviceSelection::AddDeviceSelection(Diagram* doc, const QList<DeviceId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void AddDeviceSelection::undo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->devices().setSelected(ids_, false);
    }
}

void AddDeviceSelection::redo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->devices().setSelected(ids_, true);
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

RemoveDevice::RemoveDevice(Diagram* doc, const SharedDeviceData& data)
    : doc_(doc)
    , data_(data)
{
    if (doc_ && data_)
        conn_info_ = doc_->connections()->findConnectionsData(data->id());
}

void RemoveDevice::undo()
{
    if (doc_ && data_) {
        doc_->addDevice(data_);

        for (auto& conn : conn_info_)
            doc_->connectDevices(conn.first, conn.second);
    }
}

void RemoveDevice::redo()
{
    if (doc_ && data_)
        doc_->removeDevice(data_->id());
}

RemoveSelected::RemoveSelected(Diagram* doc)
    : doc_(doc)
{
    if (!doc_)
        return;

    data_ = doc_->devices().selectedDataList();
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
        auto dev = doc_->addDevice(x);
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
        doc_->removeDevice(data->id());

    // only connection is selected
    if (data_.isEmpty()) {
        for (auto it = conn_data_.begin(); it != conn_data_.end(); ++it)
            doc_->disconnectDevices(it.key());
    }
}

DuplicateSelected::DuplicateSelected(Diagram* doc)
    : doc_(doc)
{
}

void DuplicateSelected::undo()
{
    if (!doc_)
        return;

    {
        // remove duplicated devices
        DiagramUpdatesBlocker ub(doc_);
        for (auto id : new_devs_)
            doc_->removeDevice(id);

        // restore selection
        for (auto id : sel_devs_) {
            auto dev = doc_->devices().find(id);
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
        DiagramUpdatesBlocker ub(doc_);
        doc_->devices().foreachSelectedDevice([this](Device* dev) {
            auto new_dev = doc_->addDevice(dev->deviceData());
            if (new_dev) {
                switch (new_dev->deviceData()->category()) {
                case ItemCategory::Furniture:
                    new_dev->moveBy(50, 0);
                    break;
                default:
                    new_dev->moveBy(20, 20);
                    break;
                }

                new_dev->setSelected(true);
                new_devs_.push_back(new_dev->id());
                sel_devs_.push_back(dev->id());
                dev->setSelected(false);
            }
        });
    }

    emit doc_->sceneFullUpdate();
}

DuplicateDevice::DuplicateDevice(Diagram* doc, const SharedDeviceData& data)
    : doc_(doc)
    , src_data_(data)
    , new_id_(DEV_NULL_ID)
{
}

void DuplicateDevice::undo()
{
    if (!doc_ || new_id_ == DEV_NULL_ID)
        return;

    doc_->removeDevice(new_id_);
    new_id_ = DEV_NULL_ID;
}

void DuplicateDevice::redo()
{
    if (!doc_)
        return;

    auto dev = doc_->addDevice(src_data_);
    dev->moveBy(20, 20);
    new_id_ = dev->id();
}

ToggleDevices::ToggleDevices(Diagram* doc, const QList<DeviceId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void ToggleDevices::undo()
{
    if (doc_)
        doc_->devices().toggleSelected(ids_);
}

void ToggleDevices::redo()
{
    if (doc_)
        doc_->devices().toggleSelected(ids_);
}

SetDeviceSelection::SetDeviceSelection(Diagram* doc, const QSet<DeviceId>& new_sel)
    : doc_(doc)
    , new_sel_(new_sel)
{
    if (doc_) {
        doc_->devices().foreachSelectedData([this](const SharedDeviceData& data) {
            prev_sel_.insert(data->id());
        });
    }
}

void SetDeviceSelection::undo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->devices().foreachDevice([this](Device* dev) {
            dev->setSelected(prev_sel_.contains(dev->id()));
        });
    }
}

void SetDeviceSelection::redo()
{
    if (doc_) {
        DiagramUpdatesBlocker ub(doc_);
        doc_->devices().foreachDevice([this](Device* dev) {
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

MoveByDevices::MoveByDevices(Diagram* doc, const QHash<DeviceId, QPointF>& deltas)
    : doc_(doc)
    , deltas_(deltas)
{
}

void MoveByDevices::undo()
{
    if (!doc_)
        return;

    doc_->moveItemsBy(negate(deltas_));
}

void MoveByDevices::redo()
{
    if (!doc_)
        return;

    doc_->moveItemsBy(deltas_);
}

QHash<DeviceId, QPointF> MoveByDevices::negate(const QHash<DeviceId, QPointF>& map)
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
        data_ = doc_->devices().selectedDataList();
}

void CutSelected::undo()
{
    if (!doc_ || data_.empty())
        return;

    // restore clip buffer
    doc_->setClipBuffer(prev_clip_buf_);

    // restore selected
    for (const auto& data : data_) {
        auto dev = doc_->addDevice(data);
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
        doc_->removeDevice(data->id());
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
        doc_->removeDevice(id);
}

void PasteFromClipBuffer::redo()
{
    if (!doc_)
        return;

    added_.clear();
    for (const auto& data : data_) {

        auto dev = doc_->addDevice(data);
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

    auto data = doc_->devices().selectedDataList();
    if (!data.isEmpty())
        doc_->setClipBuffer(data);
}

UpdateDeviceData::UpdateDeviceData(Diagram* doc, const SharedDeviceData& data)
    : doc_(doc)
    , new_data_(data)
{
    if (doc_ && data)
        old_data_ = doc_->devices().findData(data->id());
}

void UpdateDeviceData::undo()
{
    if (doc_)
        doc_->setDeviceData(old_data_);
}

void UpdateDeviceData::redo()
{
    if (doc_)
        doc_->setDeviceData(new_data_);
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
