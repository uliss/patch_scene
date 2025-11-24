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
#ifndef SCENE_DEVICES_H
#define SCENE_DEVICES_H

#include "connection_database.h"
#include "device_common.h"

#include <QObject>
#include <unordered_map>

class QGraphicsScene;

namespace ceam {

class Comment;
class Device;

class SceneDevices : public QObject {
    Q_OBJECT

private:
    QGraphicsScene* scene_;
    std::unordered_map<DeviceId, Device*> devices_;

public:
    SceneDevices();

    bool operator==(const SceneDevices& sc) const;
    bool operator!=(const SceneDevices& sc) const { return !operator==(sc); }

    /**
     * set scene to operate on
     */
    void setScene(QGraphicsScene* scene);

    /**
     * return number of all devices
     */
    size_t count() const { return devices_.size(); }

    /**
     * return number of selected devices
     * @complexity O(n)
     */
    size_t selectedCount() const;

    /**
     * create new device and add it to the scene
     * @return pointer to new device or nullptr on error
     * @emit added(SharedDeviceData)
     */
    Device* add(const SharedDeviceData& data);

    /**
     * creates new comment and add it to the scene
     * @return pointer to new device or nullptr on error
     */
    Comment* addComment();

    /**
     * remove device from scene
     * @return removed device data on success or null device data on error
     * @emit removed(SharedDeviceData)
     */
    SharedDeviceData remove(DeviceId id);

    /**
     * remove all devices
     * @emit removed(SharedDeviceData) for every deleted device
     */
    void clear();

    /**
     * find device by given device id
     * @return pointer to device or nullptr if not found
     * @complexity O(1)
     */
    Device* find(DeviceId id);

    /**
     * find device by given device id
     * @return pointer to device or nullptr if not found
     * @complexity O(1)
     */
    const Device* find(DeviceId id) const;

    /**
     * find device data by given device id
     * @return device data or empty data if not found
     * @complexity O(1)
     */
    SharedDeviceData findData(DeviceId id) const;

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
    int setSelected(const QList<DeviceId>& ids, bool value);
    void toggleSelected(const QList<DeviceId>& ids);

    bool moveBy(const QHash<DeviceId, QPointF>& deltas);

    /**
     * @complexity O(n)
     */
    bool moveSelectedBy(qreal dx, qreal dy);

    QList<DeviceId> idList() const;
    QList<SharedDeviceData> dataList() const;

    QList<DeviceId> selectedIdList() const;
    QList<SharedDeviceData> selectedDataList() const;

    QRectF boundingRect() const;
    QRectF boundingSelectRect() const;

    void foreachDevice(const std::function<void(Device*)>& fn);
    void foreachSelectedDevice(const std::function<void(const Device*)>& fn);

    void foreachData(const std::function<void(const SharedDeviceData& data)>& fn) const;
    void foreachSelectedData(const std::function<void(const SharedDeviceData& data)>& fn) const;

    /**
     * return a set of device id that intersects with the given rectangle
     * @param rect
     */
    QSet<DeviceId> intersected(const QRectF& rect) const;

    /**
     * return a list of device id that intersects with the given rectangle
     * @param rect
     */
    QList<DeviceId> intersectedList(const QRectF& rect) const;

    /**
     * export scene devices to json array
     */
    QJsonValue toJson() const;

signals:
    void added(SharedDeviceData);
    void removed(SharedDeviceData);
};

} // namespace ceam
QDebug operator<<(QDebug debug, const ceam::SceneDevices& sc);

#endif // SCENE_DEVICES_H
