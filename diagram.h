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
#include "connection_database.h"
#include "device.h"
#include "diagram_meta.h"
#include "diagram_state_machine.h"
#include "scene_background.h"
#include "scene_connections.h"
#include "scene_devices.h"

#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QUndoStack>
#include <QWidget>

class QPrinter;

namespace ceam {

class DiagramScene;
class ScaleWidget;

class Diagram : public QGraphicsView {
    Q_OBJECT

public:
    Q_PROPERTY(bool showCables READ showCables WRITE setShowCables NOTIFY showCablesChanged)

public:
    explicit Diagram(int w, int h, QWidget* parent = nullptr);

    /**
     * Return hash map of all connections of all selected devices
     */
    QHash<ConnectionId, ConnectionViewData> findSelectedConnections() const;

    void printScheme() const;
    void printScheme(QPrinter* printer) const;
    void renderToSvg(const QString& filename, const QString& title) const;
    void renderToPng(const QString& filename) const;

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

    /**
     * @brief add comment into the scheme, emit sceneChanged(), deviceAdded()
     * @return pointer to new comment or nullptr on error
     */
    Comment* addComment();

    bool showCables() const { return show_cables_; }
    void setShowCables(bool value);
    void setShowPeople(bool value);
    void setShowFurniture(bool value);
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
     * @param id - connection id
     * @param viewData - connection view data
     * @return true on success, false on error
     */
    bool connectDevices(const ConnectionId& id, std::optional<ConnectionViewData> viewData);

    /**
     * @brief disconnect devices, emit sceneChanged() and connectionRemoved()
     * @param id - connection id
     * @return true on success, false on error
     */
    bool disconnectDevices(const ConnectionId& id);

    SceneConnections* connections() { return connections_; }
    const SceneConnections* connections() const { return connections_; }

    SceneDevices& devices() { return devices_; }

    // clip buffer
    void clearClipBuffer();
    const QList<SharedDeviceData>& clipBuffer() const;
    void setClipBuffer(const QList<SharedDeviceData>& data);

    /**
     * render diagram as image
     * @return QImage or null image on error
     */
    QImage toImage() const;

    /**
     * render diagram as SVG image
     * @return svg text content and size
     */
    std::pair<QByteArray, QSize> toSvg() const;

    /**
     * check if grid is visible
     */
    bool gridIsVisible() const;

    /**
     * Clear all items and undo stack
     * @emit sceneClearAll()
     */
    void clearAll();

    /**
     * check if scale is visible
     */
    bool scaleIsVisible() const;

public slots:
    // undo/redo commands
    void cmdAddToSelection(const QList<QGraphicsItem*>& items);
    void cmdAddToSelection(const QRectF& sel);
    void cmdAlignHSelected();
    void cmdAlignVSelected();
    void cmdConnectDevices(const ConnectionId& conn);
    void cmdCreateComment(const QPointF& pos);
    void cmdCreateDevice(const QPointF& pos);
    void cmdCutSelected();
    void cmdDisconnectDevices(const ConnectionId& conn);
    void cmdDisconnectXlet(const XletInfo& xi);
    void cmdDistributeHSelected();
    void cmdDistributeVSelected();
    void cmdDuplicateDevice(const SharedDeviceData& data);
    void cmdDuplicateSelection();
    void cmdLockSelected();
    void cmdUnlockSelected();
    void cmdLock(DeviceId id);
    void cmdUnlock(DeviceId id);
    void cmdMirrorDevice(DeviceId id);
    void cmdMirrorSelected();
    void cmdMoveSelectedDevicesBy(qreal dx, qreal dy);
    void cmdMoveSelectedDevicesFrom(const QPointF& from, const QPointF& to);
    void cmdPaste();
    void cmdPlaceInColumnSelected();
    void cmdPlaceInRowSelected();
    void cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn);
    void cmdRemoveDevice(const SharedDeviceData& data);
    void cmdRemoveSelected();
    void cmdSelectAll();
    void cmdSelectDevices(const QRectF& sel);
    void cmdSelectUnique(DeviceId id);
    void cmdToggleDevices(const QList<QGraphicsItem*>& items);
    void cmdUpdateDevice(const SharedDeviceData& data);
    void cmdZoomInSelected();
    void cmdZoomOutSelected();

    void clearUndoStack();
    void copySelected();
    void cutSelected();
    void paste();
    void redo();
    void setGridVisible(bool value);
    void setScaleVisible(bool value);
    void showConnectionEditor();
    void undo();
    void zoomIn();
    void zoomNormal();
    void zoomOut();
    void zoomFitBest();
    void zoomFitSelected();

signals:
    void addToFavorites(SharedDeviceData data);
    void batteryChanged(const BatteryChange& data);
    void canRedoChanged(bool);
    void canUndoChanged(bool);
    void connectionAdded(ConnectionId data);
    void connectionRemoved(ConnectionId data);
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
    void fileFormatVersionMismatch(int fileVersion, int appFileVersion);
    void zoomChanged(qreal);

protected:
    bool viewportEvent(QEvent* event) override;
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

    /**
     * @complexity O(n)
     */
    void updateConnectionStyle(Connection* conn);

    /**
     * save click event position
     * @param pos - event position (not mapped!)
     */
    void saveClickPos(const QPointF& pos);

    void selectTopDevice(const QList<QGraphicsItem*>& devs);
    void selectBottomDevice(const QList<QGraphicsItem*>& devs);
    std::optional<std::pair<XletInfo, XletData>> hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt) const;
    void updateZoom(qreal zoom);

    bool dropJson(const QPointF& pos, const QByteArray& json);

    QJsonValue appInfoJson() const;

    void fitRect(const QRectF& rect);

private:
    DiagramScene* scene_ { nullptr };
    QGraphicsRectItem* selection_ { nullptr };
    QGraphicsLineItem* tmp_connection_ { nullptr };
    QUndoStack* undo_stack_ { nullptr };

    QPointF prev_move_pos_;
    QPointF prev_click_pos_;
    SceneDevices devices_;
    SceneBackground background_;
    SceneConnections* connections_ { nullptr };

    DiagramStateMachine state_machine_;
    std::optional<std::pair<XletInfo, XletData>> conn_begin_;
    qreal zoom_ { 1 };
    bool show_cables_ { true };

    QList<SharedDeviceData> clip_buffer_;
    DiagramMeta meta_;
    ConnectionDatabase conn_database_;
    ScaleWidget* scale_;

private:
    void initLiveConnection();
    void initScale();
    void initScene(int w, int h);
    void initSceneBackground();
    void initSceneConnections();
    void initSceneDevices();
    void initSelectionRect();
    void initUndoStack();

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
