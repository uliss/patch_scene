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
#include "scene_item.h"
#include "device_editor.h"
#include "logging.hpp"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QRandomGenerator>

#include <unordered_map>

using namespace ceam;

namespace {

constexpr int MIN_WIDTH = 16;
constexpr int MIN_HEIGHT = 16;
constexpr int DEF_WIDTH = 100;
constexpr int DEF_HEIGHT = 40;

constexpr SceneItemId INIT_ID = 1;
using DeviceIdMap = std::unordered_map<SceneItemId, bool>;

SharedItemData makeDeviceData()
{
    QSharedDataPointer data(new ItemData(SCENE_ITEM_NULL_ID));
    data->setTitle("Device");
    data->appendInput(XletData { ConnectorModel::XLR });
    data->appendInput(XletData { ConnectorModel::XLR });
    data->appendInput(XletData { "MIDI", ConnectorModel::DIN_MIDI });
    data->appendInput(XletData { "USB", ConnectorModel::USB_C });

    data->appendOutput(XletData { "L", ConnectorModel::JACK_TRS });
    data->appendOutput(XletData { "R", ConnectorModel::JACK_TRS });

#if 0
    XletsUserViewData user_view;
    user_view.insertXlet({ 0, 5 }, { 0, XletType::Out });
    user_view.insertXlet({ 0, 4 }, { 0, XletType::In });
    // user_view
    data->userViewData().push_back(user_view);
#endif

    return data;
}

class DeviceIdFactory {
    using key_value = DeviceIdMap::value_type;
    DeviceIdMap ids_;
    DeviceIdFactory() { }

public:
    static DeviceIdFactory& instance()
    {
        static DeviceIdFactory instance_;
        return instance_;
    }

    SceneItemId request()
    {
        if (ids_.empty()) {
            ids_.insert({ INIT_ID, true });
            return INIT_ID;
        }

        // search first not used element
        auto it = std::find_if(ids_.begin(), ids_.end(),
            [](const key_value& kv) {
                return !kv.second;
            });

        if (it != ids_.end()) {
            it->second = true;
            return it->first;
        } else {
            it = std::max_element(ids_.begin(), ids_.end(),
                [](const key_value& a, const key_value& b) {
                    return a.first < b.first;
                });

            if (it == ids_.end()) { // should not happen!
                ids_.insert({ INIT_ID, true });
                return INIT_ID;
            } else {
                auto new_id = it->first + 1;
                qDebug() << "[#id] new device id:" << new_id;
                ids_.insert({ new_id, true });
                return new_id;
            }
        }
    }

    void release(SceneItemId id)
    {
        auto it = std::find_if(ids_.begin(), ids_.end(),
            [id](const key_value& kv) {
                return kv.first == id;
            });

        if (it != ids_.end()) {
            if (it->second) {
                qDebug() << "[#id] release id" << id;
                it->second = false;
            } else
                qWarning() << "[#id] device id already released:" << id;
        } else {
            qWarning() << "[#id] device id not found:" << id;
        }
    }

    void setUsed(SceneItemId id)
    {
        ids_[id] = true;
    }

    bool isUsed(SceneItemId id) const
    {
        return ids_.end() != std::find_if(ids_.begin(), ids_.end(), //
                   [id](const key_value& kv) {
                       return kv.first == id && kv.second;
                   });
    }
};
} // namespace

SceneItem::SceneItem()
    : SceneItem(makeDeviceData())
{
}

SceneItem::SceneItem(const SharedItemData& data)
    : data_(data)
{
    if (data_->isNull() || DeviceIdFactory::instance().isUsed(data_->id())) {
        if (!data_->isNull())
            qDebug() << "device id is used #id" << data_->id();

        data_->setId(DeviceIdFactory::instance().request());
        qDebug() << "create device with new #id" << data_->id();
    } else {
        DeviceIdFactory::instance().setUsed(data_->id());
        qDebug() << "create device with requested #id" << data_->id();
    }

    setPos(data_->pos());

    data_->setZValue(data_->id());
    setZValue(data->zValue());
    setFlag(QGraphicsItem::ItemIsSelectable);
    setCacheMode(DeviceCoordinateCache);
}

SceneItem::~SceneItem()
{
    DeviceIdFactory::instance().release(data_->id());
}

void SceneItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void SceneItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void SceneItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void SceneItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    auto sc = scene();
    if (sc && sc->selectedItems().count() > 1) { // multiple selection
        auto align_hor = new QAction(tr("Align horizontal"), &menu);
        connect(align_hor, SIGNAL(triggered(bool)), this, SIGNAL(alignHorizontal()));

        auto align_ver = new QAction(tr("Align vertical"), &menu);
        connect(align_ver, SIGNAL(triggered(bool)), this, SIGNAL(alignVertical()));

        auto distrib_hor = new QAction(tr("Distribute horizontal"), &menu);
        connect(distrib_hor, SIGNAL(triggered(bool)), this, SIGNAL(distributeHorizontal()));

        auto distrib_ver = new QAction(tr("Distribute vertical"), &menu);
        connect(distrib_ver, SIGNAL(triggered(bool)), this, SIGNAL(distributeVertical()));

        auto place_hor = new QAction(tr("Place in row"), &menu);
        connect(place_hor, SIGNAL(triggered(bool)), this, SIGNAL(placeInRow()));

        auto place_ver = new QAction(tr("Place in column"), &menu);
        connect(place_ver, SIGNAL(triggered(bool)), this, SIGNAL(placeInColumn()));

        menu.addAction(align_hor);
        menu.addAction(align_ver);
        menu.addSeparator();

        if (sc->selectedItems().count() >= 3) {
            menu.addAction(distrib_hor);
            menu.addAction(distrib_ver);
            menu.addSeparator();
        }

        menu.addAction(place_hor);
        menu.addAction(place_ver);

        auto mirror = new QAction(tr("Mirror image"), &menu);
        connect(mirror, &QAction::triggered, this, &SceneItem::mirrorSelected);
        menu.addAction(mirror);

        // lock/unlock
        menu.addSeparator();
        auto lockAct = new QAction(&menu);
        lockAct->setText(tr("Lock"));
        connect(lockAct, &QAction::triggered, this, &SceneItem::lockSelected);
        auto unlockAct = new QAction(&menu);
        unlockAct->setText(tr("Unlock"));
        connect(unlockAct, &QAction::triggered, this, &SceneItem::unlockSelected);

        menu.addAction(lockAct);
        menu.addAction(unlockAct);
    } else { // single item context menu
        createContextMenu(menu);
    }

    menu.exec(event->screenPos());

    event->accept();
}

void SceneItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers().testFlag(Qt::ShiftModifier)) {
        showEditDialog();
        event->accept();
    }
}

void SceneItem::setMenuCaption(QMenu& menu)
{
    auto info = menu.addAction(data_->title());
    info->setDisabled(true);
    auto info_font = info->font();
    info_font.setBold(true);
    info->setFont(info_font);
    menu.setStyleSheet("QMenu::item:disabled {color: black;}");
}

void SceneItem::addLockAction(QMenu& menu)
{
    auto lockAct = new QAction(&menu);
    lockAct->setText(data_->isLocked() ? tr("Unlock") : tr("Lock"));
    connect(lockAct, &QAction::triggered, this, [this](bool) {
        data_->isLocked() ? emit unlock(data_->id()) : emit lock(data_->id());
    });

    menu.addAction(lockAct);
}

QJsonObject SceneItem::toJson() const
{
    auto data_json = itemData()->toJson();
    data_json["z"] = zValue();

    return data_json;
}

void SceneItem::setLocked(bool value)
{
    if (!data_ || data_->isLocked() == value)
        return;

    data_->setLocked(value);
    update();
}

void SceneItem::addDuplicateAct(QMenu& menu)
{
    auto duplicateAct = new QAction(tr("Duplicate"), &menu);
    connect(duplicateAct, &QAction::triggered, this,
        [this]() { emit duplicateDevice(data_); });

    menu.addAction(duplicateAct);
}

void SceneItem::addRemoveAct(QMenu& menu)
{
    auto removeAct = new QAction(tr("Delete"), &menu);
    connect(removeAct, &QAction::triggered, this,
        [this]() { emit removeDevice(data_); });

    menu.addAction(removeAct);
}

void SceneItem::addZValueAction(QMenu& menu)
{
    auto moveUpAct = new QAction(tr("Move up"), &menu);
    connect(moveUpAct, &QAction::triggered, this,
        [this]() { emit moveUpper(data_); });
    menu.addAction(moveUpAct);

    auto moveDownAct = new QAction(tr("Move down"), &menu);
    connect(moveDownAct, &QAction::triggered, this,
        [this]() { emit moveLower(data_); });
    menu.addAction(moveDownAct);
}

void SceneItem::addPropertiesAct(QMenu& menu)
{
    auto propertiesAct = new QAction(tr("Properties"), &menu);
    connect(propertiesAct, &QAction::triggered, this,
        [this]() {
            std::unique_ptr<DeviceEditor> dialog(new DeviceEditor(data_));
            connect(dialog.get(), &DeviceEditor::acceptData, this, &SceneItem::updateDevice);
            dialog->exec();
        });

    menu.addAction(propertiesAct);
}

void SceneItem::createContextMenu(QMenu& menu)
{
    addDuplicateAct(menu);
    addRemoveAct(menu);

    menu.addSeparator();
    addPropertiesAct(menu);
}

void SceneItem::showEditDialog()
{
    // reimplement this
}

SharedItemData SceneItem::defaultDeviceData()
{
    return makeDeviceData();
}

SharedItemData SceneItem::dataFromJson(const QJsonValue& j)
{
    if (!j.isObject()) {
        WARN() << "not a object" << j;
        return {};
    }

    SharedItemData data(new ItemData(SCENE_ITEM_NULL_ID));
    if (!data->setJson(j))
        return {};

    if (data->isNull()) {
        data->setId(DeviceIdFactory::instance().request());
    } else if (DeviceIdFactory::instance().isUsed(data->id())) {
        WARN() << "device id is already used:" << data->id();
        data->setId(DeviceIdFactory::instance().request());
    }

    return data;
}

SharedItemData SceneItem::itemData() const
{
    auto dev_pos = pos();

    if (data_->pos() != dev_pos)
        data_->setPos(dev_pos);

    return data_;
}

bool SceneItem::setItemData(const SharedItemData& data)
{
    if (data->isNull()) {
        WARN() << "NULL data";
        return false;
    }

    data_ = data;
    return true;
}

std::optional<QPointF> SceneItem::connectionPoint(XletIndex i, XletType type, bool map) const
{
    return {};
}

void SceneItem::randomizePos(qint64 delta)
{
    auto value = qAbs(delta);
    auto dx = QRandomGenerator::global()->bounded(-value, value);
    auto dy = QRandomGenerator::global()->bounded(-value, value);

    setPos(x() + dx, y() + dy);
}

void SceneItem::paintStateIcons(QPainter* painter, const QPointF& pos)
{
    if (isLocked()) {
        painter->save();
        constexpr qreal WD = 8;
        constexpr qreal HT = WD - 2;
        constexpr qreal AWD = WD - 2;
        painter->translate(pos);
        QColor c(100, 100, 100);
        painter->setBrush(QBrush(c));
        painter->setPen(QPen(c, 1.5));
        painter->drawArc(QRectF { (WD - AWD) * 0.5, 0.5 * HT, AWD, HT }, 0, 180 * 16);
        painter->drawRect(QRectF { 0, 1.4 * HT, WD, HT });
        painter->restore();
    }
}
