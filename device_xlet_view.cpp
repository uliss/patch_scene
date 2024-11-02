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
#include "device_xlet_view.h"
#include "device_common.h"
#include "device_xlet.h"
#include "logging.hpp"
#include "xlets_user_view.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QPainter>

namespace {

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

constexpr const char* DEFAULT_VIEW = "logical";
constexpr const char* JSON_KEY_CURRENT_VIEW = "current";
constexpr const char* JSON_KEY_VIEWS = "views";
constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_TYPE = "type";

}

namespace ceam {

DeviceXlets::DeviceXlets()
{
}

DeviceXlets::~DeviceXlets()
{
    clearXlets();
}

bool DeviceXlets::append(const XletData& data, XletType type, QGraphicsItem* parent)
{
    switch (type) {
    case XletType::In: {
        auto xlet = new DeviceXlet(data, XletInfo { DEV_NULL_ID, (XletIndex)inlets_.count(), type }, parent);
        inlets_.push_back(xlet);
    } break;
    case XletType::Out: {
        auto xlet = new DeviceXlet(data, XletInfo { DEV_NULL_ID, (XletIndex)outlets_.count(), type }, parent);
        outlets_.push_back(xlet);
    } break;
    default:
        return false;
    }

    return true;
}

bool DeviceXlets::isEmpty() const
{
    return inlets_.isEmpty() && outlets_.isEmpty();
}

DeviceXlet* DeviceXlets::xletAtIndex(XletViewIndex vidx)
{
    switch (vidx.type) {
    case XletType::In:
        if (vidx.index < inlets_.count())
            return inlets_[vidx.index];

        break;
    case XletType::Out:
        if (vidx.index < outlets_.count())
            return outlets_[vidx.index];

        break;
    default:
        break;
    }

    return nullptr;
}

const DeviceXlet* DeviceXlets::xletAtIndex(XletViewIndex vidx) const
{
    switch (vidx.type) {
    case XletType::In:
        if (vidx.index < inlets_.count())
            return inlets_[vidx.index];

        break;
    case XletType::Out:
        if (vidx.index < outlets_.count())
            return outlets_[vidx.index];

        break;
    default:
        break;
    }

    return nullptr;
}

DeviceXlet* DeviceXlets::inletAt(XletIndex idx)
{
    return (idx < inlets_.size()) ? inlets_[idx] : nullptr;
}

const DeviceXlet* DeviceXlets::inletAt(XletIndex idx) const
{
    return (idx < inlets_.size()) ? inlets_[idx] : nullptr;
}

DeviceXlet* DeviceXlets::outletAt(XletIndex idx)
{
    return (idx < outlets_.size()) ? outlets_[idx] : nullptr;
}

const DeviceXlet* DeviceXlets::outletAt(XletIndex idx) const
{
    return (idx < outlets_.size()) ? outlets_[idx] : nullptr;
}

void DeviceXlets::clearXlets()
{
    clearXlets(inlets_);
    clearXlets(outlets_);
}

void DeviceXlets::clearViews()
{
    user_views_.clear();
    current_view_ = logic_view_.get();
}

void DeviceXlets::initDefaultView()
{
    logic_view_ = std::make_unique<XletsTableView>(DEFAULT_VIEW, *this);
    current_view_ = logic_view_.get();
}

bool DeviceXlets::setCurrentView(const QString& name)
{
    for (auto& v : user_views_) {
        if (v && v->name() == name) {
            current_view_ = v.get();
            return true;
        }
    }

    return false;
}

void DeviceXlets::setData(const SharedDeviceData& data)
{
    if (!data)
        return;

    if (logic_view_)
        logic_view_->setData(data);

    clearViews();
    for (auto& x : data->userViewData()) {
        auto view = new XletsUserView(x.name(), *this);
        view->setData(x);
        user_views_.emplace_back(view);
    }

    if (data->currentUserView().isEmpty() || !setCurrentView(data->currentUserView()))
        current_view_ = logic_view_.get();
}

void DeviceXlets::setVisible(bool value)
{
    for (auto x : inlets_)
        x->setVisible(value);

    for (auto x : outlets_)
        x->setVisible(value);
}

bool DeviceXlets::appendView(std::unique_ptr<XletsView> view)
{
    if (!view)
        return false;

    user_views_.push_back(std::move(view));
    return true;
}

void DeviceXlets::clearXlets(QList<DeviceXlet*>& xlets)
{
    for (auto x : xlets) {
        auto scene = x->scene();
        if (scene) {
            scene->removeItem(x);
            delete x;
        }
    }

    xlets.clear();
}

XletsView::XletsView(const QString& name)
    : name_(name)
{
}

XletsView::~XletsView() { }

QRectF XletsView::boundingRect() const
{
    return { 0, 0, width(), height() };
}

bool XletsView::setData(const SharedDeviceData& /*data*/)
{
    return true;
}

XletsTableView::XletsTableView(const QString& name, DeviceXlets& xlets)
    : XletsView(name)
    , xlets_(xlets)
{
}

qreal XletsTableView::width() const
{
    return std::max(inletsWidth(), outletsWidth());
}

qreal XletsTableView::height() const
{
    return inletsHeight() + outletsHeight();
}

qreal XletsTableView::inletsWidth() const
{
    return inletsColCount() * XLET_W;
}

qreal XletsTableView::inletsHeight() const
{
    return inletsRowCount() * XLET_H;
}

qreal XletsTableView::outletsWidth() const
{
    return outletsColCount() * XLET_W;
}

int XletsTableView::inletsColCount() const
{
    return std::min<int>(data_.maxInputColumnCount(), xlets_.inletCount());
}

qreal XletsTableView::outletsHeight() const
{
    return outletsRowCount() * XLET_H;
}

int XletsTableView::inletsRowCount() const
{
    auto n = xlets_.inletCount();
    return n / data_.maxInputColumnCount() + (n % data_.maxInputColumnCount() > 0);
}

int XletsTableView::outletsColCount() const
{
    return std::min<int>(data_.maxOutputColumnCount(), xlets_.outletCount());
}

int XletsTableView::outletsRowCount() const
{
    auto n = xlets_.outletCount();
    return n / data_.maxOutputColumnCount() + (n % data_.maxOutputColumnCount() > 0);
}

bool XletsTableView::checkCellIndex(CellIndex idx, XletType type) const
{
    switch (type) {
    case XletType::In:
        return idx.row >= 0 && idx.row < inletsRowCount()
            && idx.column >= 0 && idx.column < inletsColCount();
    case XletType::Out:
        return idx.row >= 0 && idx.row < outletsRowCount()
            && idx.column >= 0 && idx.column < outletsColCount();
    default:
        return false;
    }
}

QRectF XletsTableView::inletsBRect() const
{
    auto xoff = (width() - inletsWidth()) * 0.5;
    return { xoff, 0, inletsWidth(), inletsHeight() };
}

QRectF XletsTableView::outletsBRect() const
{
    auto xoff = (width() - outletsWidth()) * 0.5;
    return { xoff, inletsHeight(), outletsWidth(), outletsHeight() };
}

std::optional<CellIndex> XletsTableView::indexToCell(XletViewIndex vidx) const
{
    if (vidx.isInlet()) {
        auto idx = vidx.index;
        if (idx >= xlets_.inletCount())
            return {};

        return CellIndex { idx / data_.maxInputColumnCount(), idx % data_.maxInputColumnCount() };

    } else if (vidx.isOutlet()) {
        auto idx = vidx.index;
        if (idx >= xlets_.outletCount())
            return {};

        return CellIndex { idx / data_.maxOutputColumnCount(), idx % data_.maxOutputColumnCount() };

    } else
        return {};
}

std::optional<XletViewIndex> XletsTableView::cellToIndex(CellIndex cellIdx, XletType type) const
{
    if (!checkCellIndex(cellIdx, type))
        return {};

    switch (type) {
    case XletType::In: {
        auto idx = cellIdx.row * data_.maxInputColumnCount() + cellIdx.column;
        if (idx < xlets_.inletCount())
            return XletViewIndex { static_cast<XletIndex>(idx), type };

    } break;
    case XletType::Out: {
        auto idx = cellIdx.row * data_.maxOutputColumnCount() + cellIdx.column;
        if (idx < xlets_.outletCount())
            return XletViewIndex { static_cast<XletIndex>(idx), type };
    } break;
    default:
        break;
    }

    return {};
}

std::optional<XletViewIndex> XletsTableView::posToIndex(const QPoint& pos) const
{
    auto in_rect = inletsBRect();
    if (in_rect.contains(pos)) {
        int col = (pos.x() - in_rect.x()) / (int)XLET_W;
        int row = (pos.y() - in_rect.y()) / (int)XLET_H;

        return cellToIndex({ row, col }, XletType::In);
    }

    auto out_rect = outletsBRect();
    if (out_rect.contains(pos)) {
        int col = (pos.x() - out_rect.x()) / (int)XLET_W;
        int row = (pos.y() - out_rect.y()) / (int)XLET_H;

        return cellToIndex({ row, col }, XletType::Out);
    }

    return {};
}

std::optional<QPoint> XletsTableView::indexToPos(XletViewIndex vidx) const
{
    auto cell = indexToCell(vidx);
    if (cell)
        return QPointF { cell->column * XLET_W, cell->row * XLET_H }.toPoint();
    else
        return {};
}

void XletsTableView::placeXlets(const QPointF& origin)
{
    auto in_xoff = origin + inletsBRect().topLeft();

    for (int i = 0; i < xlets_.inletCount(); i++) {
        XletViewIndex xi { static_cast<XletIndex>(i), XletType::In };
        auto xlet = xlets_.xletAtIndex(xi);
        if (xlet) {
            auto pt = in_xoff + xletRect(xi).topLeft();
            xlet->setPos(pt);
        }
    }

    auto out_xoff = origin + outletsBRect().topLeft();
    for (int i = 0; i < xlets_.outletCount(); i++) {
        XletViewIndex xi { static_cast<XletIndex>(i), XletType::Out };
        auto xlet = xlets_.xletAtIndex(xi);
        if (xlet) {
            auto pt = out_xoff + xletRect(xi).topLeft();
            xlet->setPos(pt);
        }
    }
}

QRect XletsView::xletRect(XletViewIndex idx) const
{
    auto pt = indexToPos(idx);
    if (!pt)
        return {};

    return QRect(pt->x(), pt->y(), XLET_W, XLET_H);
}

std::optional<QPointF> DeviceXlets::connectionPoint(XletViewIndex vidx) const
{
    auto xlet = xletAtIndex(vidx);
    if (!xlet || !xlet->isVisible())
        return {};

    return xlet->pos() + QPointF(XLET_W / 2, vidx.isOutlet() * XLET_H);
}

size_t DeviceXlets::userViewCount() const
{
    return user_views_.size();
}

void XletsTableView::paint(QPainter* painter, const QPoint& origin)
{
    auto xrect = boundingRect().translated(origin);
    // fill with white bg
    painter->setBrush(QColor(255, 255, 255));
    painter->drawRect(xrect);

    // draw xlet delimiter
    if ((inletsRowCount() * outletsRowCount()) > 1) {
        auto out_y = inletsHeight() + xrect.top();
        painter->drawLine(xrect.left(), out_y, xrect.right(), out_y);
    }
}

bool XletsTableView::setData(const SharedDeviceData& data)
{
    if (!data)
        return false;

    data_ = data->logicViewData();
    return true;
}

} // namespace ceam
