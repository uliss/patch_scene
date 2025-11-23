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
#ifndef UNDO_COMMANDS_H
#define UNDO_COMMANDS_H

#include <QHash>
#include <QPointF>
#include <QUndoCommand>

#include "connection.h"
#include "device_common.h"

namespace ceam {

class Diagram;
class DeviceData;

class CreateDevice : public QUndoCommand {
public:
    CreateDevice(Diagram* doc, const QPointF& pos);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QPointF pos_;
    DeviceId id_ { 0 };
};

class CreateComment : public QUndoCommand {
public:
    CreateComment(Diagram* doc, const QPointF& pos);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QPointF pos_;
    DeviceId id_ { 0 };
};

class RemoveDevice : public QUndoCommand {
public:
    RemoveDevice(Diagram* doc, const SharedDeviceData& data);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    SharedDeviceData data_;
    QList<ConnectionInfo> conn_info_;
};

class RemoveSelected : public QUndoCommand {
public:
    RemoveSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> data_;
    QHash<ConnectionId, ConnectionViewData> conn_data_;
};

class DuplicateSelected : public QUndoCommand {
public:
    DuplicateSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<DeviceId> new_devs_, sel_devs_;
};

class BaseLockDevices : public QUndoCommand {
public:
    BaseLockDevices(Diagram* doc, const QList<DeviceId>& devs);

protected:
    void setLocked(bool value);

    Diagram* doc_;
    QList<DeviceId> devs_;
};

class LockDevices : public BaseLockDevices {
public:
    LockDevices(Diagram* doc, const QList<DeviceId>& devs);

    void undo() override;
    void redo() override;
};

class UnlockDevices : public BaseLockDevices {
public:
    UnlockDevices(Diagram* doc, const QList<DeviceId>& devs);

    void undo() override;
    void redo() override;
};

class BaseLockSelected : public BaseLockDevices {
public:
    BaseLockSelected(Diagram* doc, bool lockState);
};

class LockSelected : public BaseLockSelected {
public:
    LockSelected(Diagram* doc);

    void undo() override;
    void redo() override;
};

class UnlockSelected : public BaseLockSelected {
public:
    UnlockSelected(Diagram* doc);

    void undo() final;
    void redo() final;
};

class ToggleDevices : public QUndoCommand {
public:
    ToggleDevices(Diagram* doc, const QList<DeviceId>& ids);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<DeviceId> ids_;
};

class MoveSelected : public QUndoCommand {
public:
    MoveSelected(Diagram* doc, qreal dx, qreal dy);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    qreal dx_, dy_;
};

class DuplicateDevice : public QUndoCommand {
public:
    DuplicateDevice(Diagram* doc, const SharedDeviceData& data);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    SharedDeviceData src_data_;
    DeviceId new_id_;
};

class AddDeviceSelection : public QUndoCommand {
public:
    AddDeviceSelection(Diagram* doc, const QList<DeviceId>& ids);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<DeviceId> ids_;
};

class SetDeviceSelection : public QUndoCommand {
public:
    SetDeviceSelection(Diagram* doc, const QSet<DeviceId>& new_sel);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QSet<DeviceId> prev_sel_;
    QSet<DeviceId> new_sel_;
};

class ConnectDevices : public QUndoCommand {
public:
    ConnectDevices(Diagram* doc, const ConnectionId& id);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ConnectionId id_;
};

class DisconnectXlet : public QUndoCommand {
public:
    DisconnectXlet(Diagram* doc, const XletInfo& xi);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ConnectionId id_;
    ConnectionViewData view_data_;
};

class MoveByDevices : public QUndoCommand {
public:
    MoveByDevices(Diagram* doc, const QHash<DeviceId, QPointF>& deltas);

    void undo() final;
    void redo() final;

private:
    static QHash<DeviceId, QPointF> negate(const QHash<DeviceId, QPointF>& map);

private:
    Diagram* doc_;
    QHash<DeviceId, QPointF> deltas_;
};

class CopySelected : public QUndoCommand {
public:
    CopySelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> old_clip_buf_;
};

class CutSelected : public QUndoCommand {
public:
    CutSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> data_, prev_clip_buf_;
};

class PasteFromClipBuffer : public QUndoCommand {
public:
    PasteFromClipBuffer(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> data_;
    QList<DeviceId> added_;
};

class UpdateDeviceData : public QUndoCommand {
public:
    UpdateDeviceData(Diagram* doc, const SharedDeviceData& data);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    SharedDeviceData old_data_, new_data_;
};

class ReconnectDevice : public QUndoCommand {
public:
    ReconnectDevice(Diagram* doc, const ConnectionInfo& old_conn, const ConnectionInfo& new_conn);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ConnectionInfo old_conn_, new_conn_;
};

class MirrorSelected : public QUndoCommand {
public:
    MirrorSelected(Diagram* doc, ImageMirrorType type);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ImageMirrorType type_;
};

class MirrorDevice : public QUndoCommand {
public:
    MirrorDevice(Diagram* doc, DeviceId id, ImageMirrorType type);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    DeviceId id_;
    ImageMirrorType type_;
};

class ZoomSelected : public QUndoCommand {
public:
    ZoomSelected(Diagram* doc, qreal k);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    qreal k_;
};

class MoveLower : public QUndoCommand {
public:
    MoveLower(Diagram* doc, DeviceId id);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    DeviceId id_;
    qreal old_z_;
};

class MoveUpper : public QUndoCommand {
public:
    MoveUpper(Diagram* doc, DeviceId id);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    DeviceId id_;
    qreal old_z_;
};

} // namespace ceam

#endif // UNDO_COMMANDS_H
