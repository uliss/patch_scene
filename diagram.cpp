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

constexpr const char* JSON_KEY_META = "meta";

} // namespace

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
    initScale();
}

void Diagram::initScale()
{
    scale_ = new ScaleWidget(this);
    scale_->setPos({ 20, 20 });
    scale_->show();
    connect(this, SIGNAL(zoomChanged(qreal)), scale_, SLOT(setScale(qreal)));
}

void Diagram::initGraphicsScene(int w, int h)
{
    diagram_scene_ = new DiagramScene(w, h, this);
    connect(diagram_scene_, &DiagramScene::removeConnection, this, &Diagram::cmdDisconnectDevices);
    setScene(diagram_scene_);

#define SIGNAL_PASS(signame) connect(diagram_scene_, &DiagramScene::signame, this, &Diagram::signame)

    // signal transfer
    SIGNAL_PASS(addToFavorites);
    SIGNAL_PASS(batteryChanged);
    SIGNAL_PASS(canRedoChanged);
    SIGNAL_PASS(canUndoChanged);
    SIGNAL_PASS(connectionAdded);
    SIGNAL_PASS(connectionRemoved);
    SIGNAL_PASS(deviceAdded);
    SIGNAL_PASS(deviceRemoved);
    SIGNAL_PASS(deviceTitleUpdated);
    SIGNAL_PASS(deviceUpdated);
    SIGNAL_PASS(requestBackgroundChange);
    SIGNAL_PASS(sceneChanged);
    SIGNAL_PASS(sceneClearAll);
    SIGNAL_PASS(sceneFullUpdate);
    SIGNAL_PASS(showBackgroundChanged);
    SIGNAL_PASS(showCablesChanged);
    SIGNAL_PASS(fileFormatVersionMismatch);
}

bool Diagram::removeItem(SceneItemId id)
{
    return diagram_scene_->removeSceneItem(id);
}

void Diagram::cmdRemoveSelected()
{
    diagram_scene_->cmdRemoveSelected();
}

void Diagram::cmdRemoveItem(const SharedItemData& data)
{
    diagram_scene_->cmdRemoveItem(data);
}

void Diagram::cmdUpdateItem(const SharedItemData& data)
{
    diagram_scene_->cmdUpdateItem(data);
}

void Diagram::cmdZoomInSelected()
{
    auto zoom = new ZoomSelected(this, 1.25);
    diagram_scene_->undoStack()->push(zoom);
}

void Diagram::cmdZoomOutSelected()
{
    auto zoom = new ZoomSelected(this, 1 / 1.25);
    diagram_scene_->undoStack()->push(zoom);
}

void Diagram::cmdMoveLower(const SharedItemData& data)
{
    diagram_scene_->cmdMoveLower(data);
}

void Diagram::cmdMoveUpper(const SharedItemData& data)
{
    diagram_scene_->cmdMoveUpper(data);
}

void Diagram::cmdCreateDevice(const QPointF& pos)
{
    diagram_scene_->cmdCreateDevice(pos);
}

void Diagram::cmdToggleSelected(const QList<QGraphicsItem*>& items)
{
    diagram_scene_->cmdToggleSelected(items);
}

void Diagram::cmdConnectDevices(const ConnectionId& conn)
{
    diagram_scene_->cmdConnectDevices(conn);
}

void Diagram::cmdCreateComment(const QPointF& pos)
{
    diagram_scene_->cmdCreateComment(pos);
}

void Diagram::cmdDisconnectDevices(const ConnectionId& conn)
{
    cmdDisconnectXlet(conn.sourceInfo());
}

void Diagram::cmdDuplicateItems(const SharedItemData& data)
{
    diagram_scene_->cmdDuplicateItem(data);
}

void Diagram::cmdDuplicateSelected()
{
    diagram_scene_->cmdDuplicateSelected();
}

void Diagram::cmdLockSelected()
{
    diagram_scene_->cmdLockSelected();
}

void Diagram::cmdUnlockSelected()
{
    diagram_scene_->cmdUnlockSelected();
}

void Diagram::cmdLock(SceneItemId id)
{
    diagram_scene_->cmdLock(id);
}

void Diagram::cmdUnlock(SceneItemId id)
{
    diagram_scene_->cmdUnlock(id);
}

void Diagram::cmdMirrorDevice(SceneItemId id)
{
    diagram_scene_->cmdMirrorDevice(id);
}

void Diagram::cmdMirrorSelected()
{
    diagram_scene_->cmdMirrorSelected();
}

void Diagram::cmdSelectAll()
{
    auto sel = new AddToSelected(diagram_scene_, itemScene().idList());
    diagram_scene_->undoStack()->push(sel);
}

void Diagram::cmdSelectItems(const QRectF& sel)
{
    diagram_scene_->cmdSelectItems(sel);
}

void Diagram::cmdSelectUnique(SceneItemId id)
{
    diagram_scene_->cmdSelectUnique(id);
}

void Diagram::cmdDisconnectXlet(const XletInfo& xi)
{
    diagram_scene_->cmdDisconnectXlet(xi);
}

void Diagram::cmdDistributeHSelected()
{
    diagram_scene_->cmdDistributeHSelected();
}

void Diagram::cmdDistributeVSelected()
{
    diagram_scene_->cmdDistributeVSelected();
}

void Diagram::cmdMoveSelectedItemsBy(qreal dx, qreal dy)
{
    diagram_scene_->cmdMoveSelectedItemsBy(dx, dy);
}

void Diagram::cmdMoveSelectedItemsFrom(const QPointF& from, const QPointF& to)
{
    diagram_scene_->cmdMoveSelectedItemsFrom(from, to);
}

void Diagram::cmdCutSelected()
{
    auto cut = new CutSelected(this);
    diagram_scene_->undoStack()->push(cut);
}

void Diagram::cmdPaste()
{
    auto paste = new PasteFromClipBuffer(this);
    diagram_scene_->undoStack()->push(paste);
}

void Diagram::cmdPlaceInColumnSelected()
{
    diagram_scene_->cmdPlaceInColumnSelected();
}

void Diagram::cmdPlaceInRowSelected()
{
    diagram_scene_->cmdPlaceInRowSelected();
}

void Diagram::cmdReconnectDevice(const ConnectionInfo& old_conn, const ConnectionInfo& new_conn)
{
    diagram_scene_->cmdReconnectDevice(old_conn, new_conn);
}

SceneItem* Diagram::addItem(const SharedItemData& data)
{
    return diagram_scene_->addSceneItem(data);
}

bool Diagram::setItemData(const SharedItemData& data)
{
    return diagram_scene_->setItemData(data);
}

void Diagram::setShowCables(bool value)
{
    show_cables_ = value;
    connections()->setVisible(value);
}

void Diagram::setShowPeople(bool value)
{
    itemScene().foreachItem([value](SceneItem* item) {
        if (item //
            && item->itemData() //
            && item->itemData()->category() == ItemCategory::Human) {
            item->setVisible(value);
        }
    });
}

void Diagram::setShowFurniture(bool value)
{
    itemScene().foreachItem([value](SceneItem* item) {
        if (item //
            && item->itemData() //
            && item->itemData()->category() == ItemCategory::Furniture) {
            item->setVisible(value);
        }
    });
}

void Diagram::setShowBackground(bool value)
{
    diagram_scene_->setShowBackground(value);
}

bool Diagram::setBackground(const QString& path)
{
    return diagram_scene_->setBackground(path);
}

bool Diagram::loadJson(const QString& path)
{
    return diagram_scene_->loadJson(path, JSON_KEY_META, meta_);
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
    auto rect = diagram_scene_->bestFitRect();
    if (rect.isEmpty())
        return;

    fitRect(rect);
}

void Diagram::zoomFitSelected()
{
    auto rect = itemScene().boundingSelectRect();
    if (rect.isEmpty())
        return;

    fitRect(rect);
}

DiagramState Diagram::state() const
{
    return diagram_scene_->state();
}

void Diagram::setGridVisible(bool value)
{
    diagram_scene_->setGridVisible(value);
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
    diagram_scene_->undoStack()->undo();
}

void Diagram::redo()
{
    diagram_scene_->undoStack()->redo();
}

void Diagram::copySelected()
{
    auto sel_data = itemScene().selectedDataList();
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
        diagram_scene_->printDiagram(&printer);
    }
}

void Diagram::printScheme(QPrinter* printer) const
{
    diagram_scene_->printDiagram(printer);
}

void Diagram::renderToSvg(const QString& filename, const QString& title) const
{
    auto grid = diagram_scene_->gridVisible();
    if (grid)
        diagram_scene_->setGridVisible(false);

    QSvgGenerator svg_gen;
    svg_gen.setFileName(filename);
    svg_gen.setTitle(title);
    svg_gen.setDescription(tr("Generated with PatchScene"));
    auto svg_size = itemScene().boundingRect().size();
    svg_gen.setSize(svg_size.toSize());
    svg_gen.setViewBox({ { 0, 0 }, svg_size });
    QPainter p(&svg_gen);

    diagram_scene_->renderDiagram(&p);

    if (grid)
        diagram_scene_->setGridVisible(true);

    p.end();
}

void Diagram::renderToPng(const QString& filename) const
{
    auto grid = diagram_scene_->gridVisible();
    if (grid)
        diagram_scene_->setGridVisible(false);

    auto img_size = itemScene().boundingRect().size().toSize() * 4;
    QImage img(img_size, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    p.setRenderHint(QPainter::Antialiasing, true);

    diagram_scene_->renderDiagram(&p);

    if (grid)
        diagram_scene_->setGridVisible(true);

    p.end();

    img.save(filename, "png");
}

void Diagram::clearAll()
{
    diagram_scene_->clearAll();
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
    auto json = diagram_scene_->toJson();
    json[JSON_KEY_META] = meta_.toJson();
    return json;
}

void Diagram::contextMenuEvent(QContextMenuEvent* event)
{
    // call default implementation
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

    diagram_scene_->background().addToContextMenu(menu);

    if (itemScene().selectedCount() >= 2) {
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

        if (itemScene().selectedCount() >= 3) {
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

bool Diagram::connectDevices(const ConnectionId& id, std::optional<ConnectionViewData> viewData)
{
    if (!itemScene().findData(id.source())) {
        WARN() << "connection source not found: " << id.source();
        return false;
    }

    if (!itemScene().findData(id.destination())) {
        WARN() << "connection destination not found: " << id.destination();
        return false;
    }

    auto conn = connections()->add(id);
    if (conn) {
        if (!viewData) {
            // updateConnectionStyle(conn);
        } else
            conn->setViewData(*viewData);

        // updateConnectionPos(conn);
        emit sceneChanged();
        return true;
    } else {
        WARN() << "can't connect:" << id;
        return false;
    }
}

SceneConnections* Diagram::connections()
{
    return diagram_scene_->connections();
}

const SceneConnections* Diagram::connections() const
{
    return diagram_scene_->connections();
}

Scene& Diagram::itemScene()
{
    return diagram_scene_->itemScene();
}

const Scene& Diagram::itemScene() const
{
    return diagram_scene_->itemScene();
}

void Diagram::moveSelectedItemsBy(qreal dx, qreal dy)
{
    diagram_scene_->moveSelectedItemsBy(dx, dy);
}

void Diagram::moveItemsBy(const QHash<SceneItemId, QPointF>& deltas)
{
    diagram_scene_->moveItemsBy(deltas);
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
    return diagram_scene_->renderToImage(4);
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

    auto items_bbox = diagram_scene_->bestFitRect().toRect();

    svg_gen.setSize(items_bbox.size());
    svg_gen.setViewBox(QRect { 0, 0, items_bbox.width(), items_bbox.height() });
    svg_gen.setTitle("PatchScheme connection diagram");
    svg_gen.setResolution(72);
    svg_gen.setDescription(QString("create with PatchScene v%1").arg(app_version()));

    QPainter painter(&svg_gen);
    diagram_scene_->renderDiagram(&painter, items_bbox);
    painter.end();

    return { buf.data(), items_bbox.size() };
}

bool Diagram::gridIsVisible() const
{
    return diagram_scene_->gridVisible();
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
