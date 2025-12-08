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

#include "device_common.h"

namespace ceam {

class Diagram;
class DiagramScene;
class ItemData;

class CreateDevice : public QUndoCommand {
public:
    CreateDevice(DiagramScene* doc, const QPointF& pos);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QPointF pos_;
    SceneItemId id_ { 0 };
};

class CreateComment : public QUndoCommand {
public:
    CreateComment(DiagramScene* doc, const QPointF& pos, const QString& txt = {});

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QPointF pos_;
    SceneItemId id_ { 0 };
    QString txt_;
};

class RemoveItem : public QUndoCommand {
public:
    RemoveItem(DiagramScene* doc, const SharedItemData& data);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SharedItemData data_;
    QList<ConnectionInfo> conn_info_;
};

class RemoveSelected : public QUndoCommand {
public:
    explicit RemoveSelected(DiagramScene* doc);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QList<SharedItemData> data_;
    QHash<ConnectionId, ConnectionViewData> conn_data_;
};

class DuplicateSelected : public QUndoCommand {
public:
    explicit DuplicateSelected(DiagramScene* doc);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QList<SceneItemId> new_devs_, sel_devs_;
};

class BaseLockItems : public QUndoCommand {
public:
    BaseLockItems(DiagramScene* doc, const QList<SceneItemId>& devs);

protected:
    void setLocked(bool value);

    DiagramScene* doc_;
    QList<SceneItemId> devs_;
};

class LockItems : public BaseLockItems {
public:
    LockItems(DiagramScene* doc, const QList<SceneItemId>& devs);

    void undo() override;
    void redo() override;
};

class UnlockItems : public BaseLockItems {
public:
    UnlockItems(DiagramScene* doc, const QList<SceneItemId>& devs);

    void undo() override;
    void redo() override;
};

class BaseLockSelected : public BaseLockItems {
public:
    BaseLockSelected(DiagramScene* doc, bool lockState);
};

class LockSelected : public BaseLockSelected {
public:
    explicit LockSelected(DiagramScene* doc);

    void undo() override;
    void redo() override;
};

class UnlockSelected : public BaseLockSelected {
public:
    explicit UnlockSelected(DiagramScene* doc);

    void undo() final;
    void redo() final;
};

class ToggleSelected : public QUndoCommand {
public:
    ToggleSelected(DiagramScene* doc, const QList<SceneItemId>& ids);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QList<SceneItemId> ids_;
};

class MoveSelected : public QUndoCommand {
public:
    MoveSelected(DiagramScene* doc, qreal dx, qreal dy);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    qreal dx_, dy_;
};

class DuplicateItem : public QUndoCommand {
public:
    DuplicateItem(DiagramScene* doc, const SharedItemData& data);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SharedItemData src_data_;
    SceneItemId new_id_;
};

class AddToSelected : public QUndoCommand {
public:
    AddToSelected(DiagramScene* doc, const QList<SceneItemId>& ids);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QList<SceneItemId> ids_;
};

class SetSelected : public QUndoCommand {
public:
    SetSelected(DiagramScene* doc, const QSet<SceneItemId>& new_sel);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    QSet<SceneItemId> prev_sel_;
    QSet<SceneItemId> new_sel_;
};

class ConnectDevices : public QUndoCommand {
public:
    ConnectDevices(DiagramScene* doc, const ConnectionId& id);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    ConnectionId id_;
};

class DisconnectXlet : public QUndoCommand {
public:
    DisconnectXlet(DiagramScene* doc, const XletInfo& xi);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    ConnectionId id_;
    ConnectionViewData view_data_;
};

class MoveByItems : public QUndoCommand {
public:
    MoveByItems(DiagramScene* doc, const QHash<SceneItemId, QPointF>& deltas);

    void undo() final;
    void redo() final;

private:
    static QHash<SceneItemId, QPointF> negate(const QHash<SceneItemId, QPointF>& map);

private:
    DiagramScene* doc_;
    QHash<SceneItemId, QPointF> deltas_;
};

class CopySelected : public QUndoCommand {
public:
    explicit CopySelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedItemData> old_clip_buf_;
};

class CutSelected : public QUndoCommand {
public:
    explicit CutSelected(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedItemData> data_, prev_clip_buf_;
};

class PasteFromClipBuffer : public QUndoCommand {
public:
    explicit PasteFromClipBuffer(Diagram* doc);

    void undo() final;
    void redo() final;

private:
    Diagram* doc_;
    QList<SharedItemData> data_;
    QList<SceneItemId> added_;
};

class UpdateDeviceData : public QUndoCommand {
public:
    UpdateDeviceData(DiagramScene* doc, const SharedItemData& data);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SharedItemData old_data_, new_data_;
};

class ReconnectDevice : public QUndoCommand {
public:
    ReconnectDevice(DiagramScene* doc, const ConnectionInfo& old_conn, const ConnectionInfo& new_conn);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    ConnectionInfo old_conn_, new_conn_;
};

class MirrorSelected : public QUndoCommand {
public:
    MirrorSelected(DiagramScene* doc, ImageMirrorType type);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    ImageMirrorType type_;
};

class MirrorDevice : public QUndoCommand {
public:
    MirrorDevice(DiagramScene* doc, SceneItemId id, ImageMirrorType type);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SceneItemId id_;
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
    MoveLower(DiagramScene* doc, SceneItemId id);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SceneItemId id_;
    qreal old_z_;
};

class MoveUpper : public QUndoCommand {
public:
    MoveUpper(DiagramScene* doc, SceneItemId id);

    void undo() final;
    void redo() final;

private:
    DiagramScene* doc_;
    SceneItemId id_;
    qreal old_z_;
};

} // namespace ceam

#endif // UNDO_COMMANDS_H
