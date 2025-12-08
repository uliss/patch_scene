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
#ifndef DIAGRAM_SCENE_H
#define DIAGRAM_SCENE_H

#include "device_xlet.h"
#include "diagram_state_machine.h"
#include "scene.h"
#include "scene_background.h"
#include "xlet_info.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QUndoStack>

class QGraphicsItemGroup;
class QMouseEvent;
class QPrinter;

namespace ceam {

class Connection;
class ConnectionId;
class DiagramMeta;
class SceneConnections;

class DiagramScene : public QGraphicsScene {
    Q_OBJECT
public:
    struct DuplicatePolicy {
        bool select_new { true }, unselect_origin { true };
    };

public:
    explicit DiagramScene(int w, int h, QObject* parent = nullptr);

    bool gridVisible() const { return grid_visible_; }
    void setGridVisible(bool value);

    void setCacheMode(QGraphicsItem::CacheMode mode);

    void renderDiagram(QPainter* painter, const QRect& rect = {});
    QImage renderToImage(qreal scale);
    void printDiagram(QPrinter* printer);

    QRectF bestFitRect() const;

    /**
     * return number of items on the scene
     */
    size_t itemCount() const { return item_scene_.count(); }

    /**
     * return number of connections on the scene
     */
    size_t connectionCount() const;

    /**
     * @return current scene state
     */
    DiagramState state() const { return state_machine_.state(); }

    /**
     * set current scene state
     */
    void setState(DiagramState s) { state_machine_.setState(s); }

    /**
     * undo stack
     */
    QUndoStack* undoStack() { return undo_stack_; }
    const QUndoStack* undoStack() const { return undo_stack_; }

    /**
     * Item scene
     */
    Scene& itemScene() { return item_scene_; }
    const Scene& itemScene() const { return item_scene_; }

    /**
     * Connections
     */
    SceneConnections* connections() { return connections_; }
    const SceneConnections* connections() const { return connections_; }

    /**
     * Connection database
     */
    const ConnectionDatabase& connectionDb() const { return conn_database_; }

    /**
     * background
     */
    SceneBackground& background() { return background_; }
    const SceneBackground& background() const { return background_; }

public: // for tests
    bool isSelectionRectVisible() const { return selection_->isVisible(); }

public: // static
    static std::optional<std::pair<XletInfo, XletData>> hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPointF& pt);
    static QJsonValue appInfoJson();

public: // scene actions
    /**
     * @brief add item into the scheme, emit sceneChanged() and possibly deviceAdded()
     * @param data - item data
     * @return pointer to new item or nullptr on error
     */
    SceneItem* addSceneItem(const SharedItemData& data);

    /**
     * @brief remove item from the scheme, emit sceneChanged(), deviceRemoved(), connectionRemoved()
     * @param id - item id
     * @return true on success, false on error
     */
    bool removeSceneItem(SceneItemId id);

    /**
     * @brief setItemData, emit sceneChanged(), deviceUpdated() and maybe batteryChanged(), deviceTitleUpdated()
     * @param data - item data
     * @return true on success, false on error
     */
    bool setItemData(const SharedItemData& data);

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

    /**
     * duplicated selected items
     * @return list new item id's
     */
    QList<SceneItemId> duplicateSelected(DuplicatePolicy policy);

    /**
     * Return hash map of all connections of all selected devices
     */
    QHash<ConnectionId, ConnectionViewData> findSelectedConnections() const;

    /**
     * move specified items by different deltas (when align, for example)
     * @param deltas
     * @emit sceneChanged()
     */
    void moveItemsBy(const QHash<SceneItemId, QPointF>& deltas);

    /**
     * move selected items by specified same delta
     * @emit sceneChanged()
     */
    void moveSelectedItemsBy(qreal dx, qreal dy);

    /**
     * @complexity O(1)
     */
    void updateConnectionPos(Connection* conn);

    /**
     * @complexity O(n)
     * @emit none
     */
    void updateConnectionPos(SceneItemId id);

    /**
     * @complexity O(n)
     */
    void updateConnectionStyle(Connection* conn);

    /**
     * clears background, emit sceneChanged()
     */
    void clearBackground();

    /**
     * set new background, emit sceneChanged()
     * @param path - full path to background image
     * @return true on success, false on error
     */
    bool setBackground(const QString& path);

    /**
     * show/hide background image
     * emits showBackgroundChanged()
     */
    void setShowBackground(bool value);

    /**
     * Save diagram objects, connections and background into JSON object
     * @return json object
     */
    QJsonObject toJson() const;

    /**
     * load from JSON file
     * @return true on success, false on error
     */
    bool loadJson(const QString& path, const char* metaKey, DiagramMeta& meta);

    /**
     * remove all undo commands
     */
    void clearUndoStack();

    /**
     * remove all items from the scene
     * emit sceneClearAll()
     */
    void clearAll();

    /**
     * drop Json to specified point
     * @param pos
     * @param json
     * @return
     */
    bool dropJson(const QPointF& pos, const QByteArray& json);

    /**
     * undo last changes
     */
    void undo();

    /**
     * redo previously ondone changes
     */
    void redo();

public slots:
    // commands with undo/redo support
    void cmdAddToSelection(const QList<QGraphicsItem*>& items);
    void cmdAddToSelection(const QRectF& sel);
    void cmdAlignHSelected();
    void cmdAlignVSelected();
    void cmdConnectDevices(const ConnectionId& conn);
    void cmdCreateComment(const QPointF& pos);
    void cmdCreateDevice(const QPointF& pos);
    void cmdDisconnectDevices(const ConnectionId& conn);
    void cmdDisconnectXlet(const XletInfo& xi);
    void cmdDistributeHSelected();
    void cmdDistributeVSelected();
    void cmdDuplicateSelected();
    void cmdMoveSelectedItemsBy(qreal dx, qreal dy);
    void cmdMoveSelectedItemsFrom(const QPointF& from, const QPointF& to);
    void cmdPlaceInColumnSelected();
    void cmdPlaceInRowSelected();
    void cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn);
    void cmdRemoveSelected();
    void cmdSelectItems(const QRectF& sel);
    void cmdSelectUnique(SceneItemId id);
    void cmdToggleSelected(const QList<QGraphicsItem*>& items);

    void cmdDuplicateItem(const SharedItemData& data);
    void cmdRemoveItem(const SharedItemData& data);
    void cmdUpdateItem(const SharedItemData& data);

    void cmdMoveLower(const SharedItemData& data);
    void cmdMoveUpper(const SharedItemData& data);

    void cmdLockSelected();
    void cmdUnlockSelected();

    void cmdLock(SceneItemId id);
    void cmdUnlock(SceneItemId id);

    void cmdMirrorDevice(SceneItemId id);
    void cmdMirrorSelected();

private slots:
    void showCommentEditor(bool value);
    void showConnectionEditor();

signals:
    void sceneClearAll();
    void addToFavorites(SharedItemData data);
    void batteryChanged(const BatteryChange& data);
    void connectionAdded(ConnectionId);
    void connectionRemoved(ConnectionId);
    void deviceAdded(SharedItemData data);
    void deviceRemoved(SharedItemData data);
    void deviceTitleUpdated(SceneItemId id, const QString& title);
    void deviceUpdated(SharedItemData data);
    void fileFormatVersionMismatch(int fileVersion, int appFileVersion);
    void removeConnection(const ConnectionId&);
    void sceneChanged(); // for document changes
    void sceneFullUpdate();
    void showCablesChanged(bool);

    void requestBackgroundChange();
    void showBackgroundChanged(bool);

    void canRedoChanged(bool);
    void canUndoChanged(bool);

private:
    void initItemScene();
    void initLiveConnection();
    void initSceneBackground();
    void initSceneConnections();
    void initSelectionRect();
    void initUndoStack();

    void saveClickPos(const QPointF& pos);

    /**
     * start selection start point
     */
    void startSelectionAt(const QPointF& pos);

    /**
     * @param pos - position in scene coordinates
     */
    void drawSelectionTo(const QPointF& pos);

    /**
     * @param pos - position in view coordinates
     */
    void drawLiveConnectionTo(const QPointF& pos);

    /**
     * @param pos - position in view coordinates
     */
    void startLiveConnectionAt(const QPointF& pos);

    /**
     * select lowest item in given list
     * @complexity O(n)
     */
    void selectLowestItem(const QList<QGraphicsItem*>& devs);

    /**
     * select topmost item from given list
     * @complexity O(n)
     */
    void selectTopItem(const QList<QGraphicsItem*>& devs);

    /**
     * send mouse event to specified items, if item accepts event, sending stops
     */
    void sendMouseEventTo(const QList<QGraphicsItem*>& items, QGraphicsSceneMouseEvent* event);

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    void keyPressEvent(QKeyEvent* event) override;

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) override;
    void dragMoveEvent(QGraphicsSceneDragDropEvent* event) override;
    void dropEvent(QGraphicsSceneDragDropEvent* event) override;

private:
    QGraphicsRectItem* selection_ { nullptr };
    QGraphicsLineItem* tmp_connection_ { nullptr };
    QUndoStack* undo_stack_ { nullptr };
    SceneConnections* connections_ { nullptr };

    Scene item_scene_;
    SceneBackground background_;
    ConnectionDatabase conn_database_;

    DiagramStateMachine state_machine_;

    std::optional<std::pair<XletInfo, XletData>> conn_begin_;
    QPointF prev_move_pos_;
    QPointF prev_click_pos_;
    bool grid_visible_ { true };
};

} // namespace ceam

#endif // DIAGRAM_SCENE_H
