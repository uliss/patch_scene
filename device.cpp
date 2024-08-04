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

constexpr qreal XLET_W = 16;
constexpr qreal XLET_H = 18;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

constexpr int MIN_WIDTH = 16;
constexpr int MIN_HEIGHT = 16;
constexpr int DEF_WIDTH = 100;
constexpr int DEF_HEIGHT = 40;

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

static DeviceData* makeDeviceData()
{
    auto* data = new DeviceData;
    data->id = Device::NullID;
    data->name = "Device";
    data->inlets.insert(0, 2, XletData { "", ConnectorModel::XLR });
    data->inlets.push_back(XletData { "MIDI", ConnectorModel::DIN_MIDI });
    data->inlets.push_back(XletData { "USB", ConnectorModel::USB_C });

    data->outlets.insert(0, 2, XletData { "", ConnectorModel::JACK_TRS });
    return data;
}

Device::Device()
    : Device(QSharedDataPointer { makeDeviceData() })
{
}

Device::Device(const SharedDeviceData& data)
    : data_(data)
    , title_ { nullptr }
    , image_ { nullptr }
{
    if (data_->id == NullID || DeviceIdFactory::instance().isUsed(data_->id)) {
        data_->id = DeviceIdFactory::instance().request();
        qDebug() << "create device with new #id" << data_->id;
    } else {
        DeviceIdFactory::instance().setUsed(data_->id);
        qDebug() << "create device with requested #id" << data_->id;
    }

    setPos(data_->pos);
    setZValue(data_->id);
    setFlag(QGraphicsItem::ItemIsSelectable);

    syncRect();

    createTitle();
    createImage();
    updateLayout();

    createXlets();
}

Device::~Device()
{
    DeviceIdFactory::instance().release(data_->id);
}

QPointF Device::inletPos(int i, bool map) const
{
    auto n = inletCount();
    if (n == 0)
        return {};

    auto bbox = boundingRect();
    qreal xstep = bbox.width() / n;
    auto pos = QPointF((i + 0.5) * xstep + bbox.left(), bbox.top());

    return map ? mapToScene(pos) : pos;
}

QPointF Device::outletPos(int i, bool map) const
{
    auto n = outletCount();
    if (n == 0)
        return {};

    auto bbox = boundingRect();
    qreal xstep = bbox.width() / n;
    auto pos = QPointF((i + 0.5) * xstep + bbox.left(), bbox.bottom());

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
    } else {
        pen.setColor(Qt::black);
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());

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
    switch (data_->category) {
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
        if (data_->inlets[i].phantom_power) {
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
    Q_UNUSED(event);
}

size_t Device::inletCount() const
{
    return data_->visInletCount();
}

size_t Device::outletCount() const
{
    return data_->visOutletCount();
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
    for (auto& data : data_->inlets) {
        if (data.visible) {
            auto xlet = new DeviceXlet(data, XletType::In, this);
            xlet->setConnectPoint(inletPos(in_idx++));
        }
    }

    int out_idx = 0;
    for (auto& data : data_->outlets) {
        if (data.visible) {
            auto xlet = new DeviceXlet(data, XletType::Out, this);
            xlet->setConnectPoint(outletPos(out_idx++));
        }
    }
}

void Device::createTitle()
{
    clearTitle();
    title_ = new QGraphicsTextItem(data_->name, this);
}

void Device::createImage()
{
    clearImage();

    if (!data_->image.isEmpty())
        image_ = new QGraphicsSvgItem(data_->image, this);
}

void Device::syncRect()
{
    setRect(-data_->width / 2, 0, data_->width, data_->height);
}

void Device::syncTitle()
{
    if (title_)
        title_->setPlainText(data_->name);
}

void Device::updateTitlePos()
{
    if (!title_)
        return;

    title_->setTextWidth(data_->width);
    auto opts = title_->document()->defaultTextOption();
    opts.setAlignment(Qt::AlignHCenter);
    title_->document()->setDefaultTextOption(opts);

    auto yoff = title_->boundingRect().height();
    title_->setPos(centerXOff(), -yoff);
}

void Device::updateImagePos()
{
    if (!image_)
        return;

    auto image_bbox = image_->boundingRect();
    auto yoff = image_bbox.height();
    if (title_)
        yoff += title_->boundingRect().height();

    image_->setPos(-image_bbox.width() / 2, -yoff);
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

void Device::updateLayout()
{
    updateTitlePos();
    updateImagePos();
}

QRectF Device::titleRect() const
{
    auto title_h = title_->boundingRect().height();
    return QRectF(centerXOff(), -title_h, data_->width, title_h);
}

qreal Device::centerXOff() const
{
    return -(data_->width / 2.0);
}

void Device::incrementName()
{
    auto parts = data_->name.split(' ', Qt::SkipEmptyParts);
    if (parts.size() > 1) {
        bool ok = false;
        auto num = parts.back().toInt(&ok);
        if (!ok) {
            data_->name += " 1";
        } else {
            auto prefix = parts.sliced(0, parts.size() - 1).join(' ');
            data_->name = QString("%1 %2").arg(prefix).arg(num + 1);
        }
    } else {
        data_->name += " 1";
    }

    syncTitle();
    updateLayout();
}

QJsonObject Device::toJson() const
{
    QJsonObject json;

    json["x"] = x();
    json["y"] = y();
    json["z"] = zValue();
    json["id"] = static_cast<qint32>(id());
    json["name"] = data_->name;
    json["width"] = data_->width;
    json["height"] = data_->height;
    json["zvalue"] = data_->zvalue;
    json["image"] = data_->image;
    json["inlets"] = xletToJson(data_->inlets);
    json["outlets"] = xletToJson(data_->outlets);
    json["category"] = toString(data_->category);

    return json;
}

bool Device::fromJson(const QJsonValue& j, Device& dev)
{
    if (!j.isObject()) {
        qWarning() << "not a object" << j;
        return false;
    }

    auto obj = j.toObject();
    dev.data_->name = obj.value("name").toString();
    dev.syncTitle();
    dev.updateTitlePos();

    dev.setPos(obj.value("x").toInt(), obj.value("y").toInt());
    // @TODO check values
    auto json_id = obj.take("id").toInteger();
    if (json_id < 0) {
        qWarning() << "invalid device id:" << json_id;
        json_id = DeviceIdFactory::instance().request();
    } else if (DeviceIdFactory::instance().isUsed(json_id)) {
        qWarning() << "device id is already used:" << json_id;
        // json_id = DeviceIdFactory::instance().request();
    }

    dev.data_->id = json_id;
    dev.data_->zvalue = obj.value("zvalue").toDouble();
    dev.setZValue(obj.value("z").toDouble());

    dev.data_->width = obj.value("width").toInteger(DEF_WIDTH);
    dev.data_->height = obj.value("height").toInteger(DEF_HEIGHT);
    dev.syncRect();

    dev.data_->image = obj.value("image").toString();
    dev.createImage();
    dev.updateImagePos();

    ItemCategory cat;
    if (fromQString(obj.value("category").toString(), cat))
        dev.data_->category = cat;

    dev.data_->inlets.clear();
    dev.data_->outlets.clear();

    auto inlets = obj.value("inlets");
    if (inlets.isArray()) {
        auto arr = inlets.toArray();
        for (const auto& x : arr) {
            if (x.isObject()) {
                auto xlet = x.toObject();
                XletData data;
                if (XletData::fromJson(xlet, data))
                    dev.data_->inlets.push_back(data);
            }
        }
    }

    auto outlets = obj.value("outlets");
    if (outlets.isArray()) {
        auto arr = outlets.toArray();
        for (const auto& x : arr) {
            if (x.isObject()) {
                auto xlet = x.toObject();
                XletData data;
                if (XletData::fromJson(xlet, data))
                    dev.data_->outlets.push_back(data);
            }
        }
    }

    dev.createXlets();
    return true;
}

int Device::inletAt(const QPointF& pt) const
{
    auto pos = mapFromScene(pt).toPoint();
    for (int i = 0; i < data_->visInletCount(); i++) {
        if (inletRect(i).contains(pos))
            return i;
    }

    return -1;
}

int Device::outletAt(const QPointF& pt) const
{
    auto pos = mapFromScene(pt).toPoint();
    for (int i = 0; i < data_->visOutletCount(); i++) {
        if (outletRect(i).contains(pos))
            return i;
    }

    return -1;
}

SharedDeviceData Device::deviceData() const
{
    auto dev_pos = pos();

    if (data_->pos != dev_pos)
        data_->pos = dev_pos;

    return data_;
}

void Device::setDeviceData(const SharedDeviceData& data)
{
    bool img_change = (data_->image != data->image);

    data_ = data;
    syncRect();
    syncTitle();
    updateTitlePos();

    if (img_change) {
        if (image_) {

            delete image_;
            image_ = nullptr;
        }

        if (!data_->image.isEmpty()) {
            image_ = new QGraphicsSvgItem(data_->image, this);
        }
    }

    updateImagePos();

    clearXlets();
    createXlets();
}

size_t DeviceData::visInletCount() const
{
    return std::count_if(inlets.begin(), inlets.end(), [](const XletData& x) { return x.visible; });
}

size_t DeviceData::visOutletCount() const
{
    return std::count_if(outlets.begin(), outlets.end(), [](const XletData& x) { return x.visible; });
}
