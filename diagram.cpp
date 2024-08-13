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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPrintDialog>
#include <QPrinter>

#include "device.h"
#include "deviceproperties.h"
#include "patch_scene_version.h"
#include "undo_commands.h"

constexpr qreal MAX_ZOOM = 4.0;
constexpr qreal MIN_ZOOM = 1.0 / MAX_ZOOM;

constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_CONNS = "connections";
constexpr const char* JSON_KEY_BACKGROUND = "background";
constexpr const char* JSON_KEY_APP = "application";
constexpr const char* JSON_KEY_VERSION = "version";
constexpr const char* JSON_KEY_VERSION_MAJOR = "version-major";
constexpr const char* JSON_KEY_VERSION_MINOR = "version-minor";
constexpr const char* JSON_KEY_VERSION_PATCH = "version-patch";
constexpr const char* JSON_KEY_VERSION_GIT = "version-git";
constexpr const char* JSON_KEY_META = "meta";

class ChangeEmitter {
    Diagram* diagram_;
    bool changed_;

public:
    ChangeEmitter(Diagram* diagram)
        : diagram_ { diagram }
        , changed_ { false }
    {
    }

    ~ChangeEmitter()
    {
        if (diagram_ && changed_)
            emit diagram_->sceneChanged();
    }

    void trigger() { changed_ = true; }

    operator bool() const { return changed_; }
};

void Diagram::initUndoStack()
{
    undo_stack_ = new QUndoStack(this);
    connect(undo_stack_, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
    connect(undo_stack_, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
}

Diagram::Diagram(QWidget* parent)
    : QGraphicsView { parent }
{
    meta_.setTitle(tr("New project"));

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMinimumHeight(300);
    setMinimumWidth(300);
    setAcceptDrops(true);

    setRenderHint(QPainter::Antialiasing);

    initScene();

    initLiveConnection();
    initSelectionRect();
    initUndoStack();
}

void Diagram::initSelectionRect()
{
    selection_ = new QGraphicsRectItem();
    selection_->setZValue(ZVALUE_SELECTION);
    auto pen = QPen(Qt::blue);
    pen.setDashPattern({ 2, 2 });
    selection_->setPen(pen);
    selection_->setVisible(false);
    scene->addItem(selection_);
}

void Diagram::initLiveConnection()
{
    connection_ = new QGraphicsLineItem();
    connection_->setZValue(ZVALUE_LIVE_CONN);
    connection_->setVisible(false);
    scene->addItem(connection_);
}

void Diagram::initScene()
{
    scene = new QGraphicsScene();
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    // scene->setSceneRect(-250, -250, 1000, 1000); // Устанавливаем область графической сцены
    scene->setBackgroundBrush(QColor(252, 252, 252));
    scene->setMinimumRenderSize(0.5);
    setScene(scene);
}

bool Diagram::removeDevice(DeviceId id)
{
    ChangeEmitter ch(this);

    for (auto x : scene->items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn && conn->relatesToId(id)) {
            emit connectionRemoved(conn->connectionData());
            delete conn;
            ch.trigger();
            continue;
        }

        auto dev = qgraphicsitem_cast<const Device*>(x);
        if (dev && dev->id() == id) {
            emit deviceRemoved(dev->deviceData());
            delete dev;
            ch.trigger();
            continue;
        }
    }

    return ch;
}

void Diagram::updateConnectionsPos()
{
    for (auto c : connections()) {
        if (c->updateCachedPos())
            c->update(c->boundingRect());
        else
            delete c;
    }
}

void Diagram::cmdRemoveSelected()
{
    auto del_sel = new RemoveSelected(this);
    undo_stack_->push(del_sel);
}

void Diagram::cmdRemoveDevice(const SharedDeviceData& data)
{
    auto rem = new RemoveDevice(this, data);
    undo_stack_->push(rem);
}

void Diagram::cmdUpdateDevice(const SharedDeviceData& data)
{
    auto up = new UpdateDeviceData(this, data);
    undo_stack_->push(up);
}

void Diagram::cmdCreateDevice(const QPointF& pos)
{
    auto add = new CreateDevice(this, pos);
    undo_stack_->push(add);
}

void Diagram::cmdToggleDevices(const QList<QGraphicsItem*>& items)
{
    QList<DeviceId> ids;
    ids.reserve(items.size());
    for (auto x : items) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            ids.push_back(dev->id());
    }

    auto tgl = new ToggleDevices(this, ids);
    undo_stack_->push(tgl);
}

void Diagram::cmdConnectDevices(const ConnectionData& conn)
{
    auto x = new ConnectDevices(this, conn);
    undo_stack_->push(x);
}

void Diagram::cmdDuplicateDevice(const SharedDeviceData& data)
{
    auto dup = new DuplicateDevice(this, data);
    undo_stack_->push(dup);
}

void Diagram::cmdDuplicateSelection()
{
    auto dup = new DuplicateSelected(this);
    undo_stack_->push(dup);
}

void Diagram::cmdSelectAll()
{
    auto sel = new AddDeviceSelection(this, allDeviceIds());
    undo_stack_->push(sel);
}

void Diagram::cmdAddToSelection(const QRectF& sel)
{
    QList<DeviceId> ids;

    for (auto dev : devices()) {
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (sel.intersects(scene_bbox))
            ids.push_back(dev->id());
    }

    auto sel_devs = new AddDeviceSelection(this, ids);
    undo_stack_->push(sel_devs);
}

void Diagram::cmdAddToSelection(const QList<QGraphicsItem*>& items)
{
    QList<DeviceId> ids;
    for (auto x : items) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            ids.push_back(dev->id());
    }

    auto add_sel = new AddDeviceSelection(this, ids);
    undo_stack_->push(add_sel);
}

void Diagram::cmdSelectDevices(const QRectF& sel)
{
    QSet<DeviceId> ids;

    for (auto dev : devices()) {
        auto scene_bbox = dev->mapRectToScene(dev->boundingRect());
        if (scene_bbox.intersects(sel))
            ids.insert(dev->id());
    }

    auto set_sel = new SetDeviceSelection(this, ids);
    undo_stack_->push(set_sel);
}

void Diagram::cmdSelectUnique(DeviceId id)
{
    QSet<DeviceId> ids { id };
    auto sel = new SetDeviceSelection(this, ids);
    undo_stack_->push(sel);
}

void Diagram::cmdDisconnectXlet(const XletInfo& xi)
{
    auto xconn = new DisconnectXlet(this, xi);
    undo_stack_->push(xconn);
}

void Diagram::cmdMoveSelectedDevicesBy(qreal dx, qreal dy)
{
    auto move_by = new MoveSelected(this, dx, dy);
    undo_stack_->push(move_by);
}

void Diagram::cmdMoveSelectedDevicesFrom(const QPointF& from, const QPointF& to)
{
    auto sel_devs = selectedDevices();
    if (sel_devs.empty())
        return;

    auto delta = to - from;
    for (auto dev : sel_devs)
        dev->setPos(dev->pos() - delta);

    auto move_by = new MoveSelected(this, delta.x(), delta.y());
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignVSelected()
{
    auto sel_devs = selectedDevices();
    if (sel_devs.empty())
        return;

    qreal x = 0; // find middle x-position
    for (auto dev : sel_devs) {
        x += dev->x();
    }
    x /= sel_devs.size();

    QMap<DeviceId, QPointF> deltas;
    for (auto dev : sel_devs) {
        deltas.insert(dev->id(), QPointF(x - dev->x(), 0));
    }

    auto move_by = new MoveByDevices(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignHSelected()
{
    auto sel_devs = selectedDevices();
    if (sel_devs.empty())
        return;

    qreal y = 0; // find middle y-position
    for (auto dev : sel_devs) {
        y += dev->y();
    }
    y /= sel_devs.size();

    QMap<DeviceId, QPointF> deltas;
    for (auto dev : sel_devs) {
        deltas.insert(dev->id(), QPointF(0, y - dev->y()));
    }

    auto move_by = new MoveByDevices(this, deltas);
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

bool Diagram::addDevice(Device* dev)
{
    if (!dev)
        return false;

    scene->addItem(dev);
    emit sceneChanged();
    emit deviceAdded(dev->deviceData());
    return true;
}

bool Diagram::addConnection(Connection* conn)
{
    if (!conn)
        return false;

    scene->addItem(conn);
    emit sceneChanged();
    emit connectionAdded(conn->connectionData());
    return true;
}

void Diagram::saveClickPos(const QPoint& pos)
{
    prev_event_pos_ = mapToScene(pos);
    prev_click_pos_ = prev_event_pos_;
}

QList<DeviceId> Diagram::allDeviceIds() const
{
    QList<DeviceId> res;

    for (auto x : scene->items()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            res.push_back(dev->id());
    }

    return res;
}

bool Diagram::setDeviceData(const SharedDeviceData& data)
{
    auto dev = findDeviceById(data->id());
    if (!dev) {
        qWarning() << "device not found:" << data->id();
        return false;
    }

    const bool title_update = (dev->deviceData()->title() != data->title());

    dev->setDeviceData(data);
    emit sceneChanged();
    emit deviceUpdated(dev->deviceData());

    if (title_update)
        emit deviceTitleUpdated(data->id(), data->title());

    updateConnectionsPos();

    return true;
}

void Diagram::setShowCables(bool value)
{
    show_cables_ = value;
    for (auto c : connections())
        c->setVisible(value);

    emit showCablesChanged(value);
}

void Diagram::setShowBackground(bool value)
{
    show_background_ = value;
    if (background_)
        background_->setVisible(value);

    emit showBackgroundChanged(value);
}

bool Diagram::setBackground(const QString& path)
{
    if (!background_) {
        background_ = new DiagramImage(path);
        background_->setZValue(ZVALUE_BACKGROUND);
        scene->addItem(background_);
        background_->setPos(0, 0);
        if (!background_->isEmpty()) {
            emit sceneChanged();
            return true;
        } else
            return false;
    } else {
        auto res = background_->setImagePath(path);
        if (res)
            emit sceneChanged();

        return res;
    }
}

void Diagram::clearBackground()
{
    if (background_ && !background_->isEmpty()) {
        background_->clearImage();
        emit sceneChanged();
    }
}

bool Diagram::loadJson(const QString& path)
{
    qDebug() << __FUNCTION__ << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << __FUNCTION__ << "can't open file:" << path;
        return false;
    }

    auto val = file.readAll();
    file.close();

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(val, &err);
    if (!doc.isObject()) {
        qWarning() << "invalid JSON file: " << err.errorString();
        return false;
    }

    auto root = doc.object();

    clearAll();

    auto app = root.value(JSON_KEY_APP).toObject();
    if (!app.isEmpty()) {
        auto app_vers = app.value(JSON_KEY_VERSION).toString();
        if (!app_vers.isEmpty())
            qDebug() << "open document, created with PatchScene:" << app_vers;

        auto app_maj = app.value(JSON_KEY_VERSION_MAJOR).toInt();
        auto app_min = app.value(JSON_KEY_VERSION_MINOR).toInt();

        auto int_vers = (app_maj * 100) + app_min;
        if (int_vers > PATCH_SCENE_VERSION_INT) {
            qWarning() << "the document was created with more recent version, then the current one, "
                          "some feature can be missing...";
        }
    }

    // load devices
    auto devs = root.value(JSON_KEY_DEVICES);
    if (devs.isArray()) {
        auto arr = devs.toArray();
        for (const auto& j : arr) {
            auto dev = Device::fromJson(j);
            if (dev)
                addDevice(dev.release());
        }
    }

    // load connections
    auto cons = root.value(JSON_KEY_CONNS);
    if (cons.isArray()) {
        auto arr = cons.toArray();
        for (const auto& j : arr) {
            ConnectionData data(0, 0, 0, 0);
            if (ConnectionData::fromJson(j, data))
                addConnection(new Connection(data));
        }

        updateConnectionsPos();
    }

    if (root.contains(JSON_KEY_BACKGROUND)) {
        auto bg_img = DiagramImage::fromJson(root.value(JSON_KEY_BACKGROUND));
        if (bg_img) {
            background_ = bg_img.release();
            scene->addItem(background_);
        } else {
            qWarning() << "can't load bg";
        }
    } else {
        delete background_;
        background_ = nullptr;
    }

    auto meta = DiagramMeta::fromJson(root.value(JSON_KEY_META));
    if (meta) {
        meta_ = meta.value();
    } else {
        meta_ = DiagramMeta();
    }

    return true;
}

bool Diagram::findConnectionXletData(const ConnectionData& data, XletData& src, XletData& dest, Device** src_dev, Device** dest_dev) const
{
    int count = 0;

    for (auto x : scene->items()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (!dev)
            continue;

        if (count == 2)
            break;

        if (dev->id() == data.src) {
            auto dev_data = dev->deviceData();
            if (data.out < dev_data->outputs().size()) {
                src = dev_data->outputAt(data.out);
                if (src_dev)
                    *src_dev = dev;
                count++;
            } else {
                qWarning() << "invalid source outlet:" << (int)data.out;
            }
        } else if (dev->id() == data.dest) {
            auto dev_data = dev->deviceData();
            if (data.in < dev_data->inputs().size()) {
                dest = dev_data->inputAt(data.in);
                if (dest_dev)
                    *dest_dev = dev;
                count++;
            } else {
                qWarning() << "invalid dest inlet:" << (int)data.in;
            }
        }
    }

    return count == 2;
}

void Diagram::zoomIn()
{
    updateZoom(zoom_ * 3 / 2.0);
}

void Diagram::zoomOut()
{
    updateZoom(zoom_ * 2 / 3.0);
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
    auto sel_devs = selectedDevices();
    if (sel_devs.empty())
        return;

    clip_buffer_.clear();

    for (auto& dev : sel_devs)
        clip_buffer_.push_back(dev->deviceData());
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
        for (auto x : scene->items())
            x->setCacheMode(QGraphicsItem::NoCache);

        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        scene->render(&painter);

        for (auto x : scene->items())
            x->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
}

void Diagram::clearAll()
{
    for (auto x : items()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev) {
            delete dev;
            continue;
        }

        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn) {
            delete conn;
            continue;
        }
    }

    if (background_) {
        delete background_;
        background_ = nullptr;
    }

    emit sceneClearAll();
}

QJsonObject Diagram::toJson() const
{
    QJsonObject json;

    QJsonArray devs;
    for (auto dev : devices())
        devs.append(dev->toJson());

    json[JSON_KEY_DEVICES] = devs;

    QJsonArray cons;
    for (auto c : connections()) {
        if (c->checkValid()) {
            cons.append(c->toJson());
        } else { // remove invalid connections on save
            delete c;
        }
    }

    json[JSON_KEY_CONNS] = cons;

    if (background_)
        json[JSON_KEY_BACKGROUND] = background_->toJson();

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
    connection_->setLine(QLineF {});
    connection_->setPos(mapToScene(pos));
    connection_->setVisible(true);
}

void Diagram::drawConnectionTo(const QPoint& pos)
{
    connection_->setLine(QLineF(QPointF {}, connection_->mapFromScene(mapToScene(pos))));
}

void Diagram::drawSelectionTo(const QPoint& pos)
{
    QRectF rect(QPointF {}, selection_->mapFromScene(mapToScene(pos)));
    selection_->setRect(rect.normalized());
}

void Diagram::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton) {
        return emit customContextMenuRequested(event->pos());
    }

    switch (state_machine_.state()) {
    case DiagramState::Init: {
        auto devs = items(event->pos());
        bool device_found = std::any_of(devs.begin(), devs.end(), [](QGraphicsItem* x) { return qgraphicsitem_cast<Device*>(x); });

        if (device_found) {
            const auto xlet = hoverDeviceXlet(devs, event->pos());

            if (xlet != XletInfo::none()) {
                bool disconnect = event->modifiers().testFlag(Qt::AltModifier);
                if (disconnect) {
                    state_machine_.setState(DiagramState::Init);
                    cmdDisconnectXlet(xlet);
                } else {
                    state_machine_.setState(DiagramState::ConnectDevice);
                    startConnectionAt(event->pos());
                    conn_start_ = xlet;
                }

                return; //

            } else if (event->modifiers().testFlag(Qt::ControlModifier)) { // add/remove to/from selection
                cmdToggleDevices(devs);
            } else if (event->modifiers().testFlag(Qt::ShiftModifier)) { // add to selection
                cmdAddToSelection(devs);
            } else
                selectTopDevice(devs);

            if (scene->selectedItems().empty()) {
                state_machine_.setState(DiagramState::Init);
            } else {
                saveClickPos(event->pos());
                state_machine_.setState(DiagramState::SelectDevice);
            }
        } else {
            startSelectionAt(event->pos());
            state_machine_.setState(DiagramState::SelectionRect);
        }
    } break;
    case DiagramState::ConnectDevice: {
        connection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
    } break;
    case DiagramState::SelectionRect: {
        selection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
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
    } break;
    case DiagramState::SelectDevice: {
        state_machine_.setState(DiagramState::Move);
    } break;
    case DiagramState::Move: {
        auto delta = mapToScene(event->pos()) - prev_event_pos_;
        moveSelectedItemsBy(delta.x(), delta.y());
        prev_event_pos_ = mapToScene(event->pos());
    } break;
    case DiagramState::ConnectDevice:
        drawConnectionTo(event->pos());
        break;
    default:
        break;
    }
}

void Diagram::mouseReleaseEvent(QMouseEvent* event)
{
    switch (state_machine_.state()) {
    case DiagramState::Move: { // finish item moving
        state_machine_.setState(DiagramState::Init);
        cmdMoveSelectedDevicesFrom(prev_click_pos_, mapToScene(event->pos()));
    } break;
    case DiagramState::SelectionRect: { // finish selection
        auto bbox = selection_->mapRectToScene(selection_->rect());
        if (event->modifiers().testFlag(Qt::ShiftModifier))
            cmdAddToSelection(bbox);
        else
            cmdSelectDevices(bbox);

        selection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);
    } break;
    case DiagramState::ConnectDevice: { // finish connection
        connection_->setVisible(false);
        state_machine_.setState(DiagramState::Init);

        auto xlet = hoverDeviceXlet(items(event->pos()), event->pos());
        if (xlet != XletInfo::none()) {
            if (!isValidConnection(conn_start_, xlet))
                return;

            if (xlet.type == XletType::Out)
                cmdConnectDevices(ConnectionData(xlet.id, xlet.index, conn_start_.id, conn_start_.index));
            else if (xlet.type == XletType::In)
                cmdConnectDevices(ConnectionData(conn_start_.id, conn_start_.index, xlet.id, xlet.index));
        }

        conn_start_ = XletInfo::none();
    } break;
    default:
        state_machine_.setState(DiagramState::Init);
        break;
    }
}

void Diagram::contextMenuEvent(QContextMenuEvent* event)
{
    event->accept();
    auto pos = event->pos();
    auto dev = deviceAt(pos);

    if (dev) {
        auto selected = scene->selectedItems();

        if (selected.size() <= 1) {
            auto id = dev->id();
            auto data = dev->deviceData();

            auto duplicateAct = new QAction(tr("Duplicate"), this);
            connect(duplicateAct, &QAction::triggered, this,
                [this, data]() { cmdDuplicateDevice(data); });

            auto removeAct = new QAction(tr("Delete"), this);
            connect(removeAct, &QAction::triggered, this,
                [this, data]() { cmdRemoveDevice(data); });

            auto addToFavoritesAct = new QAction(tr("Add to favorites"), this);
            connect(addToFavoritesAct, &QAction::triggered, this,
                [this, data]() { emit addToFavorites(data); });

            auto propertiesAct = new QAction(tr("Properties"), this);
            connect(propertiesAct, &QAction::triggered, this,
                [this, id]() {
                    auto dev = findDeviceById(id);
                    if (!dev)
                        return;

                    auto dialog = new DeviceProperties(this, dev->id(), dev->deviceData());
                    dialog->exec();
                });

            QMenu menu(this);
            menu.addAction(duplicateAct);
            menu.addAction(removeAct);
            menu.addSeparator();
            menu.addAction(addToFavoritesAct);
            menu.addAction(propertiesAct);
            menu.exec(mapToGlobal(pos));
        } else {
            auto deleteSelectedAct = new QAction(tr("Delete selected"), this);
            connect(deleteSelectedAct, &QAction::triggered, this,
                [this]() { cmdRemoveSelected(); });

            auto alignVerticalAct = new QAction(tr("Align vertical"), this);
            connect(alignVerticalAct, &QAction::triggered, this,
                [this]() { cmdAlignVSelected(); });

            auto alignHorizontalAct = new QAction(tr("Align horizontal"), this);
            connect(alignHorizontalAct, &QAction::triggered, this,
                [this]() { cmdAlignHSelected(); });

            QMenu menu(this);
            menu.addAction(deleteSelectedAct);
            menu.addAction(alignVerticalAct);
            menu.addAction(alignHorizontalAct);
            menu.exec(mapToGlobal(pos));
        }
    } else {
        auto addAct = new QAction(tr("&Add device"), this);
        connect(addAct, &QAction::triggered, this,
            [this, pos]() { cmdCreateDevice(mapToScene(pos)); });

        auto clearBgAct = new QAction(tr("&Clear background"), this);
        connect(clearBgAct, &QAction::triggered, this,
            [this, pos]() { clearBackground(); });

        QMenu menu(this);
        menu.addAction(addAct);
        menu.addAction(clearBgAct);
        menu.exec(mapToGlobal(pos));
    }
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
            if (setBackground(files.front().path()))
                event->acceptProposedAction();
        } else {
            qDebug() << __FUNCTION__ << "single image file expected, got:" << files;
        }
    }
}

void Diagram::keyPressEvent(QKeyEvent* event)
{
    const int MOVE_STEP = 2 + (8 * event->modifiers().testFlag(Qt::ShiftModifier));

    if (event->key() == Qt::Key_Backspace && event->modifiers().testFlag(Qt::ControlModifier)) {
        cmdRemoveSelected();
    } else if (event->key() == Qt::Key_Down) {
        cmdMoveSelectedDevicesBy(0, MOVE_STEP);
    } else if (event->key() == Qt::Key_Up) {
        cmdMoveSelectedDevicesBy(0, -MOVE_STEP);
    } else if (event->key() == Qt::Key_Left) {
        cmdMoveSelectedDevicesBy(-MOVE_STEP, 0);
    } else if (event->key() == Qt::Key_Right) {
        cmdMoveSelectedDevicesBy(MOVE_STEP, 0);
    } else
        QGraphicsView::keyPressEvent(event);
}

#ifdef Q_OS_DARWIN
bool Diagram::viewportEvent(QEvent* event)
{
    switch (event->type()) {
    case QEvent::NativeGesture: {
        auto nge = dynamic_cast<QNativeGestureEvent*>(event);
        if (nge) {
            switch (nge->gestureType()) {
            case Qt::ZoomNativeGesture: { // mac two-finger zoom
                updateZoom(zoom_ * (1 + nge->value()));
                return true;
            } break;
            case Qt::SmartZoomNativeGesture: { // smart zoom on two-finger double tap
                if (nge->value() == 0)
                    zoomNormal();
                else
                    updateZoom(1.5);

                return true;
            } break;
            default:
                break;
            }
        }
    } break;
    default:
        break;
    }

    return QGraphicsView::viewportEvent(event);
}
#endif

QList<Connection*> Diagram::connections() const
{
    QList<Connection*> res;
    for (auto x : items()) {

        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn)
            res.push_back(conn);
    }

    return res;
}

QList<Device*> Diagram::devices() const
{
    QList<Device*> res;
    for (auto x : items()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            res.push_back(dev);
    }

    return res;
}

QList<Device*> Diagram::selectedDevices() const
{
    auto sel_devs = scene->selectedItems();

    QList<Device*> res;
    res.reserve(sel_devs.size());

    for (auto x : sel_devs) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            res.push_back(dev);
    }

    return res;
}

void Diagram::selectTopDevice(const QList<QGraphicsItem*>& devs)
{
    auto top = std::max_element(devs.begin(), devs.end(), [](QGraphicsItem* x, QGraphicsItem* y) {
        return x->zValue() < y->zValue();
    });

    if (top == devs.end())
        return;

    auto dev = qgraphicsitem_cast<Device*>(*top);
    if (!dev) {
        qWarning() << "not a device:" << *top;
        return;
    }

    cmdSelectUnique(dev->id());
}

XletInfo Diagram::hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt) const
{
    Device* dev = nullptr;
    for (auto x : devs) {
        dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            break;
    }

    if (!dev)
        return XletInfo::none();

    auto in = dev->inletAt(mapToScene(pt));
    if (in >= 0) {
        return { dev->id(), XletType::In, in };
    }

    auto out = dev->outletAt(mapToScene(pt));
    if (out >= 0) {
        return { dev->id(), XletType::Out, out };
    }

    return XletInfo::none();
}

bool Diagram::connectDevices(const ConnectionData& data)
{
    for (auto x : items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn && *conn == data) {
            qWarning() << "already connected";
            return false;
        }
    }

    auto conn = new Connection(data);
    if (addConnection(conn)) {
        // @TODO??? need update all??
        updateConnectionsPos();
        return true;
    } else {
        delete conn;
        return false;
    }
}

bool Diagram::disconnectDevices(const ConnectionData& data)
{
    ChangeEmitter ch(this);

    for (auto x : items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn && *conn == data) {
            ch.trigger();
            emit connectionRemoved(data);
            delete conn;
            return true;
        }
    }

    return false;
}

void Diagram::moveSelectedItemsBy(qreal dx, qreal dy)
{
    ChangeEmitter ch(this);

    for (auto dev : scene->selectedItems()) {
        ch.trigger();
        dev->moveBy(dx, dy);
    }

    if (ch)
        updateConnectionsPos();
}

void Diagram::selectDevices(const QList<DeviceId>& ids, bool value)
{
    for (auto id : ids) {
        auto dev = findDeviceById(id);
        if (dev)
            dev->setSelected(value);
    }
}

void Diagram::toggleDevices(const QList<DeviceId>& ids)
{
    for (auto id : ids) {
        auto dev = findDeviceById(id);
        if (dev)
            dev->setSelected(!dev->isSelected());
    }
}

void Diagram::clearClipBuffer()
{
    clip_buffer_.clear();
}

const QList<SharedDeviceData>& Diagram::clipBuffer() const
{
    return clip_buffer_;
}

void Diagram::setClipBuffer(const QList<SharedDeviceData>& data)
{
    clip_buffer_ = data;
}

void Diagram::updateZoom(qreal zoom)
{
    if (zoom < MIN_ZOOM || zoom > MAX_ZOOM)
        return;

    zoom_ = zoom;
    setTransform(QTransform::fromScale(zoom_, zoom_));
    emit zoomChanged(zoom_);
}

bool Diagram::isValidConnection(const XletInfo& src, const XletInfo& dest) const
{
    if (src.id == dest.id) {
        qWarning() << "self connection attempt";
        return false;
    }

    if (src.type == dest.type) {
        qWarning() << "same xlet type connection attempt";
        return false;
    }

    auto data = (src.type == XletType::Out)
        ? ConnectionData(src.id, src.index, dest.id, dest.index)
        : ConnectionData(dest.id, dest.index, src.id, src.index);

    for (auto x : scene->items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn) {
            auto item_data = conn->connectionData();
            if (data.isSameDestimation(item_data) || data.isSameSource(item_data)) {
                qWarning() << "already connected";
                return false;
            }
        }
    }

    return true;
}

Device* Diagram::deviceAt(const QPoint& pos) const
{
    for (auto x : items(pos)) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            return dev;
    }

    return nullptr;
}

bool Diagram::dropJson(const QPointF& pos, const QByteArray& json)
{
    if (json.isEmpty()) {
        qDebug() << __FUNCTION__ << "empty data";
        return false;
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(json, &err);
    if (doc.isNull()) {
        qWarning() << doc << err.errorString();
        return false;
    }

    SharedDeviceData data(new DeviceData(DEV_NULL_ID));
    if (!data->setJson(doc.object())) {
        qWarning() << "can't set JSON";
        return false;
    }

    data->setPos(pos);
    cmdDuplicateDevice(data);
    return true;
}

QJsonValue Diagram::appInfoJson() const
{
    QJsonObject obj;

    obj[JSON_KEY_VERSION] = PATCH_SCENE_VERSION;
    obj[JSON_KEY_VERSION_MAJOR] = PATCH_SCENE_VERSION_MAJOR;
    obj[JSON_KEY_VERSION_MINOR] = PATCH_SCENE_VERSION_MAJOR;
    obj[JSON_KEY_VERSION_PATCH] = PATCH_SCENE_VERSION_PATCH;
    obj[JSON_KEY_VERSION_GIT] = PATCH_SCENE_GIT_VERSION;

    return obj;
}

Device* Diagram::findDeviceById(DeviceId id) const
{
    for (auto x : scene->items()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev && dev->id() == id)
            return dev;
    }

    return nullptr;
}

Connection* Diagram::findConnectionByXlet(const XletInfo& xi) const
{
    for (auto c : connections()) {
        if (xi == c->sourceInfo() || xi == c->destinationInfo())
            return c;
    }

    return nullptr;
}

QList<ConnectionData> Diagram::findDeviceConnections(DeviceId id) const
{
    QList<ConnectionData> res;

    for (auto x : items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn) {
            if (conn->relatesToId(id))
                res.push_back(conn->connectionData());
        }
    }

    return res;
}

QSet<ConnectionData> Diagram::findSelectedConnections() const
{
    QSet<ConnectionData> res;

    QList<DeviceId> dev_ids;
    dev_ids.reserve(32);

    // fill selected ID's
    for (auto& x : scene->selectedItems()) {
        auto dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            dev_ids.push_back(dev->id());
    }

    for (auto& x : scene->items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn) {
            for (auto id : dev_ids) {
                if (conn->relatesToId(id))
                    res.insert(conn->connectionData());
            }
        }
    }

    return res;
}
