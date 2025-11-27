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
#ifndef SCENE_ITEM_H
#define SCENE_ITEM_H

#include "device_common.h"
#include "user_item_types.h"

#include <QAction>
#include <QGraphicsObject>

namespace ceam {

class SceneItem : public QGraphicsObject {
    Q_OBJECT
public:
    enum { Type = UserItemTypeDevice };
    int type() const override { return Type; }

public:
    SceneItem();
    explicit SceneItem(const SharedItemData& data);
    ~SceneItem();

    /**
     * @return item id
     */
    SceneItemId id() const { return data_->id(); }

    SharedItemData deviceData() const;
    virtual bool setDeviceData(const SharedItemData& data);

    virtual std::optional<QPointF> connectionPoint(XletIndex i, XletType type, bool map) const;

    /**
     * move item into random neighborhood within the specified delta
     */
    void randomizePos(qint64 delta);

    /**
     * export item state/data to json
     */
    QJsonObject toJson() const;

    bool isLocked() const { return data_ && data_->isLocked(); }
    void setLocked(bool value);

    /**
     * fill given menu with device actions
     * @note only for single context menu
     */
    virtual void createContextMenu(QMenu& menu);

    static SharedItemData defaultDeviceData();
    static SharedItemData dataFromJson(const QJsonValue& j);

signals:
    void addToFavorites(SharedItemData data);
    void alignHorizontal();
    void alignVertical();
    void distributeHorizontal();
    void distributeVertical();
    void duplicateDevice(SharedItemData data);
    void moveLower(const SharedItemData& data);
    void moveUpper(const SharedItemData& data);
    void placeInColumn();
    void placeInRow();
    void removeDevice(SharedItemData data);
    void updateDevice(SharedItemData data);

    void lockSelected();
    void unlockSelected();
    void lock(SceneItemId id);
    void unlock(SceneItemId id);

    void mirrorSelected();
    void mirror(SceneItemId id);

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) final;

protected:
    // context menu actions
    void addDuplicateAct(QMenu& menu);
    void addLockAction(QMenu& menu);
    void addPropertiesAct(QMenu& menu);
    void addRemoveAct(QMenu& menu);
    void addZValueAction(QMenu& menu);
    void setMenuCaption(QMenu& menu);

protected:
    mutable SharedItemData data_;
};
} // namespace ceam

#endif // SCENE_ITEM_H
