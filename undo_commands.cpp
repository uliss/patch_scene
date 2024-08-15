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

constexpr int MoveDeviceId = 1000;

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

    auto dev = new Device();
    dev->setPos(pos_);
    id_ = dev->id();
    doc_->addDevice(dev);
}

AddDeviceSelection::AddDeviceSelection(Diagram* doc, const QList<DeviceId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void AddDeviceSelection::undo()
{
    doc_->selectDevices(ids_, false);
}

void AddDeviceSelection::redo()
{
    doc_->selectDevices(ids_, true);
}

ConnectDevices::ConnectDevices(Diagram* doc, const ConnectionData& conn)
    : doc_(doc)
    , conn_(conn)
{
}

void ConnectDevices::undo()
{
    if (doc_)
        doc_->disconnectDevices(conn_);
}

void ConnectDevices::redo()
{
    if (doc_)
        doc_->connectDevices(conn_);
}

DisconnectXlet::DisconnectXlet(Diagram* doc, const XletInfo& xi)
    : doc_(doc)
    , conn_(0, 0, 0, 0)
{
    auto conn = doc->findConnectionByXlet(xi);
    if (conn)
        conn_ = conn->connectionData();
}

void DisconnectXlet::undo()
{
    if (doc_ && conn_.isValid())
        doc_->connectDevices(conn_);
}

void DisconnectXlet::redo()
{
    if (doc_ && conn_.isValid())
        doc_->disconnectDevices(conn_);
}

RemoveDevice::RemoveDevice(Diagram* doc, const SharedDeviceData& data)
    : doc_(doc)
    , data_(data)
{
    if (doc_)
        conn_ = doc_->findDeviceConnections(data->id());
}

void RemoveDevice::undo()
{
    if (doc_) {
        doc_->addDevice(new Device(data_));

        for (auto& conn : conn_)
            doc_->connectDevices(conn);
    }
}

void RemoveDevice::redo()
{
    if (doc_)
        doc_->removeDevice(data_->id());
}

RemoveSelected::RemoveSelected(Diagram* doc)
    : doc_(doc)
{
    if (!doc_)
        return;

    // store device data
    for (auto dev : doc_->selectedDevices())
        data_.push_back(dev->deviceData());

    conn_ = doc_->findSelectedConnections();
}

void RemoveSelected::undo()
{
    if (!doc_)
        return;

    for (const auto& x : data_) {
        auto dev = new Device(x);
        doc_->addDevice(dev);
        dev->setSelected(true);
    }

    for (auto& c : std::as_const(conn_))
        doc_->connectDevices(c);
}

void RemoveSelected::redo()
{
    if (!doc_)
        return;

    for (const auto& data : data_)
        doc_->removeDevice(data->id());
}

DuplicateSelected::DuplicateSelected(Diagram* doc)
    : doc_(doc)
{
}

void DuplicateSelected::undo()
{
    if (!doc_)
        return;

    for (auto id : data_)
        doc_->removeDevice(id);
}

void DuplicateSelected::redo()
{
    if (!doc_)
        return;

    for (const auto& dev : doc_->selectedDevices()) {
        auto clone = new Device(dev->deviceData());
        clone->moveBy(20, 20);
        // clone->incrementName();
        doc_->addDevice(clone);
        data_.push_back(clone->id());
    }
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

    auto clone = new Device(src_data_);
    clone->moveBy(20, 20);
    doc_->addDevice(clone);
    new_id_ = clone->id();
}

ToggleDevices::ToggleDevices(Diagram* doc, const QList<DeviceId>& ids)
    : doc_(doc)
    , ids_(ids)
{
}

void ToggleDevices::undo()
{
    if (doc_)
        doc_->toggleDevices(ids_);
}

void ToggleDevices::redo()
{
    if (doc_)
        doc_->toggleDevices(ids_);
}

SetDeviceSelection::SetDeviceSelection(Diagram* doc, const QSet<DeviceId>& new_sel)
    : doc_(doc)
    , new_sel_(new_sel)
{
    if (doc_) {
        for (auto dev : doc_->selectedDevices())
            prev_sel_.insert(dev->id());
    }
}

void SetDeviceSelection::undo()
{
    if (doc_) {
        for (auto dev : doc_->devices())
            dev->setSelected(prev_sel_.contains(dev->id()));
    }
}

void SetDeviceSelection::redo()
{
    for (auto dev : doc_->devices())
        dev->setSelected(new_sel_.contains(dev->id()));
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

MoveByDevices::MoveByDevices(Diagram* doc, const QMap<DeviceId, QPointF>& deltas)
    : doc_(doc)
    , deltas_(deltas)
{
}

void MoveByDevices::undo()
{
    if (!doc_)
        return;

    for (auto dev : doc_->devices()) {
        auto it = deltas_.constFind(dev->id());
        if (it != deltas_.cend())
            dev->moveBy(-it->x(), -it->y());
    }

    doc_->updateConnectionsPos();
}

void MoveByDevices::redo()
{
    if (!doc_)
        return;

    for (auto dev : doc_->devices()) {
        auto it = deltas_.constFind(dev->id());
        if (it != deltas_.cend())
            dev->moveBy(it->x(), it->y());
    }

    doc_->updateConnectionsPos();
}

CutSelected::CutSelected(Diagram* doc)
    : doc_(doc)
{
    if (doc_) {
        for (auto dev : doc_->selectedDevices())
            data_.push_back(dev->deviceData());
    }
}

void CutSelected::undo()
{
    if (!doc_ || data_.empty())
        return;

    // restore clip buffer
    doc_->setClipBuffer(prev_clip_buf_);

    // restore selected
    for (const auto& data : data_) {
        auto dev = new Device(data);
        dev->setSelected(true);
        doc_->addDevice(dev);
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
        auto dev = new Device(data);
        // dev->setSelected(true);
        added_.push_back(dev->id());
        doc_->addDevice(dev);
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

    QList<SharedDeviceData> data;
    for (auto dev : doc_->selectedDevices())
        data.push_back(dev->deviceData());

    if (!data.isEmpty())
        doc_->setClipBuffer(data);
}

UpdateDeviceData::UpdateDeviceData(Diagram* doc, const SharedDeviceData& data)
    : doc_(doc)
    , new_data_(data)
{
    if (doc_) {
        auto dev = doc_->findDeviceById(data->id());
        if (dev)
            old_data_ = dev->deviceData();
    }
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
