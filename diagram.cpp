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
#include "diagram.h"

#include <QBuffer>
#include <QFile>
#include <QGraphicsSceneContextMenuEvent>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPrintDialog>
#include <QPrinter>
#include <QSvgGenerator>

#include "app_version.h"
#include "diagram_scene.h"
#include "diagram_updates_blocker.h"
#include "logging.hpp"
#include "scale_widget.h"
#include "scene_item.h"
#include "undo_commands.h"

using namespace ceam;

namespace {
constexpr qreal MAX_ZOOM = 4.0;
constexpr qreal MIN_ZOOM = 1.0 / MAX_ZOOM;

#ifdef Q_OS_WINDOWS
constexpr qreal PIXEL_ZOOM_FACTOR = 1.05;
constexpr qreal ANGLE_ZOOM_FACTOR = 1.05;
#else
constexpr qreal PIXEL_ZOOM_FACTOR = 1.02;
constexpr qreal ANGLE_ZOOM_FACTOR = 1.02;
#endif

constexpr const char* JSON_KEY_APP = "application";
constexpr const char* JSON_KEY_BACKGROUND = "background";
constexpr const char* JSON_KEY_CONNS = "connections";
constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_VIEW = "view";
constexpr const char* JSON_KEY_FORMAT_VERSION = "format-version";
constexpr const char* JSON_KEY_META = "meta";
constexpr const char* JSON_KEY_VERSION = "version";
constexpr const char* JSON_KEY_VERSION_GIT = "version-git";
constexpr const char* JSON_KEY_VERSION_MAJOR = "version-major";
constexpr const char* JSON_KEY_VERSION_MINOR = "version-minor";
constexpr const char* JSON_KEY_VERSION_PATCH = "version-patch";

} // namespace

void Diagram::initUndoStack()
{
    undo_stack_ = new QUndoStack(this);
    connect(undo_stack_, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
    connect(undo_stack_, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
}

void ceam::Diagram::initScale()
{
    scale_ = new ScaleWidget(this);
    scale_->setPos({ 20, 20 });
    scale_->show();
    connect(this, SIGNAL(zoomChanged(qreal)), scale_, SLOT(setScale(qreal)));
}

Diagram::Diagram(int w, int h, QWidget* parent)
    : QGraphicsView { parent }
{
    meta_.setTitle(tr("New project"));
    viewport()->setAttribute(Qt::WA_AcceptTouchEvents);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMinimumWidth(400);
    setMinimumHeight(300);

    setAcceptDrops(true);

    setRenderHint(QPainter::Antialiasing);

    initGraphicsScene(w, h);
    initSceneConnections();
    initItemScene();
    initSceneBackground();
    initScale();

    initLiveConnection();
    initSelectionRect();
    initUndoStack();

    conn_database_.initDefault();
}

void Diagram::initSelectionRect()
{
    selection_ = new QGraphicsRectItem();
    selection_->setZValue(ZVALUE_SELECTION);
    auto pen = QPen(Qt::blue);
    pen.setDashPattern({ 2, 2 });
    selection_->setPen(pen);
    selection_->setVisible(false);
    graphics_scene_->addItem(selection_);
}

void Diagram::initLiveConnection()
{
    tmp_connection_ = new QGraphicsLineItem();
    tmp_connection_->setZValue(ZVALUE_LIVE_CONN);
    tmp_connection_->setVisible(false);
    graphics_scene_->addItem(tmp_connection_);
}

void Diagram::initSceneConnections()
{
    connections_ = new SceneConnections(graphics_scene_, this);
    connect(connections_, &SceneConnections::added, this, &Diagram::connectionAdded);
    connect(connections_, &SceneConnections::removed, this, &Diagram::connectionRemoved);
    connect(connections_, &SceneConnections::visibleChanged, this, &Diagram::showCablesChanged);
    connect(connections_, &SceneConnections::update, this, &Diagram::sceneChanged);
    connect(connections_, &SceneConnections::edit, this, &Diagram::showConnectionEditor);
}

void Diagram::initItemScene()
{
    item_scene_.setGraphicsScene(graphics_scene_);
    connect(&item_scene_, &Scene::added, this, &Diagram::deviceAdded);
    connect(&item_scene_, &Scene::removed, this, &Diagram::deviceRemoved);
}

void Diagram::initGraphicsScene(int w, int h)
{
    graphics_scene_ = new DiagramScene(w, h, this);
    connect(graphics_scene_, &DiagramScene::removeConnection, this, &Diagram::cmdDisconnectDevices);
    setScene(graphics_scene_);

    // NB: should be called after setScene(scene_)!
    graphics_scene_->initGrid();
}

void Diagram::initSceneBackground()
{
    background_.setScene(graphics_scene_);
    connect(&background_, &SceneBackground::backgroundChanged, this,
        [this]() { emit sceneChanged(); });

    connect(&background_, &SceneBackground::requestBackgroundChange, this,
        [this]() { emit requestBackgroundChange(); });
}

bool Diagram::removeItem(SceneItemId id)
{
    auto data = item_scene_.remove(id);
    if (data) {
        connections_->removeAll(id);
        emit sceneChanged();
    }

    return true;
}

void Diagram::updateConnectionPos(Connection* conn)
{
    if (!conn)
        return;

    auto conn_pos = item_scene_.connectionPoints(conn->connectionId());
    if (conn_pos) {
        conn->setPoints(conn_pos->first, conn_pos->second);
        conn->setVisible(true);
    } else {
        conn->setVisible(false);
    }
}

void Diagram::updateConnectionPos(SceneItemId id)
{
    DiagramUpdatesBlocker ub(this);

    for (auto conn : connections_->findConnections(id))
        updateConnectionPos(conn);
}

void Diagram::updateConnectionStyle(Connection* conn)
{
    if (!conn)
        return;

    auto pair = item_scene_.connectionPair(conn->connectionId());
    if (!pair) {
        qWarning() << "connection pair not found";
        return;
    }

    auto style = conn_database_.search(*pair);
    if (style != ConnectionStyle::NotFound) {
        conn->setStyle(style);
    } else {
        qWarning() << "style not found";
    }
}

void Diagram::cmdRemoveSelected()
{
    auto del_sel = new RemoveSelected(this);
    undo_stack_->push(del_sel);
}

void Diagram::cmdRemoveItem(const SharedItemData& data)
{
    auto rem = new RemoveItem(this, data);
    undo_stack_->push(rem);
}

void Diagram::cmdUpdateItem(const SharedItemData& data)
{
    if (!data)
        return;
    auto up = new UpdateDeviceData(this, data);
    undo_stack_->push(up);
}

void Diagram::cmdZoomInSelected()
{
    auto zoom = new ZoomSelected(this, 1.25);
    undo_stack_->push(zoom);
}

void Diagram::cmdZoomOutSelected()
{
    auto zoom = new ZoomSelected(this, 1 / 1.25);
    undo_stack_->push(zoom);
}

void Diagram::cmdMoveLower(const SharedItemData& data)
{
    auto move = new MoveLower(this, data->id());
    undo_stack_->push(move);
}

void Diagram::cmdMoveUpper(const SharedItemData& data)
{
    auto move = new MoveUpper(this, data->id());
    undo_stack_->push(move);
}

void Diagram::cmdCreateDevice(const QPointF& pos)
{
    auto add = new CreateDevice(this, pos);
    undo_stack_->push(add);
}

void Diagram::cmdToggleSelected(const QList<QGraphicsItem*>& items)
{
    QList<SceneItemId> ids;
    ids.reserve(items.size());
    for (auto x : items) {
        auto dev = qgraphicsitem_cast<SceneItem*>(x);
        if (dev)
            ids.push_back(dev->id());
    }

    auto tgl = new ToggleSelected(this, ids);
    undo_stack_->push(tgl);
}

void Diagram::cmdConnectDevices(const ConnectionId& conn)
{
    auto x = new ConnectDevices(this, conn);
    undo_stack_->push(x);
}

void Diagram::cmdCreateComment(const QPointF& pos)
{
    auto add = new CreateComment(this, pos);
    undo_stack_->push(add);
}

void Diagram::cmdDisconnectDevices(const ConnectionId& conn)
{
    cmdDisconnectXlet(conn.sourceInfo());
}

void Diagram::cmdDuplicateItems(const SharedItemData& data)
{
    auto dup = new DuplicateItem(this, data);
    undo_stack_->push(dup);
}

void Diagram::cmdDuplicateSelection()
{
    auto dup = new DuplicateSelected(this);
    undo_stack_->push(dup);
}

void Diagram::cmdLockSelected()
{
    auto lock = new LockSelected(this);
    undo_stack_->push(lock);
}

void Diagram::cmdUnlockSelected()
{
    auto unlock = new UnlockSelected(this);
    undo_stack_->push(unlock);
}

void Diagram::cmdLock(SceneItemId id)
{
    auto lock = new LockItems(this, { id });
    undo_stack_->push(lock);
}

void Diagram::cmdUnlock(SceneItemId id)
{
    auto lock = new UnlockItems(this, { id });
    undo_stack_->push(lock);
}

void Diagram::cmdMirrorDevice(SceneItemId id)
{
    auto mirror = new MirrorDevice(this, id, ImageMirrorType::Horizontal);
    undo_stack_->push(mirror);
}

void Diagram::cmdMirrorSelected()
{
    auto mirror = new MirrorSelected(this, ImageMirrorType::Horizontal);
    undo_stack_->push(mirror);
}

void Diagram::cmdSelectAll()
{
    auto sel = new AddToSelected(this, item_scene_.idList());
    undo_stack_->push(sel);
}

void Diagram::cmdAddToSelection(const QRectF& sel)
{
    auto sel_devs = new AddToSelected(this, item_scene_.intersectedList(sel));
    undo_stack_->push(sel_devs);
}

void Diagram::cmdAddToSelection(const QList<QGraphicsItem*>& items)
{
    QList<SceneItemId> ids;
    for (auto x : items) {
        auto dev = qgraphicsitem_cast<SceneItem*>(x);
        if (dev)
            ids.push_back(dev->id());
    }

    auto add_sel = new AddToSelected(this, ids);
    undo_stack_->push(add_sel);
}

void Diagram::cmdSelectItems(const QRectF& sel)
{
    auto set_sel = new SetSelected(this, item_scene_.intersected(sel));
    undo_stack_->push(set_sel);
}

void Diagram::cmdSelectUnique(SceneItemId id)
{
    QSet<SceneItemId> ids { id };
    auto sel = new SetSelected(this, ids);
    undo_stack_->push(sel);
}

void Diagram::cmdDisconnectXlet(const XletInfo& xi)
{
    auto xconn = new DisconnectXlet(this, xi);
    undo_stack_->push(xconn);
}

void Diagram::cmdDistributeHSelected()
{
    int count = 0;
    qreal min_x = std::numeric_limits<qreal>::max();
    qreal max_x = std::numeric_limits<qreal>::lowest();
    std::vector<std::pair<SceneItemId, qreal>> sel_data;
    item_scene_.foreachSelectedItem([&count, &min_x, &max_x, &sel_data](const SceneItem* dev) {
        const auto pos = dev->pos();
        min_x = std::min(min_x, pos.x());
        max_x = std::max(max_x, pos.x());
        sel_data.push_back({ dev->id(), pos.x() });

        count++;
    });
    if (count < 3)
        return;

    qreal dist_wd = max_x - min_x;
    qreal step = dist_wd / (count - 1);
    std::sort(sel_data.begin(), sel_data.end(),
        [](const std::pair<SceneItemId, qreal>& a, const std::pair<SceneItemId, qreal>& b) { return a.second < b.second; });

    QHash<SceneItemId, QPointF> deltas;
    for (auto i = 0; i < sel_data.size(); i++) {
        auto id = sel_data[i].first;
        auto old_x = sel_data[i].second - min_x;
        auto new_x = step * i - old_x;
        deltas[id] = { new_x, 0 };
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdDistributeVSelected()
{
    int count = 0;
    qreal min_y = std::numeric_limits<qreal>::max();
    qreal max_y = std::numeric_limits<qreal>::lowest();
    std::vector<std::pair<SceneItemId, qreal>> sel_data;
    item_scene_.foreachSelectedItem([&count, &min_y, &max_y, &sel_data](const SceneItem* dev) {
        const auto pos = dev->pos();
        min_y = std::min(min_y, pos.y());
        max_y = std::max(max_y, pos.y());
        sel_data.push_back({ dev->id(), pos.y() });

        count++;
    });
    if (count < 3)
        return;

    qreal dist_height = max_y - min_y;
    qreal step = dist_height / (count - 1);
    std::sort(sel_data.begin(), sel_data.end(),
        [](const std::pair<SceneItemId, qreal>& a, const std::pair<SceneItemId, qreal>& b) { return a.second < b.second; });

    QHash<SceneItemId, QPointF> deltas;
    for (auto i = 0; i < sel_data.size(); i++) {
        auto id = sel_data[i].first;
        auto old_y = sel_data[i].second - min_y;
        auto new_y = step * i - old_y;
        deltas[id] = { 0, new_y };
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdMoveSelectedItemsBy(qreal dx, qreal dy)
{
    auto move_by = new MoveSelected(this, dx, dy);
    undo_stack_->push(move_by);
}

void Diagram::cmdMoveSelectedItemsFrom(const QPointF& from, const QPointF& to)
{
    if (!item_scene_.hasSelected())
        return;

    const auto delta = to - from;
    auto move_by = new MoveSelected(this, delta.x(), delta.y());
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignVSelected()
{
    auto sel_data = item_scene_.selectedDataList();
    if (sel_data.empty())
        return;

    qreal x = 0; // find middle x-position
    for (auto& data : sel_data) {
        x += data->pos().x();
    }
    x /= sel_data.size();

    QHash<SceneItemId, QPointF> deltas;
    for (auto& data : sel_data) {
        deltas.insert(data->id(), QPointF(x - data->pos().x(), 0));
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignHSelected()
{
    auto sel_data = item_scene_.selectedDataList();
    if (sel_data.empty())
        return;

    qreal y = 0; // find middle y-position
    for (auto& data : sel_data) {
        y += data->pos().y();
    }
    y /= sel_data.size();

    QHash<SceneItemId, QPointF> deltas;
    for (auto& data : sel_data) {
        deltas.insert(data->id(), QPointF(0, y - data->pos().y()));
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdCutSelected()
{
    auto cut = new CutSelected(this);
    undo_stack_->push(cut);
}

void Diagram::cmdPaste()
{
    auto paste = new PasteFromClipBuffer(this);
    undo_stack_->push(paste);
}

void Diagram::cmdPlaceInColumnSelected()
{
    int count = 0;
    qreal min_y = std::numeric_limits<qreal>::max();
    std::vector<std::pair<SceneItemId, const SceneItem*>> sel_data;
    item_scene_.foreachSelectedItem([&count, &min_y, &sel_data](const SceneItem* dev) {
        min_y = std::min(min_y, dev->y());
        sel_data.push_back({ dev->id(), dev });

        count++;
    });
    if (count < 2)
        return;

    std::sort(sel_data.begin(), sel_data.end(),
        [](const std::pair<SceneItemId, const SceneItem*>& a, const std::pair<SceneItemId, const SceneItem*>& b) {
            return a.second->y() < b.second->y();
        });

    qreal xpos = sel_data[0].second->x();
    qreal ypos = min_y;

    QHash<SceneItemId, QPointF> deltas;
    for (auto i = 1; i < sel_data.size(); i++) {
        auto dev = sel_data[i].second;
        auto prev_dev = sel_data[i - 1].second;
        ypos += prev_dev->boundingRect().height();
        deltas[dev->id()] = { xpos - dev->x(), ypos - dev->y() };
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdPlaceInRowSelected()
{
    int count = 0;
    qreal min_x = std::numeric_limits<qreal>::max();
    std::vector<std::pair<SceneItemId, const SceneItem*>> sel_data;
    item_scene_.foreachSelectedItem([&count, &min_x, &sel_data](const SceneItem* dev) {
        min_x = std::min(min_x, dev->x());
        sel_data.push_back({ dev->id(), dev });

        count++;
    });
    if (count < 2)
        return;

    std::sort(sel_data.begin(), sel_data.end(),
        [](const std::pair<SceneItemId, const SceneItem*>& a, const std::pair<SceneItemId, const SceneItem*>& b) {
            return a.second->x() < b.second->x();
        });

    qreal xpos = min_x;
    qreal ypos = sel_data[0].second->y();
    QHash<SceneItemId, QPointF> deltas;
    for (auto i = 1; i < sel_data.size(); i++) {
        auto dev = sel_data[i].second;
        auto prev_dev = sel_data[i - 1].second;
        xpos += prev_dev->boundingRect().width();
        deltas[dev->id()] = { xpos - dev->x(), ypos - dev->y() };
    }

    auto move_by = new MoveByItems(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn)
{
    auto recon = new ReconnectDevice(this, old_conn, new_conn);
    undo_stack_->push(recon);
}

SceneItem* Diagram::addItem(const SharedItemData& data)
{
    auto item = item_scene_.add(data);
    if (!item)
        return nullptr;

    connect(item, &SceneItem::addToFavorites, this, &Diagram::addToFavorites);
    connect(item, &SceneItem::duplicateDevice, this, &Diagram::cmdDuplicateItems);
    connect(item, &SceneItem::removeDevice, this, &Diagram::cmdRemoveItem);
    connect(item, &SceneItem::updateDevice, this, &Diagram::cmdUpdateItem);

    connect(item, &SceneItem::alignHorizontal, this, &Diagram::cmdAlignHSelected);
    connect(item, &SceneItem::alignVertical, this, &Diagram::cmdAlignVSelected);

    connect(item, &SceneItem::distributeHorizontal, this, &Diagram::cmdDistributeHSelected);
    connect(item, &SceneItem::distributeVertical, this, &Diagram::cmdDistributeVSelected);

    connect(item, &SceneItem::placeInColumn, this, &Diagram::cmdPlaceInColumnSelected);
    connect(item, &SceneItem::placeInRow, this, &Diagram::cmdPlaceInRowSelected);

    // move
    connect(item, &SceneItem::moveLower, this, &Diagram::cmdMoveLower);
    connect(item, &SceneItem::moveUpper, this, &Diagram::cmdMoveUpper);

    // lock
    connect(item, &SceneItem::lockSelected, this, &Diagram::cmdLockSelected);
    connect(item, &SceneItem::unlockSelected, this, &Diagram::cmdUnlockSelected);
    connect(item, &SceneItem::lock, this, &Diagram::cmdLock);
    connect(item, &SceneItem::unlock, this, &Diagram::cmdUnlock);

    // mirror
    connect(item, &SceneItem::mirrorSelected, this, &Diagram::cmdMirrorSelected);
    connect(item, &SceneItem::mirror, this, &Diagram::cmdMirrorDevice);

    emit sceneChanged();

    return item;
}

void Diagram::saveClickPos(const QPointF& pos)
{
    prev_move_pos_ = pos;
    prev_click_pos_ = pos;
}

bool Diagram::setItemData(const SharedItemData& data)
{
    if (!data)
        return false;

    auto dev = item_scene_.find(data->id());
    if (!dev) {
        qWarning() << "device not found:" << data->id();
        return false;
    }

    const bool title_update = (dev->itemData()->title() != data->title());

    auto battery_change = dev->itemData()->calcBatteryChange(*data);

    dev->setItemData(data);
    emit sceneChanged();
    emit deviceUpdated(dev->itemData());

    if (battery_change)
        emit batteryChanged(battery_change);

    if (title_update)
        emit deviceTitleUpdated(data->id(), data->title());

    updateConnectionPos(data->id());

    return true;
}

QList<SceneItemId> Diagram::duplicateSelected(DuplicatePolicy policy)
{
    QList<SceneItem*> dup_list;

    item_scene_.foreachItem([&dup_list](SceneItem* item) {
        if (item->isSelected())
            dup_list << item;
    });

    QList<SceneItemId> res;
    if (dup_list.empty())
        return res;

    for (auto src_dev : dup_list) {
        auto new_dev = addItem(src_dev->itemData());
        if (new_dev) {
            switch (new_dev->itemData()->category()) {
            case ItemCategory::Furniture:
                new_dev->moveBy(50, 0);
                break;
            default:
                new_dev->moveBy(20, 20);
                break;
            }

            res.push_back(new_dev->id());

            if (policy.select_new)
                new_dev->setSelected(true);

            if (policy.unselect_origin)
                src_dev->setSelected(false);
        }
    }

    return res;
}

void Diagram::setShowCables(bool value)
{
    show_cables_ = value;
    connections_->setVisible(value);
}

void Diagram::setShowPeople(bool value)
{
    item_scene_.foreachItem([value](SceneItem* item) {
        if (item //
            && item->itemData() //
            && item->itemData()->category() == ItemCategory::Human) {
            item->setVisible(value);
        }
    });
}

void Diagram::setShowFurniture(bool value)
{
    item_scene_.foreachItem([value](SceneItem* item) {
        if (item //
            && item->itemData() //
            && item->itemData()->category() == ItemCategory::Furniture) {
            item->setVisible(value);
        }
    });
}

void Diagram::setShowBackground(bool value)
{
    background_.setVisible(value);
    emit showBackgroundChanged(value);
}

bool Diagram::setBackground(const QString& path)
{
    if (background_.loadImage(path)) {
        emit sceneChanged();
        return true;
    } else
        return false;
}

void Diagram::clearBackground()
{
    if (!background_.isEmpty()) {
        background_.clear();
        emit sceneChanged();
    }
}

bool Diagram::loadJson(const QString& path)
{
    WARN() << path;

    connections_->showEditor(false);

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        WARN() << "can't open file:" << path;
        return false;
    }

    auto val = file.readAll();
    file.close();

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(val, &err);
    if (!doc.isObject()) {
        WARN() << "invalid JSON file: " << err.errorString();
        return false;
    }

    auto root = doc.object();

    clearAll();

    auto app = root.value(JSON_KEY_APP).toObject();
    if (!app.isEmpty()) {
        auto app_vers = app.value(JSON_KEY_VERSION).toString();
        if (!app_vers.isEmpty())
            qDebug() << "open document, created with PatchScene:" << app_vers;

        auto fmt_vers = app.value(JSON_KEY_FORMAT_VERSION).toInt();

        if (fmt_vers > app_file_format_version()) {
            WARN() << "the document was created with more recent version, then the current one, "
                      "some feature can be missing...";
        }

        if (fmt_vers != app_file_format_version())
            emit fileFormatVersionMismatch(fmt_vers, app_file_format_version());
    }

    // load devices
    auto devs = root.value(JSON_KEY_DEVICES);
    if (devs.isArray()) {
        auto arr = devs.toArray();
        for (const auto& j : arr)
            addItem(SceneItem::dataFromJson(j));
    }

    // load connections
    auto cons = root.value(JSON_KEY_CONNS);
    if (cons.isArray()) {
        auto arr = cons.toArray();
        for (const auto& j : arr) {
            auto conn_id = ConnectionId::fromJson(j);
            if (conn_id) {
                auto view_data = ConnectionViewData::fromJson(j.toObject().value(JSON_KEY_VIEW));
                connectDevices(*conn_id, view_data);
            }
        }
    }

    auto bg = root.value(JSON_KEY_BACKGROUND);
    if (!bg.isNull())
        background_.setFromJson(bg);

    auto meta = DiagramMeta::fromJson(root.value(JSON_KEY_META));
    if (meta) {
        meta_ = meta.value();
    } else {
        meta_ = DiagramMeta();
    }

    clearUndoStack();

    return true;
}

void Diagram::zoomIn()
{
    updateZoom(zoom_ * 3 / 2.0);
}

void Diagram::zoomOut()
{
    updateZoom(zoom_ * 2 / 3.0);
}

void Diagram::zoomFitBest()
{
    auto rect = graphics_scene_->bestFitRect();
    if (rect.isEmpty())
        return;

    fitRect(rect);
}

void Diagram::zoomFitSelected()
{
    auto rect = item_scene_.boundingSelectRect();
    if (rect.isEmpty())
        return;

    fitRect(rect);
}

void Diagram::setGridVisible(bool value)
{
    graphics_scene_->setGridVisible(value);
}

void Diagram::setScaleVisible(bool value)
{
    scale_->setVisible(value);
}

void Diagram::zoomNormal()
{
    updateZoom(1);
}

void Diagram::undo()
{
    undo_stack_->undo();
}

void Diagram::redo()
{
    undo_stack_->redo();
}

void Diagram::copySelected()
{
    auto sel_data = item_scene_.selectedDataList();
    if (sel_data.empty())
        return;

    clip_buffer_ = sel_data;
}

void Diagram::cutSelected()
{
    cmdCutSelected();
}

void Diagram::paste()
{
    cmdPaste();
}

void Diagram::printScheme() const
{
    QPrinter printer;
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        graphics_scene_->printDiagram(&printer);
    }
}

void Diagram::printScheme(QPrinter* printer) const
{
    graphics_scene_->printDiagram(printer);
}

void Diagram::renderToSvg(const QString& filename, const QString& title) const
{
    auto grid = graphics_scene_->gridVisible();
    if (grid)
        graphics_scene_->setGridVisible(false);

    QSvgGenerator svg_gen;
    svg_gen.setFileName(filename);
    svg_gen.setTitle(title);
    svg_gen.setDescription(tr("Generated with PatchScene"));
    auto svg_size = item_scene_.boundingRect().size();
    svg_gen.setSize(svg_size.toSize());
    svg_gen.setViewBox({ { 0, 0 }, svg_size });
    QPainter p(&svg_gen);

    graphics_scene_->renderDiagram(&p);

    if (grid)
        graphics_scene_->setGridVisible(true);

    p.end();
}

void Diagram::renderToPng(const QString& filename) const
{
    auto grid = graphics_scene_->gridVisible();
    if (grid)
        graphics_scene_->setGridVisible(false);

    auto img_size = item_scene_.boundingRect().size().toSize() * 4;
    QImage img(img_size, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    graphics_scene_->renderDiagram(&p);

    if (grid)
        graphics_scene_->setGridVisible(true);

    p.end();

    img.save(filename, "png");
}

void Diagram::clearAll()
{
    item_scene_.clear();

    QSignalBlocker conn_block(connections_);
    connections_->clear();
    background_.clear();

    undo_stack_->clear();

    emit sceneClearAll();
}

bool Diagram::scaleIsVisible() const
{
    return scale_->isVisible();
}

void Diagram::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
        auto numPixels = event->pixelDelta();
        auto numDegrees = event->angleDelta();

        qreal factor = 1;
        if (!numPixels.isNull()) {
            if (numPixels.y() > 0) {
                factor = PIXEL_ZOOM_FACTOR;
            } else {
                factor = 1 / PIXEL_ZOOM_FACTOR;
            }
        } else if (!numDegrees.isNull()) {
            if (numDegrees.y() > 0) {
                factor = ANGLE_ZOOM_FACTOR;
            } else {
                factor = 1 / ANGLE_ZOOM_FACTOR;
            }
        }

        updateZoom(zoom_ * factor);
    } else
        QGraphicsView::wheelEvent(event);
}

QJsonObject Diagram::toJson() const
{
    QJsonObject json;

    json[JSON_KEY_DEVICES] = item_scene_.toJson();

    QJsonArray cons;

    connections_->foreachConn([this, &cons](const ConnectionId& id, const ConnectionViewData& viewData) {
        if (item_scene_.checkConnection(id)) {
            auto obj = id.toJson();
            obj[JSON_KEY_VIEW] = viewData.toJson();
            cons.append(obj);
        } else {
            WARN() << "invalid connection" << id;
        }
    });

    json[JSON_KEY_CONNS] = cons;

    json[JSON_KEY_BACKGROUND] = background_.toJson();

    json[JSON_KEY_APP] = appInfoJson();
    json[JSON_KEY_META] = meta_.toJson();

    return json;
}

void Diagram::startSelectionAt(const QPoint& pos)
{
    selection_->setPos(mapToScene(pos));
    selection_->setRect({});
    selection_->setVisible(true);
}

void Diagram::startConnectionAt(const QPoint& pos)
{
    tmp_connection_->setLine(QLineF {});
    tmp_connection_->setPos(mapToScene(pos));
    tmp_connection_->setVisible(true);
}

void Diagram::drawConnectionTo(const QPoint& pos)
{
    tmp_connection_->setLine(QLineF(QPointF {}, tmp_connection_->mapFromScene(mapToScene(pos))));
}

void Diagram::drawSelectionTo(const QPoint& pos)
{
    QRectF rect(QPointF {}, selection_->mapFromScene(mapToScene(pos)));
    selection_->setRect(rect.normalized());
}

void Diagram::showConnectionEditor()
{
    switch (state_machine_.state()) {
    case DiagramState::Init: // normal mode
    case DiagramState::EditConnection: // update editor
        state_machine_.setState(DiagramState::EditConnection);
        connections_->showEditor(true);
        break;
    case DiagramState::MoveItem:
    case DiagramState::ConnectDevice:
    case DiagramState::SelectItem:
    case DiagramState::SelectionRect:
        break;
    }
}

void Diagram::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        return emit customContextMenuRequested(event->pos());
    }

    switch (state_machine_.state()) {
    case DiagramState::Init: {
        auto elems = items(event->pos());
        bool item_found = std::any_of(elems.begin(), elems.end(),
            [](QGraphicsItem* x) { return qgraphicsitem_cast<SceneItem*>(x); });

        if (item_found) { // click on the item
            auto xlet = hoverDeviceXlet(elems, event->pos());

            if (xlet) { // click on xlet
                bool disconnect = event->modifiers().testFlag(Qt::AltModifier);
                if (disconnect) { // remove xlet connection
                    state_machine_.setState(DiagramState::Init);
                    cmdDisconnectXlet(xlet->first);
                } else { // connection start
                    state_machine_.setState(DiagramState::ConnectDevice);
                    startConnectionAt(event->pos());
                    conn_begin_ = xlet;
                }

                return; //

            } else if (event->modifiers().testFlag(Qt::ControlModifier)) { // add/remove to/from selection
                cmdToggleSelected(elems);
            } else if (event->modifiers().testFlag(Qt::ShiftModifier)) { // add to selection
                cmdAddToSelection(elems);
            } else if (event->modifiers().testFlag(Qt::AltModifier))
                selectBottomDevice(elems);
            else
                selectTopDevice(elems);

            if (graphics_scene_->selectedItems().empty()) {
                state_machine_.setState(DiagramState::Init);
            } else {
                saveClickPos(event->position());
                state_machine_.setState(DiagramState::SelectItem);
                connections_->unselectAll();
            }

            event->accept();
        } else if (
            elems.size() > 0
            && elems[0]) { // click on other elements
            QGraphicsView::mousePressEvent(event);
        }

        if (!event->isAccepted() || !item_found) { // unhandled item click or empty space click
            connections_->unselectAll();
            startSelectionAt(event->pos());
            state_machine_.setState(DiagramState::SelectionRect);
        }
    } break;
    case DiagramState::ConnectDevice: {
        tmp_connection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
    } break;
    case DiagramState::SelectionRect: {
        selection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
    } break;
    case DiagramState::EditConnection: {
        QGraphicsView::mousePressEvent(event);
        if (!event->isAccepted()) {
            state_machine_.setState(DiagramState::Init);
            connections_->showEditor(false);
        }
    } break;
    default:
        state_machine_.setState(DiagramState::Init);
        break;
    }
}

void Diagram::mouseMoveEvent(QMouseEvent* event)
{
    switch (state_machine_.state()) {
    case DiagramState::SelectionRect: {
        drawSelectionTo(event->pos());
        event->accept();
    } break;
    case DiagramState::SelectItem: {
        state_machine_.setState(DiagramState::MoveItem);
    } break;
    case DiagramState::MoveItem: {
        auto delta = (event->position() - prev_move_pos_) / zoom_;
        moveSelectedItemsBy(delta.x(), delta.y());
        prev_move_pos_ = event->position();
    } break;
    case DiagramState::ConnectDevice:
        drawConnectionTo(event->pos());
        break;
    case DiagramState::EditConnection:
        event->accept();
        QGraphicsView::mouseMoveEvent(event);
        // WARN() << "connection edit move";
        break;
    default:
        QGraphicsView::mouseMoveEvent(event);
        break;
    }
}

void Diagram::mouseReleaseEvent(QMouseEvent* event)
{
    switch (state_machine_.state()) {
    case DiagramState::MoveItem: { // finish item moving
        state_machine_.setState(DiagramState::Init);

        auto dest_pos = mapToScene(event->position().toPoint());
        auto src_pos = mapToScene(prev_click_pos_.toPoint());
        auto delta = src_pos - dest_pos;

        item_scene_.foreachItem([delta](SceneItem* dev) {
            if (dev->isSelected() && !dev->isLocked())
                dev->moveBy(delta.x(), delta.y());
        });

        cmdMoveSelectedItemsFrom(src_pos, dest_pos);
    } break;
    case DiagramState::SelectionRect: { // finish selection
        auto bbox = selection_->mapRectToScene(selection_->rect());
        if (event->modifiers().testFlag(Qt::ShiftModifier))
            cmdAddToSelection(bbox);
        else
            cmdSelectItems(bbox);

        selection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
    } break;
    case DiagramState::ConnectDevice: { // finish connection
        tmp_connection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);

        auto conn_end = hoverDeviceXlet(items(event->pos()), event->pos());
        if (conn_end && conn_begin_) {
            if (event->modifiers().testFlag(Qt::ShiftModifier)
                && conn_end->first.id() == conn_begin_->first.id()
                && conn_end->first.type() == conn_begin_->first.type()) { // reconnect to other xlet of same device

                auto prev_conn = connections_->findByXlet(conn_begin_->first);
                if (prev_conn) {
                    auto new_conn = prev_conn->connectionInfo();
                    if (new_conn.first.setEndPoint(conn_end->first)) {
                        cmdReconnectDevice(prev_conn->connectionInfo(), new_conn);
                    }
                }
            } else {
                if (!connections_->checkConnection(conn_begin_.value(), conn_end.value()))
                    return;

                auto& d0 = conn_begin_->second;
                auto& d1 = conn_end->second;
                auto c0 = conn_begin_->first;
                auto c1 = conn_end->first;

                if (c0.type() == c1.type()) {
                    if (c0.isInlet() && !d0.isBidirect() && d1.isBidirect())
                        std::swap(c0, c1);

                    if (c0.isOutlet() && d0.isBidirect() && !d1.isBidirect())
                        std::swap(c0, c1);
                }

                auto conn = ConnectionId::fromXletPair(c0, c1);
                if (conn) {
                    cmdConnectDevices(conn.value());
                }
            }
        }

        conn_begin_ = {};
    } break;
    case DiagramState::EditConnection:
        QGraphicsView::mouseReleaseEvent(event);
        break;
    default:
        state_machine_.setState(DiagramState::Init);
        break;
    }
}

void Diagram::contextMenuEvent(QContextMenuEvent* event)
{
    QGraphicsView::contextMenuEvent(event);
    if (event->isAccepted())
        return;

    auto pos = event->pos();
    event->accept();

    auto add_act = new QAction(tr("&Add device"), this);
    connect(add_act, &QAction::triggered, this,
        [this, pos]() { cmdCreateDevice(mapToScene(pos)); });

    auto add_comment = new QAction(tr("&Add comment"), this);
    connect(add_comment, &QAction::triggered, this,
        [this, pos]() { cmdCreateComment(mapToScene(pos)); });

    QMenu menu(this);
    menu.addAction(add_act);
    menu.addAction(add_comment);

    background_.addToContextMenu(menu);

    if (item_scene_.selectedCount() >= 2) {
        menu.addSeparator();

        auto hor_align = new QAction(tr("Align &horizontal"), this);
        connect(hor_align, SIGNAL(triggered(bool)), this, SLOT(cmdAlignHSelected()));
        menu.addAction(hor_align);

        auto ver_align = new QAction(tr("Align &vertical"), this);
        connect(ver_align, SIGNAL(triggered(bool)), this, SLOT(cmdAlignVSelected()));
        menu.addAction(ver_align);

        auto place_hor = new QAction(tr("Place in row"), &menu);
        connect(place_hor, SIGNAL(triggered(bool)), this, SLOT(cmdPlaceInRowSelected()));
        menu.addAction(place_hor);

        auto place_ver = new QAction(tr("Place in column"), &menu);
        connect(place_ver, SIGNAL(triggered(bool)), this, SLOT(cmdPlaceInColumnSelected()));
        menu.addAction(place_ver);

        if (item_scene_.selectedCount() >= 3) {
            menu.addSeparator();

            auto distrib_hor = new QAction(tr("Distribute horizontal"), &menu);
            connect(distrib_hor, SIGNAL(triggered(bool)), this, SIGNAL(distributeHorizontal()));
            menu.addAction(distrib_hor);

            auto distrib_ver = new QAction(tr("Distribute vertical"), &menu);
            connect(distrib_ver, SIGNAL(triggered(bool)), this, SIGNAL(distributeVertical()));
            menu.addAction(distrib_ver);
        }
    }

    menu.exec(event->globalPos());
}

void Diagram::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->formats().contains("text/plain")) {
        auto data = event->mimeData()->data("text/plain");
        if (!data.isEmpty())
            event->acceptProposedAction();
    } else if (event->mimeData()->hasUrls()) {
        auto files = event->mimeData()->urls();
        if (files.size() == 1) {
            auto fname = files.front().fileName();

            if (fname.endsWith(".svg", Qt::CaseInsensitive)
                || fname.endsWith(".png", Qt::CaseInsensitive)
                || fname.endsWith(".jpg", Qt::CaseInsensitive)
                || fname.endsWith(".jpeg", Qt::CaseInsensitive))
                event->acceptProposedAction();
        } else {
            qDebug() << __FUNCTION__ << "single image file expected, got:" << files;
        }
    } else {
        qDebug() << __FUNCTION__ << "unsupported MIME type:" << event->mimeData()->formats();
    }
}

void Diagram::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void Diagram::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->formats().contains("text/plain")) {
        if (dropJson(mapToScene(event->position().toPoint()), event->mimeData()->data("text/plain")))
            event->acceptProposedAction();
    } else if (event->mimeData()->hasUrls()) {
        auto files = event->mimeData()->urls();
        if (files.size() == 1) {
            if (setBackground(files.front().toLocalFile()))
                event->acceptProposedAction();
        } else {
            qDebug() << __FUNCTION__ << "single image file expected, got:" << files;
        }
    }
}

void Diagram::keyPressEvent(QKeyEvent* event)
{
    auto mods = event->modifiers();

    int MOVE_STEP = 2;
    if (mods.testFlags(Qt::ControlModifier))
        MOVE_STEP = 50;
    else if (mods.testFlags(Qt::ShiftModifier))
        MOVE_STEP = 10;

    if (event->key() == Qt::Key_Backspace && event->modifiers().testFlag(Qt::ControlModifier)) {
        cmdRemoveSelected();
    } else if (event->key() == Qt::Key_Down) {
        cmdMoveSelectedItemsBy(0, MOVE_STEP);
    } else if (event->key() == Qt::Key_Up) {
        cmdMoveSelectedItemsBy(0, -MOVE_STEP);
    } else if (event->key() == Qt::Key_Left) {
        cmdMoveSelectedItemsBy(-MOVE_STEP, 0);
    } else if (event->key() == Qt::Key_Right) {
        cmdMoveSelectedItemsBy(MOVE_STEP, 0);
    } else
        QGraphicsView::keyPressEvent(event);
}

bool Diagram::viewportEvent(QEvent* event)
{
    switch (event->type()) {
#ifndef QT_NO_GESTURES
    case QEvent::NativeGesture: {
        auto nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge) {
            switch (nge->gestureType()) {
            case Qt::ZoomNativeGesture: { // mac two-finger zoom
                updateZoom(zoom_ * (1 + nge->value()));
                event->accept();
                return true;
            } break;
            case Qt::SmartZoomNativeGesture: { // smart zoom on two-finger double tap
                if (nge->value() == 0) {
                    zoomNormal();
                    centerOn(0, 0);
                } else {
                    zoomFitBest();
                }

                event->accept();
                return true;
            } break;
            default:
                break;
            }
        }
    } break;
#endif

#ifndef Q_OS_DARWIN
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd: {
        auto touchEvent = dynamic_cast<QTouchEvent*>(event);
        auto touchPoints = touchEvent->points();
        if (touchPoints.count() == 2) {
            // determine scale factor
            const auto& p0 = touchPoints.first();
            const auto& p1 = touchPoints.last();
            qreal scale_factor = //
                QLineF(p0.position(), p1.position()).length()
                / QLineF(p0.pressPosition(), p1.pressPosition()).length();
        }
        return true;
    }
#endif
    default:
        break;
    }

    return QGraphicsView::viewportEvent(event);
}

void Diagram::selectTopDevice(const QList<QGraphicsItem*>& devs)
{
    for (auto x : devs) {
        auto dev = qgraphicsitem_cast<SceneItem*>(x);
        if (dev)
            return cmdSelectUnique(dev->id());
    }
}

void Diagram::selectBottomDevice(const QList<QGraphicsItem*>& devs)
{
    for (auto it = devs.crbegin(); it != devs.crend(); ++it) {
        auto dev = qgraphicsitem_cast<SceneItem*>(*it);
        if (dev)
            return cmdSelectUnique(dev->id());
    }
}

std::optional<std::pair<XletInfo, XletData>> Diagram::hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt)
{
    DeviceXlet* xlet = nullptr;
    for (auto x : devs) {
        xlet = qgraphicsitem_cast<DeviceXlet*>(x);
        if (xlet)
            break;
    }

    if (!xlet)
        return {};
    else
        return std::make_pair(xlet->xletInfo(), xlet->xletData());
}

bool Diagram::connectDevices(const ConnectionId& id, std::optional<ConnectionViewData> viewData)
{
    auto conn = connections_->add(id);
    if (conn) {
        if (!viewData)
            updateConnectionStyle(conn);
        else
            conn->setViewData(*viewData);

        updateConnectionPos(conn);
        emit sceneChanged();
        return true;
    } else {
        WARN() << "can't connect:" << id;
        return false;
    }
}

bool Diagram::disconnectDevices(const ConnectionId& id)
{
    if (connections_->remove(id)) {
        emit sceneChanged();
        return true;
    } else {
        WARN() << "can't remove connection:" << id;
        return false;
    }
}

void Diagram::moveSelectedItemsBy(qreal dx, qreal dy)
{
    bool notify = false;

    {
        DiagramUpdatesBlocker ub(this);
        if (item_scene_.moveSelectedBy(dx, dy)) { // O(N)
            notify = true;

            // O(N)
            item_scene_.foreachSelectedData([this](const SharedItemData& data) {
                if (!data->isLocked())
                    updateConnectionPos(data->id());
            });
        }
    }

    if (notify)
        emit sceneChanged();
}

void Diagram::moveItemsBy(const QHash<SceneItemId, QPointF>& deltas)
{
    bool notify = false;

    {
        DiagramUpdatesBlocker ub(this);

        if (item_scene_.moveBy(deltas)) {
            notify = true;

            for (auto it = deltas.begin(); it != deltas.end(); ++it)
                updateConnectionPos(it.key());
        }
    }

    if (notify)
        emit sceneChanged();
}

void Diagram::clearClipBuffer()
{
    clip_buffer_.clear();
}

const QList<SharedItemData>& Diagram::clipBuffer() const
{
    return clip_buffer_;
}

void Diagram::setClipBuffer(const QList<SharedItemData>& data)
{
    clip_buffer_ = data;
}

QImage Diagram::toImage() const
{
    return graphics_scene_->renderToImage(4);
}

std::pair<QByteArray, QSize> Diagram::toSvg() const
{
    QBuffer buf;
    if (!buf.open(QBuffer::ReadWrite)) {
        qWarning() << __FUNCTION__ << "can't open buffer";
        return {};
    }

    QSvgGenerator svg_gen;
    svg_gen.setOutputDevice(&buf);

    auto items_bbox = graphics_scene_->bestFitRect().toRect();

    svg_gen.setSize(items_bbox.size());
    svg_gen.setViewBox(QRect { 0, 0, items_bbox.width(), items_bbox.height() });
    svg_gen.setTitle("PatchScheme connection diagram");
    svg_gen.setResolution(72);
    svg_gen.setDescription(QString("create with PatchScene v%1").arg(app_version()));

    QPainter painter(&svg_gen);
    graphics_scene_->renderDiagram(&painter, items_bbox);
    painter.end();

    return { buf.data(), items_bbox.size() };
}

bool Diagram::gridIsVisible() const
{
    return graphics_scene_->gridVisible();
}

void Diagram::clearUndoStack()
{
    if (undo_stack_)
        undo_stack_->clear();
}

void Diagram::updateZoom(qreal zoom)
{
    if (zoom < MIN_ZOOM && zoom_ == MIN_ZOOM)
        return;

    if (zoom > MAX_ZOOM && zoom_ == MAX_ZOOM)
        return;

    zoom_ = qBound(MIN_ZOOM, zoom, MAX_ZOOM);
    setTransform(QTransform::fromScale(zoom_, zoom_));
    emit zoomChanged(zoom_);
}

bool Diagram::dropJson(const QPointF& pos, const QByteArray& json)
{
    SharedItemData data(new ItemData(SCENE_ITEM_NULL_ID));
    if (!data->setJson(json)) {
        qWarning() << "can't set JSON";
        return false;
    }

    data->setPos(pos);
    cmdDuplicateItems(data);
    return true;
}

QJsonValue Diagram::appInfoJson()
{
    QJsonObject obj;

    obj[JSON_KEY_VERSION] = app_version();
    obj[JSON_KEY_VERSION_MAJOR] = app_version_major();
    obj[JSON_KEY_VERSION_MINOR] = app_version_minor();
    obj[JSON_KEY_VERSION_PATCH] = app_version_patch();
    obj[JSON_KEY_VERSION_GIT] = app_git_version();
    obj[JSON_KEY_FORMAT_VERSION] = app_file_format_version();

    return obj;
}

void Diagram::fitRect(const QRectF& rect)
{
    auto view_rect = viewport()->rect();
    auto zoom_x = view_rect.width() / rect.width();
    auto zoom_y = view_rect.height() / rect.height();
    auto zoom_min = std::min(zoom_x, zoom_y);

    if (MIN_ZOOM <= zoom_min && zoom_min <= MAX_ZOOM) {
        fitInView(rect, Qt::KeepAspectRatio);
        zoom_ = zoom_min;
        emit zoomChanged(zoom_);

    } else if (zoom_min < MIN_ZOOM) {
        updateZoom(MIN_ZOOM);
    } else { // zoom_min > MAX_ZOOM
        auto rect_scale = zoom_min / MAX_ZOOM;
        auto dx = (rect_scale - 1) * rect.width() * 0.5;
        auto dy = (rect_scale - 1) * rect.height() * 0.5;
        fitInView(rect.adjusted(-dx, -dy, dx, dy), Qt::KeepAspectRatio);
        zoom_ = MAX_ZOOM;
        emit zoomChanged(zoom_);
    }
}

QHash<ConnectionId, ConnectionViewData> Diagram::findSelectedConnections() const
{
    QHash<ConnectionId, ConnectionViewData> res;

    for (auto id : item_scene_.selectedIdList()) {
        for (auto& data : connections_->findConnectionsData(id)) {
            res.insert(data.first, data.second);
        }
    }

    return res;
}
