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

struct ConnectionFullInfo {
    SharedDeviceData src_data, dest_data;
    XletData src_out, dest_in;
};

class SceneDevices : public QObject {
    Q_OBJECT

private:
    QGraphicsScene* scene_;
    std::unordered_map<DeviceId, Device*> devices_;

public:
    SceneDevices();

    void setScene(QGraphicsScene* scene);
    size_t count() const { return devices_.size(); }
    size_t selectedCount() const;

    Device* add(const SharedDeviceData& data);
    SharedDeviceData remove(DeviceId id);
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

    void moveBy(const QHash<DeviceId, QPointF>& deltas);

    /**
     * @complexity O(n)
     */
    bool moveSelectedBy(int dx, int dy);

    void clear();

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
};

}

#endif // SCENE_DEVICES_H
