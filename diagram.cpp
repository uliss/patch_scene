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
#include "diagram_scene.h"
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
    }
};
}

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
    initSceneConnections();

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
    tmp_connection_ = new QGraphicsLineItem();
    tmp_connection_->setZValue(ZVALUE_LIVE_CONN);
    tmp_connection_->setVisible(false);
    scene_->addItem(tmp_connection_);
}

void Diagram::initSceneConnections()
{
    connections_.setScene(scene_);
    connect(&connections_, SIGNAL(added(ConnectionData)), this, SIGNAL(connectionAdded(ConnectionData)));
    connect(&connections_, SIGNAL(removed(ConnectionData)), this, SIGNAL(connectionRemoved(ConnectionData)));
    connect(&connections_, SIGNAL(visibleChanged(bool)), this, SIGNAL(showCablesChanged(bool)));
}

void Diagram::initScene(int w, int h)
{
    scene_ = new DiagramScene(w, h, this);
    connect(scene_, SIGNAL(removeConnection(ConnectionData)), this, SLOT(cmdDisconnectDevices(ConnectionData)));
    setScene(scene_);

    // NB: should be called after setScene(scene_)!
    scene_->initGrid();

    devices_.setScene(scene_);
}

bool Diagram::removeDevice(DeviceId id)
{
    auto data = devices_.remove(id);
    if (data) {
        connections_.removeAll(id);
        emit deviceRemoved(data);
        emit sceneChanged();
    }

    return true;
}

void Diagram::updateConnectionPos(Connection* conn)
{
    if (!conn)
        return;

    auto conn_pos = devices_.connectionPoints(conn->connectionData());
    if (conn_pos)
        conn->setPoints(conn_pos->first, conn_pos->second);
    else
        qWarning() << __FUNCTION__ << "connection points not found";
}

void Diagram::updateConnectionPos(DeviceId id)
{
    for (auto conn : connections_.findConnections(id))
        updateConnectionPos(conn);
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

void Diagram::cmdDisconnectDevices(const ConnectionData& conn)
{
    cmdDisconnectXlet(conn.sourceInfo());
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

    QHash<DeviceId, QPointF> deltas;
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

    QHash<DeviceId, QPointF> deltas;
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

    connect(dev, SIGNAL(alignHorizontal()), this, SLOT(cmdAlignHSelected()));
    connect(dev, SIGNAL(alignVertical()), this, SLOT(cmdAlignVSelected()));

    emit sceneChanged();
    emit deviceAdded(dev->deviceData());

    return dev;
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

    updateConnectionPos(data->id());

    return true;
}

void Diagram::setShowCables(bool value)
{
    show_cables_ = value;
    connections_.setVisible(value);
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
            addDevice(Device::datafromJson(j));
    }

    // load connections
    auto cons = root.value(JSON_KEY_CONNS);
    if (cons.isArray()) {
        auto arr = cons.toArray();
        for (const auto& j : arr) {
            auto conn_data = ConnectionData::fromJson(j);
            if (conn_data)
                connectDevices(conn_data.value());
        }
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
    fitInView(scene_->bestFitRect(), Qt::KeepAspectRatio);
}

void Diagram::setGridVisible(bool value)
{
    scene_->setGridVisible(value);
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
        scene_->printDiagram(&printer);
    }
}

void Diagram::clearAll()
{
    devices_.clear();

    QSignalBlocker conn_block(&connections_);
    connections_.clear();

    if (background_) {
        delete background_;
        background_ = nullptr;
    }

    emit sceneClearAll();
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

    connections_.foreachData([this, &cons](const ConnectionData& conn) {
        if (devices_.checkConnection(conn))
            cons.append(conn.toJson());
    });

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
        tmp_connection_->setVisible(false);
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

        auto dest_pos = mapToScene(event->pos());
        auto delta = prev_click_pos_ - dest_pos;
        devices_.foreachSelectedDevice([delta](Device* dev) {
            dev->moveBy(delta.x(), delta.y());
        });

        cmdMoveSelectedDevicesFrom(prev_click_pos_, dest_pos);
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
        tmp_connection_->setVisible(false);
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

    if (devices_.selectedCount() >= 2) {
        menu.addSeparator();

        auto hor_align = new QAction(tr("Align &horizontal"), this);
        connect(hor_align, SIGNAL(triggered(bool)), this, SLOT(cmdAlignHSelected()));
        menu.addAction(hor_align);

        auto ver_align = new QAction(tr("Align &vertical"), this);
        connect(ver_align, SIGNAL(triggered(bool)), this, SLOT(cmdAlignVSelected()));
        menu.addAction(ver_align);
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
    auto conn = connections_.add(data);
    if (conn) {
        updateConnectionPos(conn);
        emit sceneChanged();
        return true;
    } else
        return false;
}

bool Diagram::disconnectDevices(const ConnectionData& data)
{
    if (connections_.remove(data.sourceInfo())) {
        emit sceneChanged();
        return true;
    } else
        return false;
}

void Diagram::moveSelectedItemsBy(qreal dx, qreal dy)
{
    if (devices_.moveSelectedBy(dx, dy)) { // O(N)
        emit sceneChanged();

        // O(N)
        devices_.foreachSelectedData([this](const SharedDeviceData& data) {
            updateConnectionPos(data->id());
        });
    }
}

void Diagram::moveItemsBy(const QHash<DeviceId, QPointF>& deltas)
{
    devices_.moveBy(deltas);

    for (auto kv : deltas.asKeyValueRange())
        updateConnectionPos(kv.first);
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
    return scene_->renderToImage(4);
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

    auto items_bbox = scene_->bestFitRect().toRect();

    svg_gen.setSize(items_bbox.size());
    svg_gen.setViewBox(QRect { 0, 0, items_bbox.width(), items_bbox.height() });
    svg_gen.setTitle("PatchScheme connection diagram");
    svg_gen.setResolution(72);
    svg_gen.setDescription(QString("create with PatchScene v%1").arg(app_version()));

    QPainter painter(&svg_gen);
    scene_->renderDiagram(&painter, items_bbox);
    painter.end();

    return { buf.data(), items_bbox.size() };
}

bool Diagram::gridVisible() const
{
    return scene_->gridVisible();
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

    if (connections_.findConnection(data.sourceInfo())) {
        qWarning() << "already connected from this source";
        return false;
    }

    if (connections_.findConnection(data.destinationInfo())) {
        qWarning() << "already connected to this destination";
        return false;
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

QSet<ConnectionData> Diagram::findSelectedConnections() const
{
    QSet<ConnectionData> res;

    for (auto id : devices_.selectedIdList()) {
        for (auto& data : connections_.findConnectionsData(id)) {
            res << data;
        }
    }

    return res;
}
