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
#include "device.h"
#include "undo_commands.h"

using namespace ceam;

namespace {
constexpr qreal MAX_ZOOM = 4.0;
constexpr qreal MIN_ZOOM = 1.0 / MAX_ZOOM;

constexpr const char* JSON_KEY_APP = "application";
constexpr const char* JSON_KEY_BACKGROUND = "background";
constexpr const char* JSON_KEY_CONNS = "connections";
constexpr const char* JSON_KEY_DEVICES = "devices";
constexpr const char* JSON_KEY_FORMAT_VERSION = "format-version";
constexpr const char* JSON_KEY_META = "meta";
constexpr const char* JSON_KEY_VERSION = "version";
constexpr const char* JSON_KEY_VERSION_GIT = "version-git";
constexpr const char* JSON_KEY_VERSION_MAJOR = "version-major";
constexpr const char* JSON_KEY_VERSION_MINOR = "version-minor";
constexpr const char* JSON_KEY_VERSION_PATCH = "version-patch";

class MyScene : public QGraphicsScene {
public:
    MyScene()
    {

        //     pen.setWidth(0);
        //     painter->setPen(pen);

        // painter->drawLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0));
        // painter->drawLine(QPoint(0, rect.top()), QPoint(0, rect.bottom()));
        // painter->drawLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0));
        // painter->drawLine(QPoint(0, rect.top()), QPoint(0, rect.bottom()));
    }
};

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
}

using namespace ceam;

void Diagram::initUndoStack()
{
    undo_stack_ = new QUndoStack(this);
    connect(undo_stack_, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
    connect(undo_stack_, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
}

Diagram::Diagram(int w, int h, QWidget* parent)
    : QGraphicsView { parent }
{
    meta_.setTitle(tr("New project"));

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setAlignment(Qt::AlignCenter);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setMinimumWidth(400);
    setMinimumHeight(300);

    setAcceptDrops(true);

    setRenderHint(QPainter::Antialiasing);

    initScene(w, h);

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
    scene_->addItem(selection_);
}

void Diagram::initLiveConnection()
{
    connection_ = new QGraphicsLineItem();
    connection_->setZValue(ZVALUE_LIVE_CONN);
    connection_->setVisible(false);
    scene_->addItem(connection_);
}

void Diagram::initScene(int w, int h)
{
    scene_ = new MyScene();
    scene_->setParent(this);
    scene_->setSceneRect(-w / 2, -h / 2, w, h);
    setScene(scene_);

    if (!grid_) {
        grid_ = new QGraphicsItemGroup;
        scene_->addItem(grid_);
    }

    auto rect = sceneRect();
    createAxis(rect);
    createGrid(rect);

    devices_.setScene(scene_);
}

bool Diagram::removeDevice(DeviceId id)
{
    auto data = devices_.remove(id);
    if (data) {
        removeDeviceConnections(id);
        emit deviceRemoved(data);
        emit sceneChanged();
    }

    return true;
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
    auto sel = new AddDeviceSelection(this, devices_.idList());
    undo_stack_->push(sel);
}

void Diagram::cmdAddToSelection(const QRectF& sel)
{
    auto sel_devs = new AddDeviceSelection(this, devices_.intersectedList(sel));
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
    auto set_sel = new SetDeviceSelection(this, devices_.intersected(sel));
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
    if (!devices_.hasSelected())
        return;

    const auto delta = to - from;

    devices_.foreachSelectedDevice([delta, this](Device* dev) {
        dev->moveBy(delta.x(), delta.y());
    });

    auto move_by = new MoveSelected(this, delta.x(), delta.y());
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignVSelected()
{
    auto sel_data = devices_.selectedDataList();
    if (sel_data.empty())
        return;

    qreal x = 0; // find middle x-position
    for (auto& data : sel_data) {
        x += data->pos().x();
    }
    x /= sel_data.size();

    QMap<DeviceId, QPointF> deltas;
    for (auto& data : sel_data) {
        deltas.insert(data->id(), QPointF(x - data->pos().x(), 0));
    }

    auto move_by = new MoveByDevices(this, deltas);
    undo_stack_->push(move_by);
}

void Diagram::cmdAlignHSelected()
{
    auto sel_data = devices_.selectedDataList();
    if (sel_data.empty())
        return;

    qreal y = 0; // find middle y-position
    for (auto& data : sel_data) {
        y += data->pos().y();
    }
    y /= sel_data.size();

    QMap<DeviceId, QPointF> deltas;
    for (auto& data : sel_data) {
        deltas.insert(data->id(), QPointF(0, y - data->pos().y()));
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

Device* Diagram::addDevice(const SharedDeviceData& data)
{
    auto dev = devices_.add(data);
    if (!dev)
        return nullptr;

    connect(dev, SIGNAL(addToFavorites(SharedDeviceData)), this, SIGNAL(addToFavorites(SharedDeviceData)));
    connect(dev, SIGNAL(duplicateDevice(SharedDeviceData)), this, SLOT(cmdDuplicateDevice(SharedDeviceData)));
    connect(dev, SIGNAL(removeDevice(SharedDeviceData)), this, SLOT(cmdRemoveDevice(SharedDeviceData)));
    connect(dev, SIGNAL(updateDevice(SharedDeviceData)), this, SLOT(cmdUpdateDevice(SharedDeviceData)));

    emit sceneChanged();
    emit deviceAdded(dev->deviceData());

    return dev;
}

bool Diagram::addConnection(Connection* conn)
{
    if (!conn)
        return false;

    scene_->addItem(conn);
    emit sceneChanged();
    emit connectionAdded(conn->connectionData());
    return true;
}

void Diagram::saveClickPos(const QPoint& pos)
{
    prev_event_pos_ = mapToScene(pos);
    prev_click_pos_ = prev_event_pos_;
}

bool Diagram::setDeviceData(const SharedDeviceData& data)
{
    if (!data)
        return false;

    auto dev = devices_.find(data->id());
    if (!dev) {
        qWarning() << "device not found:" << data->id();
        return false;
    }

    const bool title_update = (dev->deviceData()->title() != data->title());

    auto battery_change = dev->deviceData()->calcBatteryChange(*data);

    dev->setDeviceData(data);
    emit sceneChanged();
    emit deviceUpdated(dev->deviceData());

    if (battery_change)
        emit batteryChanged(battery_change);

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
        background_->setPos(-background_->boundingRect().width() / 2, -background_->boundingRect().height() / 2);
        scene_->addItem(background_);
        if (!background_->isEmpty()) {
            emit sceneChanged();
            return true;
        } else {
            qWarning() << __FUNCTION__ << "can't set background image:" << path;
            return false;
        }
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

        auto fmt_vers = app.value(JSON_KEY_FORMAT_VERSION).toInt();

        if (fmt_vers > app_file_format_version()) {
            qWarning() << "the document was created with more recent version, then the current one, "
                          "some feature can be missing...";
        }
    }

    // load devices
    auto devs = root.value(JSON_KEY_DEVICES);
    if (devs.isArray()) {
        auto arr = devs.toArray();
        for (const auto& j : arr)
            devices_.add(Device::datafromJson(j));
    }

    // load connections
    auto cons = root.value(JSON_KEY_CONNS);
    if (cons.isArray()) {
        auto arr = cons.toArray();
        for (const auto& j : arr) {
            auto conn_data = ConnectionData::fromJson(j);
            if (conn_data)
                addConnection(new Connection(conn_data.value()));
        }

        updateConnectionsPos();
    }

    if (root.contains(JSON_KEY_BACKGROUND)) {
        auto bg_img = DiagramImage::fromJson(root.value(JSON_KEY_BACKGROUND));
        if (bg_img) {
            background_ = bg_img.release();
            scene_->addItem(background_);
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

void Diagram::zoomFit()
{
    auto grid_show = grid_->isVisible();
    if (grid_show)
        grid_->setVisible(false);

    fitInView(devices_.boundingRect(), Qt::KeepAspectRatio);

    if (grid_show)
        grid_->setVisible(true);
}

void Diagram::setGridVisible(bool value)
{
    grid_->setVisible(value);
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
    auto sel_data = devices_.selectedDataList();
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
        for (auto x : scene_->items())
            x->setCacheMode(QGraphicsItem::NoCache);

        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        // save scene_ rect
        auto scene_rect = scene_->sceneRect();
        // update scene rect
        scene_->setSceneRect(scene_->itemsBoundingRect());
        // render
        scene_->render(&painter);
        // restore scene rect
        scene_->setSceneRect(scene_rect);

        for (auto x : scene_->items())
            x->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
    }
}

void Diagram::clearAll()
{
    devices_.clear();

    for (auto x : items()) {
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

void Diagram::removeDeviceConnections(DeviceId id)
{
    for (auto conn : connections()) {
        if (conn) {
            scene_->removeItem(conn);
            delete conn;
        }
    }
}

void Diagram::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
        auto numPixels = event->pixelDelta();
        auto numDegrees = event->angleDelta();

        qreal factor = 1;
        if (!numPixels.isNull()) {
            if (numPixels.y() > 0) {
                factor = 1.02;
            } else {
                factor = 0.98;
            }
        } else if (!numDegrees.isNull()) {
            if (numDegrees.y() > 0) {
                factor = 1.01;
            } else {
                factor = 0.99;
            }
        }

        updateZoom(zoom_ * factor);
    } else
        QGraphicsView::wheelEvent(event);
}

QJsonObject Diagram::toJson() const
{
    QJsonObject json;

    QJsonArray devs;
    devices_.foreachDevice([&devs](const Device* dev) {
        devs.append(dev->toJson());
    });

    json[JSON_KEY_DEVICES] = devs;

    QJsonArray cons;
    for (auto c : connections()) {
        if (c->checkConnectedElements()) {
            cons.append(c->connectionData().toJson());
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

void Diagram::createAxis(const QRectF& rect)
{
    if (!grid_)
        return;

    QPen pen(QColor(100, 100, 100));
    pen.setWidth(0);

    auto x_axis = new QGraphicsLineItem;
    x_axis->setPen(pen);
    x_axis->setLine(QLine(QPoint(rect.left(), 0), QPoint(rect.right(), 0)));
    grid_->addToGroup(x_axis);

    auto y_axis = new QGraphicsLineItem;
    y_axis->setPen(pen);
    y_axis->setLine(QLine(QPoint(0, rect.top()), QPoint(0, rect.bottom())));
    grid_->addToGroup(y_axis);
}

void Diagram::createGrid(const QRectF& rect)
{
    if (!grid_)
        return;

    QPen pen(QColor(100, 100, 100, 100));
    pen.setWidth(0);

    for (int i = 0; i < rect.width() / 50; i++) {
        auto x = 50 * (int(rect.left() + i * 50) / 50);
        auto p0 = QPoint(x, rect.top());
        auto p1 = QPoint(x, rect.bottom());
        auto line = new QGraphicsLineItem;
        line->setPen(pen);
        line->setLine(QLine(p0, p1));
        grid_->addToGroup(line);
    }

    for (int i = 0; i < rect.height() / 50; i++) {
        auto y = 50 * (int(rect.top() + i * 50) / 50);
        auto p0 = QPoint(rect.left(), y);
        auto p1 = QPoint(rect.right(), y);
        auto line = new QGraphicsLineItem;
        line->setPen(pen);
        line->setLine(QLine(p0, p1));
        grid_->addToGroup(line);
    }
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

            if (xlet) {
                bool disconnect = event->modifiers().testFlag(Qt::AltModifier);
                if (disconnect) {
                    state_machine_.setState(DiagramState::Init);
                    cmdDisconnectXlet(xlet.value());
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

            if (scene_->selectedItems().empty()) {
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
        if (xlet && conn_start_) {
            if (!isValidConnection(conn_start_.value(), xlet.value()))
                return;

            if (xlet->type() == XletType::Out)
                cmdConnectDevices(ConnectionData(xlet->id(), xlet->index(), conn_start_->id(), conn_start_->index()));
            else if (xlet->type() == XletType::In)
                cmdConnectDevices(ConnectionData(conn_start_->id(), conn_start_->index(), xlet->id(), xlet->index()));
        }

        conn_start_ = {};
    } break;
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

    QMenu menu(this);
    menu.addAction(add_act);

    if (background_ && !background_->isEmpty()) {
        auto clear_bg = new QAction(tr("&Clear background"), this);
        connect(clear_bg, &QAction::triggered, this,
            [this]() { clearBackground(); });

        menu.addAction(clear_bg);
    } else {
        auto set_bg = new QAction(tr("&Set background"), this);
        connect(set_bg, &QAction::triggered, this,
            [this]() { emit requestBackgroundChange(); });

        menu.addAction(set_bg);
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

std::optional<XletInfo> Diagram::hoverDeviceXlet(const QList<QGraphicsItem*>& devs, const QPoint& pt) const
{
    Device* dev = nullptr;
    for (auto x : devs) {
        dev = qgraphicsitem_cast<Device*>(x);
        if (dev)
            break;
    }

    if (!dev)
        return {};

    auto in = dev->inletAt(mapToScene(pt));
    if (in >= 0) {
        return XletInfo { dev->id(), in, XletType::In };
    }

    auto out = dev->outletAt(mapToScene(pt));
    if (out >= 0) {
        return XletInfo { dev->id(), out, XletType::Out };
    }

    return {};
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
    if (devices_.moveSelectedBy(dx, dy)) {
        emit sceneChanged();
        updateConnectionsPos();
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

QImage Diagram::toImage() const
{
    QImage image(scene_->itemsBoundingRect().size().toSize() * 4, QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    const QSignalBlocker block(scene_);
    // save scene_ rect
    auto scene_rect = scene_->sceneRect();
    auto brush = scene_->backgroundBrush();
    // update scene rect
    scene_->setSceneRect(scene_->itemsBoundingRect());
    scene_->setBackgroundBrush(Qt::white);
    // render
    scene_->render(&painter);
    // restore scene rect
    scene_->setSceneRect(scene_rect);
    scene_->setBackgroundBrush(brush);
    return image;
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

    QSignalBlocker db(scene_);

    auto box = scene_->itemsBoundingRect().toRect();
    auto old_rect = scene_->sceneRect();

    svg_gen.setSize(box.size());
    svg_gen.setViewBox(QRect { 0, 0, box.width(), box.height() });
    svg_gen.setTitle("PatchScheme connection diagram");
    svg_gen.setResolution(72);
    svg_gen.setDescription(QString("create with PatchScene v%1").arg(app_version()));

    QPainter painter(&svg_gen);

    scene_->setSceneRect(box);
    for (auto x : scene_->items())
        x->setCacheMode(QGraphicsItem::NoCache);

    scene_->render(&painter);
    painter.end();

    for (auto x : scene_->items())
        x->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    scene_->setSceneRect(old_rect);

    return { buf.data(), box.size() };
}

bool Diagram::isGridVisible() const
{
    return (grid_ && grid_->isVisible());
}

void Diagram::clearUndoStack()
{
    if (undo_stack_)
        undo_stack_->clear();
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
    if (src.id() == dest.id()) {
        qWarning() << "self connection attempt";
        return false;
    }

    if (src.type() == dest.type()) {
        qWarning() << "same xlet type connection attempt";
        return false;
    }

    auto data = (src.type() == XletType::Out)
        ? ConnectionData(src.id(), src.index(), dest.id(), dest.index())
        : ConnectionData(dest.id(), dest.index(), src.id(), src.index());

    for (auto x : scene_->items()) {
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

bool Diagram::dropJson(const QPointF& pos, const QByteArray& json)
{
    SharedDeviceData data(new DeviceData(DEV_NULL_ID));
    if (!data->setJson(json)) {
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

    obj[JSON_KEY_VERSION] = app_version();
    obj[JSON_KEY_VERSION_MAJOR] = app_version_major();
    obj[JSON_KEY_VERSION_MINOR] = app_version_minor();
    obj[JSON_KEY_VERSION_PATCH] = app_version_patch();
    obj[JSON_KEY_VERSION_GIT] = app_git_version();
    obj[JSON_KEY_FORMAT_VERSION] = app_file_format_version();

    return obj;
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
            if (conn->relatesToDevice(id))
                res.push_back(conn->connectionData());
        }
    }

    return res;
}

QSet<ConnectionData> Diagram::findSelectedConnections() const
{
    auto dev_ids = devices_.selectedIdList();
    QSet<ConnectionData> res;

    for (auto& x : scene_->items()) {
        auto conn = qgraphicsitem_cast<Connection*>(x);
        if (conn) {
            for (auto id : dev_ids) {
                if (conn->relatesToDevice(id))
                    res.insert(conn->connectionData());
            }
        }
    }

    return res;
}
