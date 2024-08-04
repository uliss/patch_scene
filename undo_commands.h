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

#include <QPointF>
#include <QUndoCommand>

#include "connection.h"
#include "device.h"

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

class RemoveDevice : public QUndoCommand {
public:
    RemoveDevice(Diagram* doc, const SharedDeviceData& data);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    SharedDeviceData data_;
};

class RemoveSelected : public QUndoCommand {
public:
    RemoveSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> data_;
};

class DuplicateSelected : public QUndoCommand {
public:
    DuplicateSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedDeviceData> data_;
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
    ConnectDevices(Diagram* doc, const ConnectionData& conn);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ConnectionData conn_;
};

class DisconnectXlet : public QUndoCommand {
public:
    DisconnectXlet(Diagram* doc, const XletInfo& xi);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    ConnectionData conn_;
};

class MoveByDevices : public QUndoCommand {
public:
    MoveByDevices(Diagram* doc, const QMap<DeviceId, QPointF>& deltas);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QMap<DeviceId, QPointF> deltas_;
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

#endif // UNDO_COMMANDS_H
