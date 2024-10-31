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

#include <QGraphicsScene>
#include <QPainter>

namespace {

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;
}

namespace ceam {

DeviceXlets::DeviceXlets()
{
}

DeviceXlets::~DeviceXlets()
{
    clear();
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

void DeviceXlets::clear()
{
    clearXlets(inlets_);
    clearXlets(outlets_);
}

void DeviceXlets::initDefaultView()
{
    if (views_.empty()) {
        views_.push_back(std::make_unique<XletsTableView>("logical", *this));
        current_view_ = views_.back().get();
    } else {
        views_.front() = std::make_unique<XletsTableView>("logical", *this);
        current_view_ = views_.front().get();
    }
}

bool DeviceXlets::setCurrentView(const QString& name)
{
    for (auto& v : views_) {
        if (v && v->name() == name) {
            current_view_ = v.get();
            return true;
        }
    }

    return false;
}

void DeviceXlets::setData(const SharedDeviceData& data)
{
    for (auto& v : views_)
        v->setData(data);
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

DeviceXletsView::DeviceXletsView(const QString& name)
    : name_(name)
{
}

DeviceXletsView::~DeviceXletsView() { }

QRectF DeviceXletsView::boundingRect() const
{
    return { 0, 0, width(), height() };
}

void DeviceXletsView::setData(const SharedDeviceData& /*data*/)
{
}

XletsTableView::XletsTableView(const QString& name, DeviceXlets& xlets)
    : DeviceXletsView(name)
    , max_inlets_cols_(DeviceData::DEF_COL_COUNT)
    , max_outlets_cols_(DeviceData::DEF_COL_COUNT)
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

bool XletsTableView::setMaxInletsCols(XletIndex n)
{
    if (n < DeviceData::MIN_COL_COUNT || n > DeviceData::MAX_COL_COUNT)
        return false;

    max_inlets_cols_ = n;
    return true;
}

bool XletsTableView::setMaxOutletsCols(XletIndex n)
{
    if (n < DeviceData::MIN_COL_COUNT || n > DeviceData::MAX_COL_COUNT)
        return false;

    max_outlets_cols_ = n;
    return true;
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
    return std::min<int>(max_inlets_cols_, xlets_.inletCount());
}

qreal XletsTableView::outletsHeight() const
{
    return outletsRowCount() * XLET_H;
}

int XletsTableView::inletsRowCount() const
{
    auto n = xlets_.inletCount();
    return n / max_inlets_cols_ + (n % max_inlets_cols_ > 0);
}

int XletsTableView::outletsColCount() const
{
    return std::min<int>(max_outlets_cols_, xlets_.outletCount());
}

int XletsTableView::outletsRowCount() const
{
    auto n = xlets_.outletCount();
    return n / max_outlets_cols_ + (n % max_outlets_cols_ > 0);
}

bool XletsTableView::checkCellIndex(CellIndex idx, XletType type) const
{
    switch (type) {
    case XletType::In:
        return idx.first >= 0 && idx.first < inletsRowCount()
            && idx.second >= 0 && idx.second < inletsColCount();
    case XletType::Out:
        return idx.first >= 0 && idx.first < outletsRowCount()
            && idx.second >= 0 && idx.second < outletsColCount();
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

        return std::make_pair(idx / maxInletsCols(), idx % maxInletsCols());

    } else if (vidx.isOutlet()) {
        auto idx = vidx.index;
        if (idx >= xlets_.outletCount())
            return {};

        return std::make_pair(idx / maxOutletsCols(), idx % maxOutletsCols());

    } else
        return {};
}

std::optional<XletViewIndex> XletsTableView::cellToIndex(CellIndex cellIdx, XletType type) const
{
    if (!checkCellIndex(cellIdx, type))
        return {};

    switch (type) {
    case XletType::In: {
        auto idx = cellIdx.first * maxInletsCols() + cellIdx.second;
        if (idx < xlets_.inletCount())
            return XletViewIndex { static_cast<XletIndex>(idx), type };

    } break;
    case XletType::Out: {
        auto idx = cellIdx.first * maxOutletsCols() + cellIdx.second;
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

QRect XletsTableView::xletRect(XletViewIndex idx) const
{
    auto cell_pos = indexToCell(idx);
    if (!cell_pos)
        return {};

    return QRect(cell_pos->second * XLET_W, cell_pos->first * XLET_H, XLET_W, XLET_H);
}

std::optional<QPointF> XletsTableView::connectionPoint(XletViewIndex vidx) const
{
    auto xlet = xlets_.xletAtIndex(vidx);
    if (!xlet || !xlet->isVisible())
        return {};

    return xlet->pos() + QPointF(XLET_W / 2, vidx.isOutlet() * XLET_H);
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

void XletsTableView::setData(const SharedDeviceData& data)
{
    if (!data)
        return;

    setMaxInletsCols(data->maxInputColumnCount());
    setMaxOutletsCols(data->maxOutputColumnCount());
}

} // namespace ceam
