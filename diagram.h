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
#ifndef DIAGRAM_H
#define DIAGRAM_H

#include "connection.h"
#include "device.h"
#include "diagram_image.h"
#include "diagram_meta.h"
#include "diagram_state_machine.h"
#include "scene_connections.h"
#include "scene_devices.h"

#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QUndoStack>
#include <QWidget>

namespace ceam {

class DiagramScene;

class Diagram : public QGraphicsView {
    Q_OBJECT

public:
    Q_PROPERTY(bool showCables READ showCables WRITE setShowCables NOTIFY showCablesChanged)
    Q_PROPERTY(bool showBackground READ showBackground WRITE setShowBackground NOTIFY showBackgroundChanged)

public:
    explicit Diagram(int w, int h, QWidget* parent = nullptr);

    /**
     * Return set of all connections of all selected devices
     */
    QSet<ConnectionData> findSelectedConnections() const;

    void printScheme() const;

    /**
     * Save diagram objects, connections and background into JSON object
     * @return
     */
    QJsonObject toJson() const;

    /**
     * load from JSON file
     * @return true on success, false on error
     */
    bool loadJson(const QString& path);

    /**
     * @brief add device into the scheme, emit sceneChanged(), deviceAdded()
     * @param data - device data
     * @return pointer to new device or nullptr on error
     */
    Device* addDevice(const SharedDeviceData& data);

    /**
     * @brief remove device from scheme, emit sceneChanged(), deviceRemoved(), connectionRemoved()
     * @param id - device id
     * @return true on success, false on error
     */
    bool removeDevice(DeviceId id);

    /**
     * @brief setDeviceData, emit sceneChanged(), deviceUpdated() and batteryChanged()
     * @param data - device data
     * @return true on success, false on error
     */
    bool setDeviceData(const SharedDeviceData& data);

    bool showCables() const { return show_cables_; }
    void setShowCables(bool value);
    bool showBackground() const { return show_background_; }
    void setShowBackground(bool value);

    /**
     * set new background, emit sceneChanged()
     * @param path - full path to background image
     * @return true on success, false on error
     */
    bool setBackground(const QString& path);

    /**
     * clears background, emit sceneChanged()
     */
    void clearBackground();

    /**
     * Diagram meta information
     */
    const DiagramMeta& meta() const { return meta_; }

    /**
     * Set diagram meta information
     */
    void setMeta(const DiagramMeta& meta) { meta_ = meta; }

    /**
     * move specified items by different deltas (when align, for example)
     * @param deltas
     */
    void moveItemsBy(const QHash<DeviceId, QPointF>& deltas);

    /**
     * move selected items by specified same delta
     */
    void moveSelectedItemsBy(qreal dx, qreal dy);

    /**
     * @brief connect devices, emit sceneChanged() and connectionAdded()
     * @param data - connection data
     * @return true on success, false on error
     */
    bool connectDevices(const ConnectionData& data);

    /**
     * @brief disconnect devices, emit sceneChanged() and connectionRemoved()
     * @param data - connection data
     * @return true on success, false on error
     */
    bool disconnectDevices(const ConnectionData& data);

    SceneConnections& connections() { return connections_; }
    const SceneConnections& connections() const { return connections_; }

    SceneDevices& devices() { return devices_; }

    // clip buffer
    void clearClipBuffer();
    const QList<SharedDeviceData>& clipBuffer() const;
    void setClipBuffer(const QList<SharedDeviceData>& data);

    QImage toImage() const;
    std::pair<QByteArray, QSize> toSvg() const;

    bool gridVisible() const;

public slots:
    // undo/redo commands
    void cmdCreateDevice(const QPointF& pos);
    void cmdDuplicateDevice(const SharedDeviceData& data);
    void cmdDuplicateSelection();
    void cmdRemoveSelected();
    void cmdRemoveDevice(const SharedDeviceData& data);
    void cmdUpdateDevice(const SharedDeviceData& data);
    void cmdSelectAll();
    void cmdAddToSelection(const QRectF& sel);
    void cmdAddToSelection(const QList<QGraphicsItem*>& items);
    void cmdSelectDevices(const QRectF& sel);
    void cmdSelectUnique(DeviceId id);
    void cmdToggleDevices(const QList<QGraphicsItem*>& items);
    void cmdConnectDevices(const ConnectionData& conn);
    void cmdDisconnectDevices(const ConnectionData& conn);
    void cmdDisconnectXlet(const XletInfo& xi);
    void cmdMoveSelectedDevicesBy(qreal dx, qreal dy);
    void cmdMoveSelectedDevicesFrom(const QPointF& from, const QPointF& to);
    void cmdAlignVSelected();
    void cmdAlignHSelected();
    void cmdCutSelected();
    void cmdPaste();

    void clearUndoStack();
    void copySelected();
    void cutSelected();
    void paste();
    void redo();
    void setGridVisible(bool value);
    void undo();
    void zoomIn();
    void zoomNormal();
    void zoomOut();
    void zoomFit();

signals:
    void addToFavorites(SharedDeviceData data);
    void batteryChanged(const BatteryChange& data);
    void canRedoChanged(bool);
    void canUndoChanged(bool);
    void connectionAdded(ConnectionData data);
    void connectionRemoved(ConnectionData data);
    void deviceAdded(SharedDeviceData data);
    void deviceRemoved(SharedDeviceData data);
    void deviceTitleUpdated(DeviceId id, const QString& title);
    void deviceUpdated(SharedDeviceData data);
    void requestBackgroundChange();
    void sceneChanged(); // for document changes
    void sceneClearAll();
    void sceneFullUpdate();
    void showBackgroundChanged(bool);
    void showCablesChanged(bool);
    void zoomChanged(qreal);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) final;

private:
    /**
     * @complexity O(1)
     */
    void updateConnectionPos(Connection* conn);

    /**
     * @complexity O(n)
     */
    void updateConnectionPos(DeviceId id);

    void saveClickPos(const QPoint& pos);

    /**
     * @emit sceneClearAll()
     */
    void clearAll();

#ifdef __MACH__
    bool viewportEvent(QEvent* event) override;
#endif

    void selectTopDevice(const QList<QGraphicsItem*>& devs);
    std::optional<XletInfo> hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt) const;
    void updateZoom(qreal zoom);

    bool isValidConnection(const XletInfo& src, const XletInfo& dest) const;
    bool dropJson(const QPointF& pos, const QByteArray& json);

    QJsonValue appInfoJson() const;

private:
    DiagramScene* scene_ { nullptr };
    QGraphicsRectItem* selection_ { nullptr };
    QGraphicsLineItem* tmp_connection_ { nullptr };
    DiagramImage* background_ { nullptr };
    QUndoStack* undo_stack_ { nullptr };
    QPointF prev_event_pos_;
    QPointF prev_click_pos_;
    SceneDevices devices_;
    SceneConnections connections_;

    DiagramStateMachine state_machine_;
    std::optional<XletInfo> conn_start_;
    qreal zoom_ { 1 };
    bool show_cables_ { true };
    bool show_background_ { true };

    QList<SharedDeviceData> clip_buffer_;
    DiagramMeta meta_;

private:
    void initUndoStack();
    void initSelectionRect();
    void initLiveConnection();
    void initScene(int w, int h);
    void initSceneConnections();
    void initSceneDevices();

    /**
     * @param pos - position in view coordinates
     */
    void startSelectionAt(const QPoint& pos);

    /**
     * @param pos - position in view coordinates
     */
    void startConnectionAt(const QPoint& pos);

    /**
     * @param pos - position in view coordinates
     */
    void drawConnectionTo(const QPoint& pos);

    /**
     * @param pos - position in view coordinates
     */
    void drawSelectionTo(const QPoint& pos);
};

}

#endif // DIAGRAM_H
