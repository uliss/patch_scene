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

#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTimer>
#include <QUndoStack>
#include <QWidget>

enum class DiagramState {
    None,
    Move,
    ConnectDevice,
    SelectDevice,
    SelectionRect,
};

class Diagram : public QGraphicsView {
    Q_OBJECT

public:
    Q_PROPERTY(bool showCables READ showCables WRITE setShowCables NOTIFY showCablesChanged)
    Q_PROPERTY(bool showImages READ showImages WRITE setShowImages NOTIFY showImagesChanged)

public:
    explicit Diagram(QWidget* parent = nullptr);

    Connection* findConnectionByXlet(const XletInfo& xi) const;

    void printScheme() const;
    QJsonObject toJson() const;
    bool loadJson(const QString& path);

    /**
     * @brief find connection info
     * @param src
     * @param dest
     * @return true on success, false on error
     * @note complexity O(N)
     */
    bool findConnectionXletData(const ConnectionData& data, XletData& src, XletData& dest, Device** src_dev, Device** dest_dev) const;

    /**
     * @brief find device by id
     * @param id - device id
     * @return pointer to found Device or nullptr
     * @note complexity O(N)
     */
    Device* findDeviceById(DeviceId id) const;

    /**
     * @brief add device into the scheme, emit sceneChanged(), deviceAdded()
     * @param dev - pointer to device, takes ownership
     * @return true on success, false on error
     */
    bool addDevice(Device* dev);

    /**
     * @brief remove device from scheme, emit sceneChanged(), deviceRemoved(), connectionRemoved()
     * @param id - device id
     * @return true on success, false on error
     */
    bool removeDevice(DeviceId id);

    /**
     * @brief setDeviceData, emit sceneChanged(), deviceUpdated()
     * @param id - device id
     * @param data - device data
     * @return true on success, false on error
     */
    bool setDeviceData(DeviceId id, const SharedDeviceData& data);

    bool showCables() const { return show_cables_; }
    void setShowCables(bool value);
    bool showImages() const { return show_images_; }
    void setShowImages(bool value);

    bool addImage(DiagramImage* img);
    bool addImage(const QString& path);

    // undo/redo commands
    void cmdCreateDevice(const QPointF& pos);
    void cmdDuplicateDevice(const SharedDeviceData& data);
    void cmdDuplicateSelection();
    void cmdRemoveSelected();
    void cmdRemoveDevice(const SharedDeviceData& data);
    void cmdSelectAll();
    void cmdAddToSelection(const QRectF& sel);
    void cmdSelectDevices(const QRectF& sel);
    void cmdSelectUnique(DeviceId id);
    void cmdToggleDevices(const QList<QGraphicsItem*>& items);
    void cmdConnectDevices(const ConnectionData& conn);
    void cmdDisconnectXlet(const XletInfo& xi);
    void cmdMoveSelectedDevicesBy(qreal dx, qreal dy);
    void cmdMoveSelectedDevicesFrom(const QPointF& from, const QPointF& to);
    void cmdAlignVSelected();
    void cmdAlignHSelected();
    void cmdCutSelected();
    void cmdPaste();

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

    QList<Connection*> connections() const;
    QList<Device*> devices() const;
    QList<Device*> selectedDevices() const;
    QList<DiagramImage*> images() const;

    void selectDevices(const QList<DeviceId>& ids, bool value = true);
    void toggleDevices(const QList<DeviceId>& ids);

    // clip buffer
    void clearClipBuffer();
    const QList<SharedDeviceData>& clipBuffer() const;
    void setClipBuffer(const QList<SharedDeviceData>& data);

signals:
    void sceneChanged(); // for document changes
    void canRedoChanged(bool);
    void canUndoChanged(bool);
    void showCablesChanged(bool);
    void showImagesChanged(bool);
    void zoomChanged(qreal);
    void deviceAdded(SharedDeviceData data);
    void deviceRemoved(SharedDeviceData data);
    void deviceUpdated(SharedDeviceData data);
    void connectionAdded(ConnectionData data);
    void connectionRemoved(ConnectionData data);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomNormal();
    void undo();
    void redo();
    void copySelected();
    void cutSelected();
    void paste();

private slots:
    void updateConnectionsPos();

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    QGraphicsScene* scene;

private:
    bool addConnection(Connection* conn);
    void saveClickPos(const QPoint& pos);
    QList<DeviceId> allDeviceIds() const;
    void clearAll();

#ifdef __MACH__
    bool viewportEvent(QEvent* event) override;
#endif

    void startSelectionRect(const QPoint& pos);
    void selectTopDevice(const QList<QGraphicsItem*>& devs);
    XletInfo hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt) const;
    void updateZoom(qreal zoom);

    bool isValidConnection(const XletInfo& src, const XletInfo& dest) const;

private:
    QGraphicsRectItem* selection_ { nullptr };
    QGraphicsLineItem* connection_ { nullptr };
    QPoint selection_origin_;
    QPointF prev_event_pos_;
    QPointF prev_click_pos_;
    QUndoStack* undo_stack_ = nullptr;
    DiagramState state_ { DiagramState::None };
    XletInfo conn_start_;
    qreal zoom_ { 1 };
    bool show_cables_ { true };
    bool show_images_ { true };

    QList<SharedDeviceData> clip_buffer_;
};

#endif // DIAGRAM_H
