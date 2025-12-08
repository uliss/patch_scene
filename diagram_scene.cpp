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
#include "diagram_scene.h"
#include "app_version.h"
#include "diagram_meta.h"
#include "diagram_updates_blocker.h"
#include "logging.hpp"
#include "scene_connections.h"
#include "scene_item.h"
#include "undo_commands.h"

#include <QFile>
#include <QGraphicsSceneMouseEvent>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMimeData>
#include <QPainter>
#include <QPrinter>

using namespace ceam;

namespace {

constexpr qreal ZVALUE_CONN = 100;
constexpr qreal ZVALUE_BACKGROUND = -200;
constexpr qreal ZVALUE_LIVE_CONN = 16000;
constexpr qreal ZVALUE_SELECTION = 32000;

constexpr const char* JSON_KEY_APP = "application";
constexpr const char* JSON_KEY_BACKGROUND = "background";
constexpr const char* JSON_KEY_CONNS = "connections";
constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_VIEW = "view";
constexpr const char* JSON_KEY_FORMAT_VERSION = "format-version";
constexpr const char* JSON_KEY_VERSION = "version";
constexpr const char* JSON_KEY_VERSION_GIT = "version-git";
constexpr const char* JSON_KEY_VERSION_MAJOR = "version-major";
constexpr const char* JSON_KEY_VERSION_MINOR = "version-minor";
constexpr const char* JSON_KEY_VERSION_PATCH = "version-patch";

} // namespace

#define SIGNAL_PASS2(member, type, from, to) connect(member, &type ::from, this, &DiagramScene ::to)
#define SIGNAL_PASS(member, type, name) SIGNAL_PASS2(member, type, name, name)

DiagramScene::DiagramScene(int w, int h, QObject* parent)
    : QGraphicsScene { parent }
{
    setSceneRect(-w / 2, -h / 2, w, h);

    initItemScene();
    initSelectionRect();
    initSceneConnections();
    initLiveConnection();
    initUndoStack();
    initSceneBackground();

    conn_database_.initDefault();
}

void DiagramScene::setGridVisible(bool value)
{
    grid_visible_ = value;
    update();
}

void DiagramScene::setCacheMode(QGraphicsItem::CacheMode mode)
{
    for (auto x : items())
        x->setCacheMode(mode);
}

void DiagramScene::renderDiagram(QPainter* painter, const QRect& rect)
{
    QSignalBlocker db(this);

    const auto old_rect = sceneRect();
    const auto new_rect = rect.isNull() ? bestFitRect() : rect;

    auto selected = selectedItems();
    clearSelection();

    setSceneRect(new_rect);
    setCacheMode(QGraphicsItem::NoCache);

    render(painter);

    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    setSceneRect(old_rect);

    for (auto& x : selected)
        x->setSelected(true);
}

QImage DiagramScene::renderToImage(qreal scale)
{
    auto rect = bestFitRect();

    QImage image(rect.size().toSize() * scale, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    renderDiagram(&painter, rect.toRect());
    painter.end();
    return image;
}

void DiagramScene::printDiagram(QPrinter* printer)
{
    QPainter painter(printer);
    painter.setRenderHint(QPainter::Antialiasing);
    renderDiagram(&painter);
    painter.end();
}

QRectF DiagramScene::bestFitRect() const
{
    return itemsBoundingRect();
}

size_t DiagramScene::connectionCount() const
{
    return connections_ ? connections_->count() : 0;
}

std::optional<std::pair<XletInfo, XletData>> DiagramScene::hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPointF& pt)
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

QJsonValue DiagramScene::appInfoJson()
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

SceneItem* DiagramScene::addSceneItem(const SharedItemData& data)
{
    auto item = itemScene().add(data);
    if (!item)
        return nullptr;

    // signal transfer
    SIGNAL_PASS(item, SceneItem, addToFavorites);

    // handle slot
    connect(item, &SceneItem::duplicateDevice, this, &DiagramScene::cmdDuplicateItem);
    connect(item, &SceneItem::removeDevice, this, &DiagramScene::cmdRemoveItem);
    connect(item, &SceneItem::updateDevice, this, &DiagramScene::cmdUpdateItem);

    // align
    connect(item, &SceneItem::alignHorizontal, this, &DiagramScene::cmdAlignHSelected);
    connect(item, &SceneItem::alignVertical, this, &DiagramScene::cmdAlignVSelected);

    // distribute
    connect(item, &SceneItem::distributeHorizontal, this, &DiagramScene::cmdDistributeHSelected);
    connect(item, &SceneItem::distributeVertical, this, &DiagramScene::cmdDistributeVSelected);

    // place
    connect(item, &SceneItem::placeInColumn, this, &DiagramScene::cmdPlaceInColumnSelected);
    connect(item, &SceneItem::placeInRow, this, &DiagramScene::cmdPlaceInRowSelected);

    // move
    connect(item, &SceneItem::moveLower, this, &DiagramScene::cmdMoveLower);
    connect(item, &SceneItem::moveUpper, this, &DiagramScene::cmdMoveUpper);

    // lock
    connect(item, &SceneItem::lockSelected, this, &DiagramScene::cmdLockSelected);
    connect(item, &SceneItem::unlockSelected, this, &DiagramScene::cmdUnlockSelected);
    connect(item, &SceneItem::lock, this, &DiagramScene::cmdLock);
    connect(item, &SceneItem::unlock, this, &DiagramScene::cmdUnlock);

    // mirror
    connect(item, &SceneItem::mirrorSelected, this, &DiagramScene::cmdMirrorSelected);
    connect(item, &SceneItem::mirror, this, &DiagramScene::cmdMirrorDevice);

    emit sceneChanged();

    return item;
}

bool DiagramScene::removeSceneItem(SceneItemId id)
{
    auto data = itemScene().remove(id);
    if (data) {
        connections()->removeAll(id);
        emit sceneChanged();
    }

    return true;
}

bool DiagramScene::setItemData(const SharedItemData& data)
{
    if (!data)
        return false;

    auto item = itemScene().find(data->id());
    if (!item) {
        WARN() << "item not found:" << data->id();
        return false;
    }

    const bool title_update = (item->itemData()->title() != data->title());

    auto battery_change = item->itemData()->calcBatteryChange(*data);

    item->setItemData(data);
    emit sceneChanged();
    emit deviceUpdated(item->itemData());

    if (battery_change)
        emit batteryChanged(battery_change);

    if (title_update)
        emit deviceTitleUpdated(data->id(), data->title());

    updateConnectionPos(data->id());

    return true;
}

bool DiagramScene::connectDevices(const ConnectionId& id, std::optional<ConnectionViewData> viewData)
{
    auto src = item_scene_.findData(id.source());
    if (!src) {
        WARN() << "connection source not found: " << id.source();
        return false;
    }

    if (id.sourceIndex() >= src->outputs().size()) {
        WARN() << "invalid source outlet index: " << id.sourceIndex();
        return false;
    }

    auto dest = item_scene_.findData(id.destination());
    if (!dest) {
        WARN() << "connection destination not found: " << id.destination();
        return false;
    }

    if (id.destinationIndex() >= dest->inputs().size()) {
        WARN() << "invalid destination inlet index: " << id.destinationIndex();
        return false;
    }

    auto conn = connections()->add(id);
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

bool DiagramScene::disconnectDevices(const ConnectionId& id)
{
    if (connections()->remove(id)) {
        emit sceneChanged();
        return true;
    } else {
        WARN() << "can't remove connection:" << id;
        return false;
    }
}

QList<SceneItemId> DiagramScene::duplicateSelected(DuplicatePolicy policy)
{
    QList<SceneItem*> dup_list;

    itemScene().foreachItem([&dup_list](SceneItem* item) {
        if (item->isSelected())
            dup_list << item;
    });

    QList<SceneItemId> res;
    if (dup_list.empty())
        return res;

    for (auto src_item : dup_list) {
        auto new_item = addSceneItem(src_item->itemData());
        if (new_item) {
            switch (new_item->itemData()->category()) {
            case ItemCategory::Furniture:
                new_item->moveBy(50, 0);
                break;
            default:
                new_item->moveBy(20, 20);
                break;
            }

            res.push_back(new_item->id());

            if (policy.select_new)
                new_item->setSelected(true);

            if (policy.unselect_origin)
                src_item->setSelected(false);
        }
    }

    return res;
}

void DiagramScene::drawLiveConnectionTo(const QPointF& pos)
{
    tmp_connection_->setLine(QLineF(QPointF {}, tmp_connection_->mapFromScene(pos)));
}

void DiagramScene::startLiveConnectionAt(const QPointF& pos)
{
    tmp_connection_->setLine(QLineF {});
    tmp_connection_->setPos(pos);
    tmp_connection_->setVisible(true);
}

void DiagramScene::selectLowestItem(const QList<QGraphicsItem*>& devs)
{
    for (auto it = devs.crbegin(); it != devs.crend(); ++it) {
        auto item = qgraphicsitem_cast<SceneItem*>(*it);
        if (item)
            return cmdSelectUnique(item->id());
    }
}

QHash<ConnectionId, ConnectionViewData> DiagramScene::findSelectedConnections() const
{
    QHash<ConnectionId, ConnectionViewData> res;

    for (auto id : item_scene_.selectedIdList()) {
        for (auto& data : connections()->findConnectionsData(id)) {
            res.insert(data.first, data.second);
        }
    }

    return res;
}

void DiagramScene::moveItemsBy(const QHash<SceneItemId, QPointF>& deltas)
{
    bool notify = false;

    {
        DiagramSceneUpdatesBlocker ub(this);

        if (itemScene().moveBy(deltas)) {
            notify = true;

            for (auto it = deltas.begin(); it != deltas.end(); ++it)
                updateConnectionPos(it.key());
        }
    }

    if (notify)
        emit sceneChanged();
}

void DiagramScene::cmdAddToSelection(const QList<QGraphicsItem*>& items)
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

void DiagramScene::cmdAddToSelection(const QRectF& sel)
{
    auto sel_devs = new AddToSelected(this, item_scene_.intersectedList(sel));
    undo_stack_->push(sel_devs);
}

void DiagramScene::cmdAlignHSelected()
{
    auto sel_data = itemScene().selectedDataList();
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

void DiagramScene::cmdAlignVSelected()
{
    auto sel_data = itemScene().selectedDataList();
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

void DiagramScene::cmdConnectDevices(const ConnectionId& conn)
{
    auto x = new ConnectDevices(this, conn);
    undo_stack_->push(x);
}

void DiagramScene::cmdCreateComment(const QPointF& pos)
{
    auto add = new CreateComment(this, pos, tr("Comment"));
    undo_stack_->push(add);
}

void DiagramScene::cmdCreateDevice(const QPointF& pos)
{
    auto add = new CreateDevice(this, pos);
    undo_stack_->push(add);
}

void DiagramScene::cmdDisconnectDevices(const ConnectionId& conn)
{
    cmdDisconnectXlet(conn.sourceInfo());
}

void DiagramScene::cmdDisconnectXlet(const XletInfo& xi)
{
    auto xconn = new DisconnectXlet(this, xi);
    undo_stack_->push(xconn);
}

void DiagramScene::cmdDistributeHSelected()
{
    int count = 0;
    qreal min_x = std::numeric_limits<qreal>::max();
    qreal max_x = std::numeric_limits<qreal>::lowest();
    std::vector<std::pair<SceneItemId, qreal>> sel_data;
    itemScene().foreachSelectedItem([&count, &min_x, &max_x, &sel_data](const SceneItem* dev) {
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

void DiagramScene::cmdDistributeVSelected()
{
    int count = 0;
    qreal min_y = std::numeric_limits<qreal>::max();
    qreal max_y = std::numeric_limits<qreal>::lowest();
    std::vector<std::pair<SceneItemId, qreal>> sel_data;
    itemScene().foreachSelectedItem([&count, &min_y, &max_y, &sel_data](const SceneItem* dev) {
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

void DiagramScene::cmdDuplicateSelected()
{
    auto dup = new DuplicateSelected(this);
    undo_stack_->push(dup);
}

void DiagramScene::cmdMoveSelectedItemsBy(qreal dx, qreal dy)
{
    auto move_by = new MoveSelected(this, dx, dy);
    undo_stack_->push(move_by);
}

void DiagramScene::cmdDuplicateItem(const SharedItemData& data)
{
    auto dup = new DuplicateItem(this, data);
    undo_stack_->push(dup);
}

void DiagramScene::cmdMoveSelectedItemsFrom(const QPointF& from, const QPointF& to)
{
    if (!itemScene().hasSelected())
        return;

    const auto delta = to - from;
    auto move_by = new MoveSelected(this, delta.x(), delta.y());
    undo_stack_->push(move_by);
}

void DiagramScene::cmdPlaceInColumnSelected()
{
    int count = 0;
    qreal min_y = std::numeric_limits<qreal>::max();
    std::vector<std::pair<SceneItemId, const SceneItem*>> sel_data;
    itemScene().foreachSelectedItem([&count, &min_y, &sel_data](const SceneItem* dev) {
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

void DiagramScene::cmdPlaceInRowSelected()
{
    int count = 0;
    qreal min_x = std::numeric_limits<qreal>::max();
    std::vector<std::pair<SceneItemId, const SceneItem*>> sel_data;
    itemScene().foreachSelectedItem([&count, &min_x, &sel_data](const SceneItem* dev) {
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

void DiagramScene::cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn)
{
    auto recon = new ReconnectDevice(this, old_conn, new_conn);
    undo_stack_->push(recon);
}

void DiagramScene::cmdRemoveItem(const SharedItemData& data)
{
    auto rem = new RemoveItem(this, data);
    undo_stack_->push(rem);
}

void DiagramScene::cmdRemoveSelected()
{
    auto del_sel = new RemoveSelected(this);
    undo_stack_->push(del_sel);
}

void DiagramScene::cmdSelectItems(const QRectF& sel)
{
    auto set_sel = new SetSelected(this, item_scene_.intersected(sel));
    undo_stack_->push(set_sel);
}

void DiagramScene::cmdSelectUnique(SceneItemId id)
{
    QSet<SceneItemId> ids { id };
    auto sel = new SetSelected(this, ids);
    undo_stack_->push(sel);
}

void DiagramScene::cmdToggleSelected(const QList<QGraphicsItem*>& items)
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

void DiagramScene::cmdUpdateItem(const SharedItemData& data)
{
    if (!data)
        return;

    auto up = new UpdateDeviceData(this, data);
    undo_stack_->push(up);
}

void DiagramScene::cmdMoveLower(const SharedItemData& data)
{
    auto move = new MoveLower(this, data->id());
    if (move->isObsolete()) {
        delete move;
        return;
    }

    undo_stack_->push(move);
}

void DiagramScene::cmdMoveUpper(const SharedItemData& data)
{
    auto move = new MoveUpper(this, data->id());
    if (move->isObsolete()) {
        delete move;
        return;
    }
    undo_stack_->push(move);
}

void DiagramScene::cmdLockSelected()
{
    auto lock = new LockSelected(this);
    undo_stack_->push(lock);
}

void DiagramScene::cmdUnlockSelected()
{
    auto unlock = new UnlockSelected(this);
    undo_stack_->push(unlock);
}

void DiagramScene::cmdLock(SceneItemId id)
{
    auto lock = new LockItems(this, { id });
    undo_stack_->push(lock);
}

void DiagramScene::cmdUnlock(SceneItemId id)
{
    auto lock = new UnlockItems(this, { id });
    undo_stack_->push(lock);
}

void DiagramScene::cmdMirrorDevice(SceneItemId id)
{
    auto mirror = new MirrorDevice(this, id, ImageMirrorType::Horizontal);
    undo_stack_->push(mirror);
}

void DiagramScene::cmdMirrorSelected()
{
    auto mirror = new MirrorSelected(this, ImageMirrorType::Horizontal);
    undo_stack_->push(mirror);
}

void DiagramScene::showCommentEditor(bool value)
{
    switch (state()) {
    case DiagramState::Init: // normal mode
    case DiagramState::EditComment: // update editor
        setState(value ? DiagramState::EditComment : DiagramState::Init);
        break;
    default:
        break;
    }
}

void DiagramScene::showConnectionEditor()
{
    switch (state()) {
    case DiagramState::Init: // normal mode
    case DiagramState::EditConnection: // update editor
        setState(DiagramState::EditConnection);
        connections()->showEditor(true);
        break;
    case DiagramState::MoveItem:
    case DiagramState::ConnectDevice:
    case DiagramState::SelectItem:
    case DiagramState::SelectionRect:
    default:
        break;
    }
}

void DiagramScene::initItemScene()
{
    item_scene_.setGraphicsScene(this);
    connect(&item_scene_, &Scene::added, this, [this](const SharedItemData& data) {
        if (data->category() != ItemCategory::Comment)
            emit DiagramScene::deviceAdded(data);
    });
    connect(&item_scene_, &Scene::removed, this, &DiagramScene::deviceRemoved);
    connect(&item_scene_, &Scene::showCommentEditor, this, &DiagramScene::showCommentEditor);
}

void DiagramScene::initLiveConnection()
{
    tmp_connection_ = new QGraphicsLineItem();
    tmp_connection_->setZValue(ZVALUE_LIVE_CONN);
    tmp_connection_->setVisible(false);
    addItem(tmp_connection_);
}

void DiagramScene::initSceneBackground()
{
    background_.setScene(this);

    SIGNAL_PASS2(&background_, SceneBackground, backgroundChanged, sceneChanged);
    SIGNAL_PASS(&background_, SceneBackground, requestBackgroundChange);
}

void DiagramScene::initSelectionRect()
{
    selection_ = new QGraphicsRectItem();
    selection_->setZValue(ZVALUE_SELECTION);
    auto pen = QPen(Qt::blue);
    pen.setWidth(1);
    pen.setDashPattern({ 2, 2 });
    selection_->setPen(pen);
    selection_->setVisible(false);
    selection_->setBrush(QColor(50, 50, 255, 25));
    addItem(selection_);
}

void DiagramScene::initUndoStack()
{
    undo_stack_ = new QUndoStack(this);
    SIGNAL_PASS(undo_stack_, QUndoStack, canRedoChanged);
    SIGNAL_PASS(undo_stack_, QUndoStack, canUndoChanged);
}

void DiagramScene::initSceneConnections()
{
    connections_ = new SceneConnections(this);
    // passthru
    SIGNAL_PASS2(connections_, SceneConnections, added, connectionAdded);
    SIGNAL_PASS2(connections_, SceneConnections, removed, connectionRemoved);
    SIGNAL_PASS2(connections_, SceneConnections, update, sceneChanged);
    SIGNAL_PASS2(connections_, SceneConnections, visibleChanged, showCablesChanged);

    // slots
    connect(connections_, &SceneConnections::edit, this, &DiagramScene::showConnectionEditor);
    connect(connections_, &SceneConnections::removeRequested, this, &DiagramScene::cmdDisconnectDevices);
}

void DiagramScene::startSelectionAt(const QPointF& pos)
{
    selection_->setPos(pos);
    selection_->setRect({});
    selection_->setVisible(true);
}

void DiagramScene::drawSelectionTo(const QPointF& pos)
{
    QRectF rect({}, pos - selection_->pos());
    selection_->setRect(rect.normalized());
}

void DiagramScene::saveClickPos(const QPointF& pos)
{
    prev_move_pos_ = pos;
    prev_click_pos_ = pos;
}

void DiagramScene::selectTopItem(const QList<QGraphicsItem*>& devs)
{
    for (auto x : devs) {
        auto item = qgraphicsitem_cast<SceneItem*>(x);
        if (item)
            return cmdSelectUnique(item->id());
    }
}

void DiagramScene::sendMouseEventTo(const QList<QGraphicsItem*>& items, QGraphicsSceneMouseEvent* event)
{
    for (auto it : items) {
        if (event->isAccepted())
            break;

        sendEvent(it, event);
    }
}

void DiagramScene::moveSelectedItemsBy(qreal dx, qreal dy)
{
    bool notify = false;

    {
        DiagramSceneUpdatesBlocker ub(this);
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

void DiagramScene::updateConnectionPos(Connection* conn)
{
    if (!conn)
        return;

    auto conn_pos = itemScene().connectionPoints(conn->connectionId());
    if (conn_pos) {
        conn->setPoints(conn_pos->first, conn_pos->second);
        conn->setVisible(true);
    } else {
        conn->setVisible(false);
    }
}

void DiagramScene::updateConnectionPos(SceneItemId id)
{
    DiagramSceneUpdatesBlocker ub(this);

    connections_->showEditor(false);

    for (auto conn : connections_->findConnections(id))
        updateConnectionPos(conn);
}

void DiagramScene::updateConnectionStyle(Connection* conn)
{
    if (!conn)
        return;

    auto pair = itemScene().connectionPair(conn->connectionId());
    if (!pair) {
        WARN() << "connection pair not found";
        return;
    }

    auto style = conn_database_.search(*pair);
    if (style != ConnectionStyle::NotFound) {
        conn->setStyle(style);
    } else {
        WARN() << "style not found";
    }
}

void DiagramScene::clearBackground()
{
    if (!background_.isEmpty()) {
        background_.clear();
        emit sceneChanged();
    }
}

bool DiagramScene::setBackground(const QString& path)
{
    if (background_.loadImage(path)) {
        emit sceneChanged();
        return true;
    } else
        return false;
}

void DiagramScene::setShowBackground(bool value)
{
    background_.setVisible(value);
    emit showBackgroundChanged(value);
}

QJsonObject DiagramScene::toJson() const
{
    QJsonObject json;

    json[JSON_KEY_DEVICES] = itemScene().toJson();

    QJsonArray cons;

    connections_->foreachConn([this, &cons](const ConnectionId& id, const ConnectionViewData& viewData) {
        if (itemScene().checkConnection(id)) {
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

    return json;
}

bool DiagramScene::loadJson(const QString& path, const char* metaKey, DiagramMeta& meta)
{
    WARN() << path;

    connections()->showEditor(false);

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
            DBG() << "open document, created with PatchScene:" << app_vers;

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
            addSceneItem(SceneItem::dataFromJson(j));
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

    auto tmp_meta = DiagramMeta::fromJson(root.value(metaKey));
    if (tmp_meta) {
        meta = tmp_meta.value();
    } else {
        meta = DiagramMeta();
    }

    clearUndoStack();

    return true;
}

void DiagramScene::clearUndoStack()
{
    if (undo_stack_)
        undo_stack_->clear();
}

void DiagramScene::clearAll()
{
    item_scene_.clear();

    QSignalBlocker conn_block(connections_);
    connections_->clear();
    background_.clear();
    undo_stack_->clear();

    emit sceneClearAll();
}

bool DiagramScene::dropJson(const QPointF& pos, const QByteArray& json)
{
    SharedItemData data(new ItemData(SCENE_ITEM_NULL_ID));
    if (!data->setJson(json)) {
        WARN() << "can't set JSON";
        return false;
    }

    data->setPos(pos);
    cmdDuplicateItem(data);
    return true;
}

void DiagramScene::undo()
{
    undo_stack_->undo();
}

void DiagramScene::redo()
{
    undo_stack_->redo();
}

void DiagramScene::drawBackground(QPainter* painter, const QRectF& rect)
{
    if (!grid_visible_)
        return;

    // axis
    QPen pen(QColor(100, 100, 100));
    pen.setWidth(0);
    painter->setBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->drawLine(QLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0)));
    painter->drawLine(QLine(QPoint(0, rect.top()), QPoint(0, rect.bottom())));

    auto p0 = QPoint(0, rect.top());
    auto p1 = QPoint(0, rect.bottom());

    pen.setColor(QColor(100, 100, 100, 100));
    painter->setPen(pen);

    // horizontal grid lines
    for (int i = 0; i <= qCeil(rect.width() / 50); i++) {
        auto x = 50 * (static_cast<int>(rect.left() + i * 50) / 50);
        p0.rx() = x;
        p1.rx() = x;
        painter->drawLine(QLine(p0, p1));
    }

    // vertical grid lines
    p0.rx() = rect.left();
    p1.rx() = rect.right();
    for (int i = 0; i <= qCeil(rect.height() / 50); i++) {
        auto y = 50 * (static_cast<int>(rect.top() + i * 50) / 50);
        p0.ry() = y;
        p1.ry() = y;
        painter->drawLine(QLine(p0, p1));
    }
}

void DiagramScene::keyPressEvent(QKeyEvent* event)
{
    switch (state()) {
    case DiagramState::EditComment:
        // to handle in comment editor
        return QGraphicsScene::keyPressEvent(event);
    case DiagramState::Init:
    case DiagramState::ConnectDevice:
    case DiagramState::EditConnection:
    case DiagramState::MoveItem:
    case DiagramState::SelectItem:
    case DiagramState::SelectionRect:
        break;
    }

    if (!itemScene().hasSelected())
        return QGraphicsScene::keyPressEvent(event);

    auto mods = event->modifiers();

    int MOVE_STEP = 2;
    if (mods.testFlags(Qt::ControlModifier))
        MOVE_STEP = 50;
    else if (mods.testFlags(Qt::ShiftModifier))
        MOVE_STEP = 10;

    if (event->key() == Qt::Key_Backspace && event->modifiers().testFlag(Qt::ControlModifier)) {
        cmdRemoveSelected();
        event->accept();
    } else if (event->key() == Qt::Key_Down) {
        cmdMoveSelectedItemsBy(0, MOVE_STEP);
        event->accept();
    } else if (event->key() == Qt::Key_Up) {
        cmdMoveSelectedItemsBy(0, -MOVE_STEP);
        event->accept();
    } else if (event->key() == Qt::Key_Left) {
        cmdMoveSelectedItemsBy(-MOVE_STEP, 0);
        event->accept();
    } else if (event->key() == Qt::Key_Right) {
        cmdMoveSelectedItemsBy(MOVE_STEP, 0);
        event->accept();
    } else
        QGraphicsScene::keyPressEvent(event);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    switch (state_machine_.state()) {
    case DiagramState::SelectionRect: {
        drawSelectionTo(event->scenePos());
        event->accept();
    } break;
    case DiagramState::SelectItem: {
        state_machine_.setState(DiagramState::MoveItem);
        auto delta = (event->scenePos() - prev_move_pos_);
        moveSelectedItemsBy(delta.x(), delta.y());
        prev_move_pos_ = event->scenePos();
        event->accept();
    } break;
    case DiagramState::MoveItem: {
        auto delta = (event->scenePos() - prev_move_pos_);
        moveSelectedItemsBy(delta.x(), delta.y());
        prev_move_pos_ = event->scenePos();
        event->accept();
    } break;
    case DiagramState::ConnectDevice:
        drawLiveConnectionTo(event->scenePos());
        break;
    case DiagramState::EditConnection:
        event->accept();
        QGraphicsScene::mouseMoveEvent(event);
        break;
    case DiagramState::EditComment:
        QGraphicsScene::mouseMoveEvent(event);
        event->accept();
        break;
    default:
        QGraphicsScene::mouseMoveEvent(event);
        break;
    }
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        QGraphicsScene::mousePressEvent(event);
        return event->accept();
    }

    switch (state_machine_.state()) {
    case DiagramState::Init: {
        auto elems = items(event->scenePos());
        bool item_found = std::any_of(elems.begin(), elems.end(),
            [](QGraphicsItem* x) { return qgraphicsitem_cast<SceneItem*>(x); });

        if (item_found) { // click on the item
            auto xlet = hoverDeviceXlet(elems, event->scenePos());

            if (xlet) { // click on xlet
                bool disconnect = event->modifiers().testFlag(Qt::AltModifier);
                if (disconnect) { // remove xlet connection
                    state_machine_.setState(DiagramState::Init);
                    cmdDisconnectXlet(xlet->first);
                } else { // connection start
                    state_machine_.setState(DiagramState::ConnectDevice);
                    startLiveConnectionAt(event->scenePos());
                    conn_begin_ = xlet;
                }

                // accept?
                return event->accept();

            } else if (event->modifiers().testFlag(Qt::ControlModifier)) { // add/remove to/from selection
                cmdToggleSelected(elems);
            } else if (event->modifiers().testFlag(Qt::ShiftModifier)) { // add to selection
                cmdAddToSelection(elems);
            } else if (event->modifiers().testFlag(Qt::AltModifier)) {
                selectLowestItem(elems);
            } else {
                selectTopItem(elems);
            }

            // TODO(uliss): optimize
            if (!item_scene_.hasSelected()) {
                state_machine_.setState(DiagramState::Init);
            } else {
                saveClickPos(event->scenePos());
                state_machine_.setState(DiagramState::SelectItem);
                connections_->unselectAll();
            }

            event->accept();
        } else {
            sendMouseEventTo(elems, event);

            connections_->unselectAll();
            item_scene_.doneCommentEditor();
            startSelectionAt(event->scenePos());
            state_machine_.setState(DiagramState::SelectionRect);
            event->accept();
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
        QGraphicsScene::mousePressEvent(event);
        if (!event->isAccepted()) {
            state_machine_.setState(DiagramState::Init);
            connections_->showEditor(false);
        }
    } break;
    case DiagramState::EditComment: {
        QGraphicsScene::mousePressEvent(event);
        if (!event->isAccepted()) {
            item_scene_.doneCommentEditor();
            state_machine_.setState(DiagramState::Init);
        }
    } break;
    case DiagramState::MoveItem:
        break;
    case DiagramState::SelectItem:
        break;
    default:
        state_machine_.setState(DiagramState::Init);
        break;
    }
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::RightButton)
        return QGraphicsScene::mouseReleaseEvent(event);

    switch (state()) {
    case DiagramState::MoveItem: { // finish item moving
        setState(DiagramState::Init);

        auto dest_pos = event->scenePos();
        auto src_pos = prev_click_pos_;
        auto delta = src_pos - dest_pos;

        item_scene_.foreachItem([delta](SceneItem* item) {
            if (item->isSelected() && !item->isLocked())
                item->moveBy(delta.x(), delta.y());
        });

        cmdMoveSelectedItemsFrom(src_pos, dest_pos);
        event->accept();
    } break;
    case DiagramState::SelectionRect: { // finish selection
        auto bbox = selection_->mapRectToScene(selection_->rect());
        if (event->modifiers().testFlag(Qt::ShiftModifier)) {
            cmdAddToSelection(bbox);
        } else {
            cmdSelectItems(bbox);
        }

        selection_->setVisible(false);
        setState(DiagramState::Init);
        event->accept();
    } break;
    case DiagramState::ConnectDevice: { // finish connection
        tmp_connection_->setVisible(false);
        setState(DiagramState::Init);

        auto conn_end = hoverDeviceXlet(items(event->scenePos()), event->scenePos());
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
        QGraphicsScene::mouseReleaseEvent(event);
        break;
    case DiagramState::EditComment:
        QGraphicsScene::mouseReleaseEvent(event);
        break;
    case DiagramState::SelectItem:
        setState(DiagramState::Init);
        break;
    case DiagramState::Init: // fallthru
    default:
        setState(DiagramState::Init);
        QGraphicsScene::mouseReleaseEvent(event);
        event->accept();
        break;
    }
}

void DiagramScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    switch (state()) {
    case DiagramState::Init:
    case DiagramState::ConnectDevice:
    case DiagramState::EditConnection:
    case DiagramState::EditComment:
    case DiagramState::MoveItem:
    case DiagramState::SelectItem:
    case DiagramState::SelectionRect:
    default:
        sendMouseEventTo(items(event->scenePos()), event);
        break;
    }
}

void DiagramScene::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
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
            WARN() << "single image file expected, got:" << files;
        }
    } else {
        WARN() << "unsupported MIME type:" << event->mimeData()->formats();
    }
}

void DiagramScene::dragMoveEvent(QGraphicsSceneDragDropEvent* event)
{
    event->acceptProposedAction();
}

void DiagramScene::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    if (event->mimeData()->formats().contains("text/plain")) {
        if (dropJson(event->scenePos(), event->mimeData()->data("text/plain")))
            event->acceptProposedAction();
    } else if (event->mimeData()->hasUrls()) {
        auto files = event->mimeData()->urls();
        if (files.size() == 1) {
            if (setBackground(files.front().toLocalFile()))
                event->acceptProposedAction();
        } else {
            WARN() << "single image file expected, got:" << files;
        }
    }
}
