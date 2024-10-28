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

#include <QGraphicsScene>

namespace {

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;
}

namespace ceam {

DeviceXletView::DeviceXletView()
    : max_cols_(DeviceData::DEF_COL_COUNT)
{
}

DeviceXletView::~DeviceXletView()
{
    clear();
}

bool DeviceXletView::append(const XletData& data, XletType type, QGraphicsItem* parent)
{
    auto xlet = new DeviceXlet(data, XletInfo { DEV_NULL_ID, (XletIndex)xlets_.count(), type }, parent);
    xlets_.push_back(xlet);
    return true;
}

qsizetype DeviceXletView::cellCount() const
{
    return rowCount() * max_cols_;
}

DeviceXlet* DeviceXletView::xletAtIndex(XletIndex index)
{
    if (index >= xlets_.count())
        return nullptr;
    else
        return xlets_[index];
}

const DeviceXlet* DeviceXletView::xletAtIndex(XletIndex index) const
{
    if (index >= xlets_.count())
        return nullptr;
    else
        return xlets_[index];
}

DeviceXlet* DeviceXletView::xletAtCell(int row, int col)
{
    auto idx = cellToIndex(row, col);
    return idx ? xlets_[*idx] : nullptr;
}

const DeviceXlet* DeviceXletView::xletAtCell(int row, int col) const
{
    auto idx = cellToIndex(row, col);
    return idx ? xlets_[*idx] : nullptr;
}

int DeviceXletView::rowCount() const
{
    auto n = xlets_.count();
    return n / max_cols_ + (n % max_cols_ > 0);
}

bool DeviceXletView::setMaxColumnCount(int n)
{
    if (n < DeviceData::MIN_COL_COUNT || n > DeviceData::MAX_COL_COUNT)
        return false;

    max_cols_ = n;
    return true;
}

std::optional<XletIndex> DeviceXletView::cellToIndex(int row, int col) const
{
    if (row < 0
        || row >= rowCount()
        || col < 0
        || col >= maxColumnCount())
        return {};

    int idx = row * maxColumnCount() + col;
    if (idx < xlets_.count())
        return idx;
    else
        return {};
}

std::optional<XletIndex> DeviceXletView::cellToIndex(CellIndex cellIdx) const
{
    return cellToIndex(cellIdx.first, cellIdx.second);
}

std::optional<CellIndex> DeviceXletView::indexToCell(int index) const
{
    if (index >= xlets_.count())
        return {};

    return std::make_pair(index / maxColumnCount(), index % maxColumnCount());
}

std::optional<XletIndex> DeviceXletView::posToIndex(const QPoint& pos) const
{
    int col = pos.x() / (int)XLET_W;
    int row = pos.y() / (int)XLET_H;

    return cellToIndex(row, col);
}

std::optional<CellIndex> DeviceXletView::posToCell(const QPoint& pos) const
{
    auto idx = posToIndex(pos);
    if (idx)
        return indexToCell(*idx);
    else
        return {};
}

std::optional<QPointF> DeviceXletView::connectionPoint(XletIndex index) const
{
    if (index >= xlets_.count())
        return {};
    else {
        auto xlet = xlets_[index];
        bool yoff = xlet->xletInfo().isOutlet();
        // bool yoff = (xlet->xletInfo().isOutlet() && !xlet->xletData().isBidirect())
        //     || (xlet->xletInfo().isInlet() && xlet->xletData().isBidirect());
        return xlet->pos() + QPointF(XLET_W / 2, yoff * XLET_H);
    }
}

QRect DeviceXletView::xletRect(XletIndex index) const
{
    auto cell_pos = indexToCell(index);
    if (!cell_pos)
        return {};

    return QRect(cell_pos->second * XLET_W, cell_pos->first * XLET_H, XLET_W, XLET_H);
}

void DeviceXletView::placeXlets(const QPointF& origin)
{
    for (int i = 0; i < xlets_.count(); i++) {
        auto pt = origin + xletRect(i).topLeft();
        xlets_[i]->setPos(pt);
    }
}

void DeviceXletView::clear()
{
    for (auto x : xlets_) {
        auto scene = x->scene();
        if (scene) {
            scene->removeItem(x);
            delete x;
        }
    }

    xlets_.clear();
}

QRectF DeviceXletView::boundingRect() const
{
    return QRectF { 0, 0, width(), rowCount() * XLET_H };
}

qreal DeviceXletView::width() const
{
    return std::min<int>(max_cols_, xlets_.count()) * XLET_W;
}

} // namespace ceam
