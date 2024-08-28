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

#include "connection.h"
#include "device_common.h"

#include <QObject>
#include <unordered_map>

class QGraphicsScene;

namespace ceam {

class Device;

class SceneDevices : public QObject {
    Q_OBJECT

private:
    QGraphicsScene* scene_;
    std::unordered_map<DeviceId, Device*> devices_;

public:
    SceneDevices();

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

    Device* find(DeviceId id);
    const Device* find(DeviceId id) const;
    SharedDeviceData findData(DeviceId id) const;

    std::optional<ConnectionFullInfo> connectionInfo(const ConnectionData& conn) const;

    /**
     * return connection points in the scene coords
     * @param conn - connection data
     * @return pair of point, or null on error
     * @complexity O(1)
     */
    std::optional<std::pair<QPointF, QPointF>> connectionPoints(const ConnectionData& conn) const;

    /**
     * check if connection data is valid
     * @param conn - connection data
     * @complexity O(1)
     */
    bool checkConnection(const ConnectionData& conn) const;

    bool hasSelected() const;
    void setSelected(const QList<DeviceId>& ids, bool value);
    void toggleSelected(const QList<DeviceId>& ids);

    bool moveBy(const QHash<DeviceId, QPointF>& deltas);

    /**
     * @complexity O(n)
     */
    bool moveSelectedBy(int dx, int dy);

    QList<DeviceId> idList() const;
    QList<SharedDeviceData> dataList() const;

    QList<DeviceId> selectedIdList() const;
    QList<SharedDeviceData> selectedDataList() const;

    QRectF boundingRect() const;

    void foreachDevice(std::function<void(Device*)> fn);
    void foreachDevice(std::function<void(const Device*)> fn) const;
    void foreachSelectedDevice(std::function<void(Device*)> fn);
    void foreachSelectedDevice(std::function<void(const Device*)> fn) const;

    void foreachData(std::function<void(const SharedDeviceData& data)> fn) const;
    void foreachSelectedData(std::function<void(const SharedDeviceData& data)> fn) const;

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

    /**
     * remove all current scene devices and set new from specified json array
     * @return true on success, false on error
     * @emit removed(), added()
     */
    bool setFromJson(const QJsonValue& v);

signals:
    void added(SharedDeviceData);
    void removed(SharedDeviceData);
};

}

#endif // SCENE_DEVICES_H
