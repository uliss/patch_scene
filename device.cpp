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
#include "device.h"
#include "deviceproperties.h"
#include "svg_render_factory.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QRandomGenerator>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>

#include <unordered_map>

using namespace ceam;

namespace {
constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

constexpr int MIN_WIDTH = 16;
constexpr int MIN_HEIGHT = 16;
constexpr int DEF_WIDTH = 100;
constexpr int DEF_HEIGHT = 40;

constexpr int TITLE_MIN_CHAR_WIDTH = 8;
constexpr int TITLE_MAX_CHAR_WIDTH = 18;
constexpr int TITLE_CHAR_WIDTH = 10;
constexpr qreal DEF_IMAGE_WIDTH = 100;

constexpr DeviceId INIT_ID = 1;
using DeviceIdMap = std::unordered_map<DeviceId, bool>;

SharedDeviceData makeDeviceData()
{
    QSharedDataPointer data(new DeviceData(DEV_NULL_ID));
    data->setTitle("Device");
    data->appendInput(XletData { ConnectorModel::XLR });
    data->appendInput(XletData { ConnectorModel::XLR });
    data->appendInput(XletData { "MIDI", ConnectorModel::DIN_MIDI });
    data->appendInput(XletData { "USB", ConnectorModel::USB_C });

    data->appendOutput(XletData { "L", ConnectorModel::JACK_TRS });
    data->appendOutput(XletData { "R", ConnectorModel::JACK_TRS });
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

    DeviceId request()
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

    void release(DeviceId id)
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

    void setUsed(DeviceId id)
    {
        ids_[id] = true;
    }

    bool isUsed(DeviceId id) const
    {
        return ids_.end() != std::find_if(ids_.begin(), ids_.end(), //
                   [id](const key_value& kv) {
                       return kv.first == id && kv.second;
                   });
    }
};
}

Device::Device()
    : Device(makeDeviceData())
{
}

Device::Device(const SharedDeviceData& data)
    : data_(data)
    , title_ { nullptr }
    , image_ { nullptr }
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
    setZValue(data_->id());
    setFlag(QGraphicsItem::ItemIsSelectable);

    syncRect();
}

Device::~Device()
{
    DeviceIdFactory::instance().release(data_->id());
}

QRectF Device::boundingRect() const
{
    return rect_;
}

QPointF Device::inletPos(int i, bool map) const
{
    const auto NUM_IN = visInletCount();
    if (NUM_IN == 0)
        return {};

    const auto bbox = xletRect();
    const auto xstep = bbox.width() / NUM_IN;
    const auto xoff = (i + 0.5) * xstep;

    const auto pos = QPointF(bbox.left() + xoff, bbox.top());

    return map ? mapToScene(pos) : pos;
}

QPointF Device::outletPos(int i, bool map) const
{
    const auto NUM_OUT = visOutletCount();
    if (NUM_OUT == 0)
        return {};

    const auto bbox = xletRect();
    const auto xstep = bbox.width() / NUM_OUT;
    const auto xoff = (i + 0.5) * xstep;

    const auto pos = QPointF(bbox.left() + xoff, bbox.bottom());

    return map ? mapToScene(pos) : pos;
}

QRect Device::inletRect(int i) const
{
    auto pos = inletPos(i);
    return QRect(pos.x() - (XLET_W / 2), pos.y(), XLET_W, XLET_H);
}

QRect Device::outletRect(int i) const
{
    auto pos = outletPos(i);
    return QRect(pos.x() - (XLET_W / 2), pos.y() - XLET_H, XLET_W, XLET_H);
}

void Device::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    paintTitleBox(painter);

    QPen pen(Qt::SolidPattern, 0);

    if (option->state & QStyle::State_Selected) {
        pen.setColor(Qt::blue);
        painter->setPen(pen);

        if (!noXlets()) {
            painter->setBrush(QColor(255, 255, 255));
            painter->drawRect(xletRect());
        }

        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRect(boundingRect());
    } else if (!noXlets()) { // draw xlet box
        pen.setColor(Qt::black);
        painter->setPen(pen);
        painter->setBrush(QColor(255, 255, 255));
        painter->drawRect(xletRect());
    }

    Q_UNUSED(widget);
}

void Device::paintTitleBox(QPainter* painter)
{
    if (!title_ || !data_->showTitle())
        return;

    painter->setPen(Qt::NoPen);
    switch (data_->category()) {
    case ItemCategory::Send:
        painter->setBrush(QColor::fromRgb(0xD7, 0xF4, 0xf5, 200));
        break;
    case ItemCategory::Return:
        painter->setBrush(QColor::fromRgb(0xff, 0xd1, 0x71, 200));
        break;
    default:
        painter->setBrush(QColor::fromRgbF(0.9, 0.9, 0.9, 0.6));
        break;
    }

    auto title_box = titleRect();
    painter->drawRoundedRect(title_box.adjusted(4, 3, -4, -4), 5, 5);
}

void Device::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void Device::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void Device::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    Q_UNUSED(event);
}

void Device::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    QMenu menu;

    auto duplicateAct = new QAction(tr("Duplicate"), &menu);
    connect(duplicateAct, &QAction::triggered, this,
        [this]() { emit duplicateDevice(data_); });

    auto removeAct = new QAction(tr("Delete"), &menu);
    connect(removeAct, &QAction::triggered, this,
        [this]() { emit removeDevice(data_); });

    auto addToFavoritesAct = new QAction(tr("Add to favorites"), &menu);
    connect(addToFavoritesAct, &QAction::triggered, this,
        [this]() { emit addToFavorites(data_); });

    auto propertiesAct = new QAction(tr("Properties"), &menu);
    connect(propertiesAct, &QAction::triggered, this,
        [this]() {
            std::unique_ptr<DeviceProperties> dialog(new DeviceProperties(data_));
            connect(dialog.get(), SIGNAL(acceptData(SharedDeviceData)), this, SIGNAL(updateDevice(SharedDeviceData)));
            dialog->exec();
        });

    menu.addAction(duplicateAct);
    menu.addAction(removeAct);

    auto sc = scene();
    if (sc && sc->selectedItems().count() >= 2) {
        menu.addSeparator();

        auto hAlign = new QAction(tr("Align horizontal"), &menu);
        connect(hAlign, SIGNAL(triggered(bool)), this, SIGNAL(alignHorizontal()));
        menu.addAction(hAlign);

        auto vAlign = new QAction(tr("Align vertical"), &menu);
        connect(vAlign, SIGNAL(triggered(bool)), this, SIGNAL(alignVertical()));
        menu.addAction(vAlign);
    }

    menu.addSeparator();
    menu.addAction(addToFavoritesAct);
    menu.addAction(propertiesAct);
    menu.exec(event->screenPos());

    event->accept();
}

size_t Device::visInletCount() const
{
    return data_->visInputCount();
}

size_t Device::visOutletCount() const
{
    return data_->visOutputCount();
}

bool Device::noXlets() const
{
    return !data_->hasVisInputs() && !data_->hasVisOutputs();
}

QJsonArray Device::xletToJson(const QList<XletData>& xlets) const
{
    QJsonArray arr;
    for (auto& x : xlets)
        arr.append(x.toJson());

    return arr;
}

void Device::clearInlets()
{
    for (auto x : childItems()) {
        auto xlet = qgraphicsitem_cast<DeviceXlet*>(x);
        if (xlet && xlet->isInlet())
            delete xlet;
    }
}

void Device::clearOutlets()
{
    for (auto x : childItems()) {
        auto xlet = qgraphicsitem_cast<DeviceXlet*>(x);
        if (xlet && xlet->isOutlet())
            delete xlet;
    }
}

void Device::clearXlets()
{
    for (auto x : inputs_)
        delete x;

    inputs_.clear();

    for (auto x : outputs_)
        delete x;

    outputs_.clear();
}

void Device::clearTitle()
{
    if (title_) {
        delete title_;
        title_ = nullptr;
    }
}

void Device::clearImage()
{
    if (image_) {
        delete image_;
        image_ = nullptr;
    }
}

void Device::createXlets()
{
    clearXlets();

    int in_idx = 0;
    for (auto& data : data_->inputs()) {
        if (data.isVisible()) {
            auto xlet = new DeviceXlet(data, XletType::In, this);
            xlet->setConnectPoint(inletPos(in_idx++));
            inputs_.push_back(xlet);
        }
    }

    int out_idx = 0;
    for (auto& data : data_->outputs()) {
        if (data.isVisible()) {
            auto xlet = new DeviceXlet(data, XletType::Out, this);
            xlet->setConnectPoint(outletPos(out_idx++));
            outputs_.push_back(xlet);
        }
    }
}

void Device::createTitle(qreal wd)
{
    clearTitle();

    if (data_->showTitle()) {
        title_ = new QGraphicsTextItem(data_->title(), this);
        title_->setTextWidth(wd);
        title_->setToolTip(data_->modelVendor());
    }
}

void Device::createImage(qreal wd)
{
    clearImage();

    if (!data_->image().isEmpty()) {
        image_ = new QGraphicsSvgItem(this);
        image_->setSharedRenderer(SvgRenderFactory::instance().getRender(data_->imageIconPath()));
        image_->setScale(data_->zoom());
    }
}

void Device::syncRect()
{
    // should be called first
    auto calc_wd = calcWidth();
    createTitle(calc_wd - 10);
    createImage(calc_wd);

    auto calc_ht = calcHeight();
    rect_.setRect(-calc_wd * 0.5, 0, calc_wd, calc_ht);

    updateTitlePos();
    updateImagePos();

    createXlets();
}

void Device::updateTitlePos()
{
    if (!title_)
        return;

    title_->setVisible(data_->showTitle());
    if (!data_->showTitle())
        return;

    auto opts = title_->document()->defaultTextOption();
    opts.setAlignment(Qt::AlignHCenter);
    title_->document()->setDefaultTextOption(opts);
    auto title_bbox = titleRect();
    title_->setPos(title_bbox.left(), 0);
}

void Device::updateImagePos()
{
    if (!image_)
        return;

    auto yoff = 0;
    if (title_ && data_->showTitle())
        yoff += title_->boundingRect().height();

    auto bbox = boundingRect();
    auto image_wd = image_->boundingRect().width() * image_->scale();
    image_->setPos(bbox.left() + (bbox.width() - image_wd) * 0.5, yoff);
}

void Device::updateXletsPos()
{
    int in_idx = 0;
    int out_idx = 0;

    for (auto x : inputs_)
        x->setConnectPoint(inletPos(in_idx++));

    for (auto x : outputs_)
        x->setConnectPoint(outletPos(out_idx++));
}

QRectF Device::titleRect() const
{
    auto bbox = boundingRect();
    auto title_box = title_->boundingRect();
    auto xoff = (bbox.width() - title_box.width()) * 0.5;
    return QRectF(bbox.left() + xoff, 0, title_box.width(), title_box.height());
}

QRectF Device::xletRect() const
{
    const auto NUM_IN = data_->visInputCount();
    const auto NUM_OUT = data_->visOutputCount();

    int h = 0;
    h += ((NUM_IN > 0) * XLET_H);
    h += ((NUM_OUT > 0) * XLET_H);
    const auto w = qMax(NUM_IN, NUM_OUT) * XLET_W;
    const auto bbox = boundingRect();
    const auto xoff = (bbox.width() - w) * 0.5;
    return QRectF(bbox.left() + xoff, bbox.bottom() - h, w, h);
}

int Device::calcWidth() const
{
    const auto NUM_IN = data_->visInputCount();
    const auto NUM_OUT = data_->visOutputCount();
    const auto TITLE_SIZE = data_->title().size();

    int w = qMax(NUM_IN, NUM_OUT) * XLET_W;

    if (TITLE_SIZE > 0 && data_->showTitle()) {
        auto txt_wd = qBound<int>(TITLE_MIN_CHAR_WIDTH, TITLE_SIZE, TITLE_MAX_CHAR_WIDTH);
        w = qMax<int>(w, txt_wd * TITLE_CHAR_WIDTH);
    }

    if (!data_->image().isEmpty())
        w = qMax<int>(w, DEF_IMAGE_WIDTH * data_->zoom());

    return w;
}

int Device::calcHeight() const
{
    int h = 0;
    if (title_ && data_->showTitle())
        h += title_->boundingRect().height();

    if (image_)
        h += image_->boundingRect().height() * image_->scale();

    h += (data_->hasVisInputs() * XLET_H);
    h += (data_->hasVisOutputs() * XLET_H);

    return h;
}

void Device::syncXlets()
{
    data_->foreachVisInput([this](XletIndex idx, XletData& data) {
        if (idx < inputs_.size())
            data = inputs_[idx]->xletData();
    });

    data_->foreachVisOutput([this](XletIndex idx, XletData& data) {
        if (idx < outputs_.size())
            data = outputs_[idx]->xletData();
    });
}

QJsonObject Device::toJson() const
{
    auto data_json = deviceData()->toJson();
    data_json["z"] = zValue();

    return data_json;
}

SharedDeviceData Device::defaultDeviceData()
{
    return makeDeviceData();
}

SharedDeviceData Device::datafromJson(const QJsonValue& j)
{
    if (!j.isObject()) {
        qWarning() << "not a object" << j;
        return {};
    }

    SharedDeviceData data(new DeviceData(DEV_NULL_ID));
    if (!data->setJson(j))
        return {};

    if (data->isNull()) {
        data->setId(DeviceIdFactory::instance().request());
    } else if (DeviceIdFactory::instance().isUsed(data->id())) {
        qWarning() << "device id is already used:" << data->id();
        data->setId(DeviceIdFactory::instance().request());
    }

    return data;
}

int Device::inletAt(const QPointF& pt) const
{
    auto pos = mapFromScene(pt).toPoint();
    for (int i = 0; i < data_->visInputCount(); i++) {
        if (inletRect(i).contains(pos))
            return i;
    }

    return -1;
}

int Device::outletAt(const QPointF& pt) const
{
    auto pos = mapFromScene(pt).toPoint();
    for (int i = 0; i < data_->visOutputCount(); i++) {
        if (outletRect(i).contains(pos))
            return i;
    }

    return -1;
}

SharedDeviceData Device::deviceData() const
{
    auto dev_pos = pos();

    if (data_->pos() != dev_pos)
        data_->setPos(dev_pos);

    return data_;
}

void Device::setDeviceData(const SharedDeviceData& data)
{
    if (data->isNull()) {
        qWarning() << "NULL data";
        return;
    }

    data_ = data;
    syncRect();
}

void Device::randomizePos(qint64 delta)
{
    auto value = qAbs(delta);
    auto dx = QRandomGenerator::global()->bounded(-value, value);
    auto dy = QRandomGenerator::global()->bounded(-value, value);

    setPos(x() + dx, y() + dy);
}
