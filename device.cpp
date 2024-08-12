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

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSvgItem>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>

#include <unordered_map>

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

constexpr int MIN_WIDTH = 16;
constexpr int MIN_HEIGHT = 16;
constexpr int DEF_WIDTH = 100;
constexpr int DEF_HEIGHT = 40;

constexpr int TITLE_MIN_CHAR_WIDTH = 5;
constexpr int TITLE_MAX_CHAR_WIDTH = 18;
constexpr int TITLE_CHAR_WIDTH = 10;
constexpr qreal DEF_IMAGE_WIDTH = 100;

constexpr DeviceId INIT_ID = 1;

using DeviceIdMap = std::unordered_map<DeviceId, bool>;

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

static SharedDeviceData makeDeviceData()
{
    QSharedDataPointer data(new DeviceData(DEV_NULL_ID));
    data->setTitle("Device");
    data->appendInput(XletData { "", ConnectorModel::XLR });
    data->appendInput(XletData { "", ConnectorModel::XLR });
    data->appendInput(XletData { "MIDI", ConnectorModel::DIN_MIDI });
    data->appendInput(XletData { "USB", ConnectorModel::USB_C });

    data->appendOutput(XletData { "L", ConnectorModel::JACK_TRS });
    data->appendOutput(XletData { "R", ConnectorModel::JACK_TRS });
    return data;
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

QPointF Device::inletPos(int i, bool map) const
{
    const auto NUM_IN = inletCount();
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
    const auto NUM_OUT = outletCount();
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

void Device::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
    QWidget* widget)
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

    Q_UNUSED(option);
    Q_UNUSED(widget);

    paintInlets(painter);
    paintOutlets(painter);
}

void Device::paintTitleBox(QPainter* painter)
{
    if (!title_)
        return;

    painter->setPen(Qt::NoPen);
    switch (data_->category()) {
    case ItemCategory::Send:
        painter->setBrush(QColor::fromRgb(0xD7, 0xF4, 0xf5, 200));
        break;
    case ItemCategory::Return:
        painter->setBrush(QColor::fromRgb(0xff, 0xd1, 0x71, 200));
        break;
    case ItemCategory::Device:
    case ItemCategory::Instrument:
    case ItemCategory::Human:
    case ItemCategory::Furniture:
    default:
        painter->setBrush(QColor::fromRgbF(0.9, 0.9, 0.9, 0.6));
        break;
    }

    auto title_box = titleRect();
    painter->drawRoundedRect(title_box.adjusted(4, 3, -4, -4), 5, 5);
}

void Device::paintInlets(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 0.5));

    for (size_t i = 0; i < inletCount(); i++) {
        auto pos = inletPos(i);
        if (data_->inputAt(i).phantom_power) {
            painter->setBrush(Qt::red);
            painter->setPen(QPen(Qt::red, 1));
        } else {
            painter->setBrush(Qt::black);
            painter->setPen(QPen(Qt::black, 1));
        }

        painter->drawRect(pos.x() - (XLET_BOX_W / 2), pos.y(), XLET_BOX_W, XLET_BOX_H);

        // painter->drawRect(inletRect(i));
    }
}

void Device::paintOutlets(QPainter* painter)
{
    painter->setPen(QPen(Qt::black, 0.5));
    painter->setBrush(Qt::black);

    for (size_t i = 0; i < outletCount(); i++) {
        auto pos = outletPos(i);
        painter->drawRect(pos.x() - (XLET_BOX_W / 2), pos.y() - XLET_BOX_H, XLET_BOX_W, XLET_BOX_H);
    }
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
    qDebug() << __FUNCTION__;
    Q_UNUSED(event);
}

size_t Device::inletCount() const
{
    return data_->visInputCount();
}

size_t Device::outletCount() const
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
    for (auto x : childItems()) {
        auto xlet = qgraphicsitem_cast<DeviceXlet*>(x);
        if (xlet)
            delete xlet;
    }
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
        if (data.visible) {
            auto xlet = new DeviceXlet(data, XletType::In, this);
            xlet->setConnectPoint(inletPos(in_idx++));
        }
    }

    int out_idx = 0;
    for (auto& data : data_->outputs()) {
        if (data.visible) {
            auto xlet = new DeviceXlet(data, XletType::Out, this);
            xlet->setConnectPoint(outletPos(out_idx++));
        }
    }
}

void Device::createTitle(qreal wd)
{
    clearTitle();
    title_ = new QGraphicsTextItem(data_->title(), this);
    title_->setTextWidth(wd);
}

void Device::createImage(qreal wd)
{
    clearImage();

    if (!data_->image().isEmpty()) {
        image_ = new QGraphicsSvgItem(data_->imageIconPath(), this);
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
    setRect(-calc_wd * 0.5, 0, calc_wd, calc_ht);

    updateTitlePos();
    updateImagePos();

    createXlets();
}

void Device::updateTitlePos()
{
    if (!title_)
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
    if (title_)
        yoff += title_->boundingRect().height();

    auto bbox = boundingRect();
    auto image_wd = image_->boundingRect().width() * image_->scale();
    image_->setPos(bbox.left() + (bbox.width() - image_wd) * 0.5, yoff);
}

void Device::updateXletsPos()
{
    int in_idx = 0;
    int out_idx = 0;

    for (auto x : childItems()) {
        auto xlet = qgraphicsitem_cast<DeviceXlet*>(x);
        if (xlet) {
            if (xlet->isInlet() && xlet->isVisible()) {
                xlet->setConnectPoint(inletPos(in_idx++));
                continue;
            }

            if (xlet->isOutlet() && xlet->isVisible()) {
                xlet->setConnectPoint(outletPos(out_idx++));
                continue;
            }
        }
    }
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

qreal Device::centerXOff() const
{
    return -boundingRect().width() * 0.5;
}

int Device::calcWidth() const
{
    const auto NUM_IN = data_->visInputCount();
    const auto NUM_OUT = data_->visOutputCount();
    const auto TITLE_SIZE = data_->title().size();

    int w = qMax(NUM_IN, NUM_OUT) * XLET_W;

    if (TITLE_SIZE > 0) {
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
    if (title_)
        h += title_->boundingRect().height();

    if (image_)
        h += image_->boundingRect().height() * image_->scale();

    h += (data_->hasVisInputs() * XLET_H);
    h += (data_->hasVisOutputs() * XLET_H);

    return h;
}

void Device::incrementName()
{
    auto parts = data_->title().split(' ', Qt::SkipEmptyParts);
    if (parts.size() > 1) {
        bool ok = false;
        auto num = parts.back().toInt(&ok);
        if (!ok) {
            data_->title() += " 1";
        } else {
            auto prefix = parts.sliced(0, parts.size() - 1).join(' ');
            data_->setTitle(QString("%1 %2").arg(prefix).arg(num + 1));
        }
    } else {
        data_->setTitle(QString("%1 1").arg(data_->title()));
    }

    syncRect();
}

QJsonObject Device::toJson() const
{
    QJsonObject json;

    auto data_json = data_->toJson();

    data_json["x"] = x();
    data_json["y"] = y();
    data_json["z"] = zValue();

    return json;
}

bool Device::fromJson(const QJsonValue& j, Device& dev)
{
    if (!j.isObject()) {
        qWarning() << "not a object" << j;
        return false;
    }

    if (!dev.data_->setJson(j)) {
        return false;
    }

    auto obj = j.toObject();

    dev.setPos(obj.value("x").toInt(), obj.value("y").toInt());
    dev.setZValue(obj.value("z").toDouble());

    if (dev.data_->isNull()) {
        dev.data_->setId(DeviceIdFactory::instance().request());
    } else if (DeviceIdFactory::instance().isUsed(dev.data_->id())) {
        qWarning() << "device id is already used:" << dev.data_->id();
        dev.data_->setId(DeviceIdFactory::instance().request());
    }

    dev.syncRect();
    return true;
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
