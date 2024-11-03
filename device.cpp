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
#include "logging.hpp"
#include "svg_render_factory.h"
#include "xlets_view.h"

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
    xlets_.initDefaultView();
    xlets_.setData(data);

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

    setCacheMode(DeviceCoordinateCache);
}

Device::~Device()
{
    DeviceIdFactory::instance().release(data_->id());
}

QRectF Device::boundingRect() const
{
    return rect_;
}

void Device::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    paintTitleBox(painter);

    QPen pen(Qt::SolidPattern, 0);
    auto view = xlets_.currentView();

    if (option->state & QStyle::State_Selected) {
        pen.setColor(Qt::blue);
        painter->setPen(pen);

        if (view)
            view->paint(painter, { -static_cast<int>(view->width() / 2), inletsYOff() });

        pen.setStyle(Qt::DotLine);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(pen);
        painter->drawRect(boundingRect());
    } else if (view) { // draw xlet box
        pen.setColor(Qt::black);
        painter->setPen(pen);

        if (view)
            view->paint(painter, { static_cast<int>(view->width() / -2), inletsYOff() });
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

    auto sc = scene();
    if (sc && sc->selectedItems().count() > 1) {
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
    } else {
        auto showTitle = new QAction(&menu);
        showTitle->setChecked(data_->showTitle());
        showTitle->setText(data_->showTitle() ? tr("Hide title") : tr("Show title"));
        connect(showTitle, &QAction::triggered, this,
            [this](bool checked) {
                auto data = data_;
                auto show_title = data_->showTitle();
                data.detach();
                data->setShowTitle(!show_title);
                emit updateDevice(data);
            });

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

        auto info = menu.addAction(data_->title());
        info->setDisabled(true);
        auto info_font = info->font();
        info_font.setBold(true);
        info->setFont(info_font);
        menu.setStyleSheet("QMenu::item:disabled {color: black;}");
        menu.addAction(showTitle);
        if (!data_->userViewData().isEmpty()) {
            auto views = menu.addMenu(tr("Views"));
            auto act_view_default = views->addAction(tr("Logic"));
            connect(act_view_default, &QAction::triggered, this,
                [this]() {
                    data_->setCurrentUserView({});
                    setDeviceData(data_);
                    emit updateDevice(data_);
                });

            for (auto& x : data_->userViewData()) {
                auto name = x.name();
                auto act_view_user = views->addAction(name);
                connect(act_view_user, &QAction::triggered, this,
                    [this, name]() {
                        data_->setCurrentUserView(name);
                        setDeviceData(data_);
                        emit updateDevice(data_);
                    });
            }
        }

        menu.addSeparator();
        menu.addAction(duplicateAct);
        menu.addAction(removeAct);

        menu.addSeparator();
        menu.addAction(addToFavoritesAct);
        menu.addAction(propertiesAct);
    }

    menu.exec(event->screenPos());

    event->accept();
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
    xlets_.clearXlets();

    for (auto& data : data_->inputs()) {
        if (data.isVisible())
            xlets_.append(data, XletType::In, this);
    }

    for (auto& data : data_->outputs()) {
        if (data.isVisible())
            xlets_.append(data, XletType::Out, this);
    }
}

void Device::createTitle(qreal wd)
{
    clearTitle();

    if (data_->showTitle()) {
        title_ = new QGraphicsTextItem(data_->title());
        title_->setTextWidth(wd);
        title_->setToolTip(data_->modelVendor());
        title_->setParentItem(this);
    }
}

void Device::createImage()
{
    clearImage();

    if (!data_->image().isEmpty()) {
        image_ = new QGraphicsSvgItem;
        image_->setSharedRenderer(SvgRenderFactory::instance().getRender(data_->imageIconPath()));
        image_->setScale(data_->zoom());
        switch (data_->imageMirror()) {
        case ImageMirrorType::Horizontal: {
            auto tr = QTransform::fromScale(-1, 1).translate(-image_->boundingRect().width() * data_->zoom(), 0);
            image_->setTransform(tr);
        } break;
        default:
            break;
        }

        image_->setParentItem(this);
    }
}

void Device::syncRect()
{
    prepareGeometryChange();

    createXlets();
    createImage();

    // should be called after xlets and image creation
    auto calc_wd = calcWidth();
    createTitle(calc_wd - 10);

    auto calc_ht = calcHeight();
    rect_.setRect(-calc_wd * 0.5, 0, calc_wd, calc_ht);

    updateTitlePos();
    updateImagePos();
    updateXletsPos();
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

    image_->setPos(centerAlignedLeftPos(imageWidth()), yoff);
}

void Device::updateXletsPos()
{
    auto v = xlets_.currentView();
    if (v) {
        auto xoff = v->width() * 0.5;
        v->placeXlets({ -xoff, static_cast<qreal>(inletsYOff()) });
    }
}

QRectF Device::titleRect() const
{
    if (!title_ || !data_->showTitle())
        return {};

    auto title_box = title_->boundingRect();
    title_box.translate(centerAlignedLeftPos(title_box.width()), 0);
    return title_box;
}

QRectF Device::xletRect() const
{
    auto v = xlets_.currentView();
    if (!v || xlets_.isEmpty())
        return {};

    auto brect = v->boundingRect();
    return brect.translated(brect.width() * -0.5, inletsYOff());
}

std::optional<QPointF> Device::connectionPoint(XletIndex i, XletType type, bool map) const
{
    auto view = xlets_.currentView();
    if (!view)
        return {};

    auto pos = xlets_.connectionPoint({ i, type });
    if (!pos)
        return pos;

    return map ? mapToScene(*pos) : *pos;
}

int Device::calcWidth() const
{
    const auto TITLE_SIZE = data_->title().size();

    auto view = xlets_.currentView();
    int w = view ? xlets_.currentView()->width() : 0;

    if (TITLE_SIZE > 0 && data_->showTitle()) {
        auto txt_wd = qBound<int>(TITLE_MIN_CHAR_WIDTH, TITLE_SIZE, TITLE_MAX_CHAR_WIDTH);
        w = qMax<int>(w, txt_wd * TITLE_CHAR_WIDTH);
    }

    if (image_)
        w = qMax<int>(w, imageWidth());

    return w;
}

int Device::calcHeight() const
{
    int h = 0;
    if (title_ && data_->showTitle())
        h += title_->boundingRect().height();

    h += imageHeight();

    auto view = xlets_.currentView();
    if (view)
        h += xlets_.currentView()->height();

    return h;
}

qreal Device::imageWidth() const
{
    return image_ ? (image_->boundingRect().width() * image_->scale()) : 0;
}

qreal Device::imageHeight() const
{
    return image_ ? (image_->boundingRect().height() * image_->scale()) : 0;
}

int Device::inletsYOff() const
{
    qreal yoff = 0;
    if (title_ && data_->showTitle())
        yoff += title_->boundingRect().height();

    yoff += imageHeight();

    return qRound(yoff);
}

void Device::syncXletData()
{
    {
        XletIndex in_idx = 0;
        for (auto& data : data_->inputs()) {
            auto xlet = xlets_.xletAtIndex({ in_idx++, XletType::In });
            if (xlet)
                data = xlet->xletData();
        }
    }

    {
        XletIndex out_idx = 0;
        for (auto& data : data_->outputs()) {
            auto xlet = xlets_.xletAtIndex({ out_idx++, XletType::Out });
            if (xlet)
                data = xlet->xletData();
        }
    }
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

SharedDeviceData Device::dataFromJson(const QJsonValue& j)
{
    if (!j.isObject()) {
        WARN() << "not a object" << j;
        return {};
    }

    SharedDeviceData data(new DeviceData(DEV_NULL_ID));
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
        WARN() << "NULL data";
        return;
    }

    data_ = data;
    xlets_.setData(data);
    syncRect();
}

void Device::randomizePos(qint64 delta)
{
    auto value = qAbs(delta);
    auto dx = QRandomGenerator::global()->bounded(-value, value);
    auto dy = QRandomGenerator::global()->bounded(-value, value);

    setPos(x() + dx, y() + dy);
}
