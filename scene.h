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
#ifndef SCENE_H
#define SCENE_H

#include "connection_database.h"
#include "device_common.h"

#include <QObject>
#include <unordered_map>

class QGraphicsScene;

namespace ceam {

class CommentItem;
class SceneItem;

class Scene : public QObject {
    Q_OBJECT

private:
    QGraphicsScene* scene_;
    std::unordered_map<SceneItemId, SceneItem*> items_;
    SceneItemId edited_comment_id_;

public:
    Scene();

    bool operator==(const Scene& sc) const;
    bool operator!=(const Scene& sc) const { return !operator==(sc); }

    /**
     * set graphics scene to operate on
     */
    void setGraphicsScene(QGraphicsScene* scene);

    /**
     * return number of all scene items
     */
    size_t count() const { return items_.size(); }

    /**
     * return number of selected scene items
     * @complexity O(n)
     */
    size_t selectedCount() const;

    /**
     * create new item and add it to the scene
     * @return pointer to new item or nullptr on error
     * @emit added(SharedItemData)
     */
    SceneItem* add(const SharedItemData& data);

    /**
     * remove item from the scene
     * @return removed device data on success or null device data on error
     * @emit removed(SharedItemData)
     */
    SharedItemData remove(SceneItemId id);

    /**
     * remove all scene items
     * @emit removed(SharedItemData) for every deleted item
     */
    void clear();

    /**
     * find device by given device id
     * @return pointer to item or nullptr if not found
     * @complexity O(1)
     */
    SceneItem* find(SceneItemId id);

    /**
     * find item by given item id
     * @return pointer to item or nullptr if not found
     * @complexity O(1)
     */
    const SceneItem* find(SceneItemId id) const;

    /**
     * find first item that is lower than specified
     * @param id
     * @return pointer to item or nullptr
     * @complexity ~O(n)?
     */
    const SceneItem* findFirstLower(SceneItemId id) const;

    /**
     * find first item that is upper than specified
     * @param id
     * @return pointer to item or nullptr
     * @complexity ~O(n)?
     */
    const SceneItem* findFirstUpper(SceneItemId id) const;

    /**
     * find item data by given device id
     * @return device data or empty data if not found
     * @complexity O(1)
     */
    SharedItemData findData(SceneItemId id) const;

    std::optional<DeviceConnectionData> connectionInfo(const ConnectionId& id) const;

    /**
     * return connection points in the scene coords
     * @param id - connection data
     * @return pair of point, or null on error
     * @complexity O(1)
     */
    std::optional<std::pair<QPointF, QPointF>> connectionPoints(const ConnectionId& id) const;

    /**
     * return connection endpoint data
     * @param conn - connection data
     * @return pair of XletData, or null on error
     * @complexity O(1)
     */
    std::optional<ConnectorPair> connectionPair(const ConnectionId& id) const;

    /**
     * check if connection data is valid
     * @param conn - connection data
     * @complexity O(1)
     */
    bool checkConnection(const ConnectionId& id) const;

    bool hasSelected() const;
    int setSelected(const QList<SceneItemId>& ids, bool value);
    void toggleSelected(const QList<SceneItemId>& ids);

    bool moveBy(const QHash<SceneItemId, QPointF>& deltas);

    /**
     * @complexity O(n)
     */
    bool moveSelectedBy(qreal dx, qreal dy);

    QList<SceneItemId> idList() const;
    QList<SharedItemData> dataList() const;

    QList<SceneItemId> selectedIdList() const;
    QList<SharedItemData> selectedDataList() const;

    QRectF boundingRect() const;
    QRectF boundingSelectRect() const;

    void foreachItem(const std::function<void(SceneItem*)>& fn);
    void foreachSelectedItem(const std::function<void(const SceneItem*)>& fn) const;

    void foreachData(const std::function<void(const SharedItemData& data)>& fn) const;
    void foreachSelectedData(const std::function<void(const SharedItemData& data)>& fn) const;

    /**
     * return a set of device id that intersects with the given rectangle
     * @param rect
     */
    QSet<SceneItemId> intersected(const QRectF& rect) const;

    /**
     * return a list of device id that intersects with the given rectangle
     * @param rect
     */
    QList<SceneItemId> intersectedList(const QRectF& rect) const;

    /**
     * export scene items to json array
     */
    QJsonValue toJson() const;

    /**
     * done comment editimg if any
     */
    void doneCommentEditor();

signals:
    void added(SharedItemData);
    void removed(SharedItemData);
    void showCommentEditor(bool);
};

} // namespace ceam
QDebug operator<<(QDebug debug, const ceam::Scene& sc);

#endif // SCENE_H
