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
#include "diagram_meta.h"
#include "diagram_state_machine.h"
#include "scene.h"
#include "scene_connections.h"
#include "scene_item.h"

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
    Diagram(int w, int h, QWidget* parent = nullptr);

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
     * @brief add item into the scheme, emit sceneChanged() and possibly deviceAdded()
     * @param data - item data
     * @return pointer to new item or nullptr on error
     */
    SceneItem* addItem(const SharedItemData& data);

    /**
     * @brief remove item from the scheme, emit sceneChanged(), deviceRemoved(), connectionRemoved()
     * @param id - item id
     * @return true on success, false on error
     */
    bool removeItem(SceneItemId id);

    /**
     * @brief setItemData, emit sceneChanged(), deviceUpdated() and batteryChanged()
     * @param data - item data
     * @return true on success, false on error
     */
    bool setItemData(const SharedItemData& data);

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
    void moveItemsBy(const QHash<SceneItemId, QPointF>& deltas);

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

    SceneConnections* connections();
    const SceneConnections* connections() const;

    Scene& itemScene();
    const Scene& itemScene() const;

    // clip buffer
    void clearClipBuffer();
    const QList<SharedItemData>& clipBuffer() const;
    void setClipBuffer(const QList<SharedItemData>& data);

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
    void cmdConnectDevices(const ConnectionId& conn);
    void cmdCreateComment(const QPointF& pos);
    void cmdCreateDevice(const QPointF& pos);
    void cmdCutSelected();
    void cmdDisconnectDevices(const ConnectionId& conn);
    void cmdDisconnectXlet(const XletInfo& xi);
    void cmdDistributeHSelected();
    void cmdDistributeVSelected();
    void cmdDuplicateItems(const SharedItemData& data);
    void cmdDuplicateSelected();
    void cmdLockSelected();
    void cmdUnlockSelected();
    void cmdLock(SceneItemId id);
    void cmdUnlock(SceneItemId id);
    void cmdMirrorDevice(SceneItemId id);
    void cmdMirrorSelected();
    void cmdMoveSelectedItemsBy(qreal dx, qreal dy);
    void cmdMoveSelectedItemsFrom(const QPointF& from, const QPointF& to);
    void cmdPaste();
    void cmdPlaceInColumnSelected();
    void cmdPlaceInRowSelected();
    void cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn);
    void cmdRemoveItem(const SharedItemData& data);
    void cmdRemoveSelected();
    void cmdSelectAll();
    void cmdSelectItems(const QRectF& sel);
    void cmdSelectUnique(SceneItemId id);
    void cmdToggleSelected(const QList<QGraphicsItem*>& items);
    void cmdUpdateItem(const SharedItemData& data);
    void cmdZoomInSelected();
    void cmdZoomOutSelected();

    void cmdMoveLower(const SharedItemData& data);
    void cmdMoveUpper(const SharedItemData& data);

    void copySelected();
    void cutSelected();
    void paste();
    void redo();
    void setGridVisible(bool value);
    void setScaleVisible(bool value);
    void undo();
    void zoomIn();
    void zoomNormal();
    void zoomOut();
    void zoomFitBest();
    void zoomFitSelected();

public:
    // for tests
    DiagramState state() const;

signals:
    void addToFavorites(SharedItemData data);
    void batteryChanged(const BatteryChange& data);
    void canRedoChanged(bool);
    void canUndoChanged(bool);
    void connectionAdded(ConnectionId data);
    void connectionRemoved(ConnectionId data);
    void deviceAdded(SharedItemData data);
    void deviceRemoved(SharedItemData data);
    void deviceTitleUpdated(SceneItemId id, const QString& title);
    void deviceUpdated(SharedItemData data);
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
    void wheelEvent(QWheelEvent* event) final;

private:
    void initGraphicsScene(int w, int h);
    void initScale();

    void fitRect(const QRectF& rect);
    void updateZoom(qreal zoom);

private:
    DiagramScene* diagram_scene_ { nullptr };
    ScaleWidget* scale_ { nullptr };

    qreal zoom_ { 1 };
    bool show_cables_ { true };

    QList<SharedItemData> clip_buffer_;
    DiagramMeta meta_;
};

} // namespace ceam

#endif // DIAGRAM_H
