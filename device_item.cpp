/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
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
#include "device_item.h"
#include "svg_render_factory.h"
#include "xlets_view.h"

#include <QAction>
#include <QGraphicsSvgItem>
#include <QMenu>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QTextDocument>

constexpr int TITLE_MIN_CHAR_WIDTH = 8;
constexpr int TITLE_MAX_CHAR_WIDTH = 18;
constexpr int TITLE_CHAR_WIDTH = 10;
constexpr qreal DEF_IMAGE_WIDTH = 100;

namespace ceam {

DeviceItem::DeviceItem()
    : title_ { nullptr }
    , image_ { nullptr }
{
    xlets_.initDefaultView();
    xlets_.setData(data_);

    syncRect();
}

DeviceItem::DeviceItem(const SharedDeviceData& data)
    : SceneItem(data)
    , title_ { nullptr }
    , image_ { nullptr }
{
    xlets_.initDefaultView();
    xlets_.setData(data);

    syncRect();
}

QRectF DeviceItem::boundingRect() const
{
    return rect_;
}

void DeviceItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
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

    paintStateIcons(painter);

    Q_UNUSED(widget);
}

void DeviceItem::paintTitleBox(QPainter* painter)
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

void DeviceItem::paintStateIcons(QPainter* painter)
{
    if (isLocked()) {
        painter->save();
        constexpr qreal WD = 8;
        constexpr qreal HT = WD - 2;
        constexpr qreal AWD = WD - 2;
        painter->translate(rect_.width() / 2 - 12, 0);
        QColor c(100, 100, 100);
        painter->setBrush(QBrush(c));
        painter->setPen(QPen(c, 1.5));
        painter->drawArc(QRectF { (WD - AWD) * 0.5, 0.5 * HT, AWD, HT }, 0, 180 * 16);
        painter->drawRect(QRectF { 0, 1.4 * HT, WD, HT });
        painter->restore();
    }
}

int DeviceItem::inletsYOff() const
{
    qreal yoff = 0;
    if (title_ && data_->showTitle())
        yoff += title_->boundingRect().height();

    yoff += imageHeight();

    return qRound(yoff);
}

qreal DeviceItem::imageWidth() const
{
    return image_ ? (image_->boundingRect().width() * image_->scale()) : 0;
}

qreal DeviceItem::imageHeight() const
{
    return image_ ? (image_->boundingRect().height() * image_->scale()) : 0;
}

int DeviceItem::calcWidth() const
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

int DeviceItem::calcHeight() const
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

void DeviceItem::clearTitle()
{
    if (title_) {
        delete title_;
        title_ = nullptr;
    }
}

void DeviceItem::clearImage()
{
    if (image_) {
        delete image_;
        image_ = nullptr;
    }
}

void DeviceItem::createXlets()
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

void DeviceItem::createTitle(qreal wd)
{
    clearTitle();

    if (data_->showTitle()) {
        title_ = new QGraphicsTextItem(data_->title());
        title_->setTextWidth(wd);
        title_->setToolTip(data_->verboseInfo());
        title_->setParentItem(this);
    }
}

void DeviceItem::createImage()
{
    clearImage();

    if (!data_->image().isEmpty()) {
        image_ = new QGraphicsSvgItem;
        image_->setSharedRenderer(SvgRenderFactory::instance().getRender(data_->imageIconPath()));
        image_->setScale(data_->zoom());
        image_->setToolTip(data_->verboseInfo());
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

void DeviceItem::updateTitlePos()
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

void DeviceItem::updateImagePos()
{
    if (!image_)
        return;

    auto yoff = 0;
    if (title_ && data_->showTitle())
        yoff += title_->boundingRect().height();

    image_->setPos(centerAlignedLeftPos(imageWidth()), yoff);
}

void DeviceItem::updateXletsPos()
{
    auto v = xlets_.currentView();
    if (v) {
        auto xoff = v->width() * 0.5;
        v->placeXlets({ -xoff, static_cast<qreal>(inletsYOff()) });
    }
}

QRectF DeviceItem::titleRect() const
{
    if (!title_ || !data_->showTitle())
        return {};

    auto title_box = title_->boundingRect();
    title_box.translate(centerAlignedLeftPos(title_box.width()), 0);
    return title_box;
}

QRectF DeviceItem::xletRect() const
{
    auto v = xlets_.currentView();
    if (!v || xlets_.isEmpty())
        return {};

    auto brect = v->boundingRect();
    return brect.translated(brect.width() * -0.5, inletsYOff());
}

bool DeviceItem::setDeviceData(const SharedDeviceData& data)
{
    if (SceneItem::setDeviceData(data)) {
        xlets_.setData(data);
        syncRect();
        return true;
    } else
        return false;
}

std::optional<QPointF> DeviceItem::connectionPoint(XletIndex i, XletType type, bool map) const
{
    auto view = xlets_.currentView();
    if (!view)
        return {};

    auto pos = xlets_.connectionPoint({ i, type });
    if (!pos)
        return pos;

    return map ? mapToScene(*pos) : *pos;
}

void DeviceItem::syncXletData()
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

bool DeviceItem::mirrorImage(ImageMirrorType type)
{
    if (!data_ || data_->isLocked() || !image_)
        return false;

    const auto im = data_->imageMirror();

    if (im == type)
        data_->setImageMirror(ImageMirrorType::None);
    else if (im == ImageMirrorType::None)
        data_->setImageMirror(type);
    else
        return false;

    clearImage();
    createImage();
    updateImagePos();
    return true;
}

bool DeviceItem::zoomImage(qreal k)
{
    if (!data_ || data_->isLocked())
        return false;

    auto old_zoom = data_->zoom();
    auto new_zoom = old_zoom * k;
    data_->setZoom(new_zoom);

    // no zoom change due min/max clipping
    if (data_->zoom() == old_zoom)
        return false;

    syncRect();
    return true;
}

void DeviceItem::syncRect()
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

void DeviceItem::addTitleAction(QMenu& menu)
{
    auto showTitle = new QAction(&menu);
    showTitle->setChecked(data_->showTitle());
    showTitle->setText(data_->showTitle() ? tr("Hide title") : tr("Show title"));
    connect(showTitle, &QAction::triggered, this,
        [this](bool checked) {
            if (!data_ || data_->isLocked())
                return;

            auto data = data_;
            auto show_title = data_->showTitle();
            data.detach();
            data->setShowTitle(!show_title);
            emit updateDevice(data);
        });

    menu.addAction(showTitle);
}

void DeviceItem::addMirrorAction(QMenu& menu)
{
    auto mirrorAct = new QAction(tr("Mirror image"), &menu);
    connect(mirrorAct, &QAction::triggered, this, [this](bool) { emit mirror(data_->id()); });
    menu.addAction(mirrorAct);
}

void DeviceItem::addToFavoritesAct(QMenu& menu)
{
    auto addToFavoritesAct = new QAction(tr("Add to favorites"), &menu);
    // addToFavoritesAct->setIcon(QIcon(":/icons/favorite.svg"));
    connect(addToFavoritesAct, &QAction::triggered, this,
        [this]() { emit addToFavorites(data_); });

    menu.addAction(addToFavoritesAct);
}

void DeviceItem::addViewSubMenu(QMenu& menu)
{
    auto views = menu.addMenu(tr("Views"));
    auto act_view_default = views->addAction(tr("Logic"));
    act_view_default->setCheckable(true);
    if (data_->currentUserView().isEmpty())
        act_view_default->setChecked(true);

    connect(act_view_default, &QAction::triggered, this,
        [this]() {
            data_->setCurrentUserView({});
            setDeviceData(data_);
            emit updateDevice(data_);
        });

    for (auto& x : data_->userViewData()) {
        auto name = x.name();
        auto act_view_user = views->addAction(name);
        act_view_user->setCheckable(true);

        if (name == data_->currentUserView())
            act_view_user->setChecked(true);

        connect(act_view_user, &QAction::triggered, this,
            [this, name]() {
                data_->setCurrentUserView(name);
                setDeviceData(data_);
                emit updateDevice(data_);
            });
    }
}

void DeviceItem::createContextMenu(QMenu& menu)
{
    setMenuCaption(menu);

    if (!data_->isLocked()) {
        addTitleAction(menu);
        addMirrorAction(menu);
        menu.addSeparator();
        addZValueAction(menu);
        menu.addSeparator();
    }

    addLockAction(menu);

    if (!data_->userViewData().isEmpty()) {
        addViewSubMenu(menu);
    }

    menu.addSeparator();
    addDuplicateAct(menu);
    addRemoveAct(menu);

    menu.addSeparator();
    addToFavoritesAct(menu);
    addPropertiesAct(menu);
}

} // namespace ceam
