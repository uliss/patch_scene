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
#include "device_xlet.h"

#include <QGraphicsScene>

namespace {
constexpr int MAX_ROW_COUNT = 8;
constexpr int MIN_ROW_COUNT = 1;
constexpr int MAX_COL_COUNT = 24;
constexpr int MIN_COL_COUNT = 2;

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;
}

namespace ceam {

DeviceXletView::DeviceXletView() { }

DeviceXletView::~DeviceXletView()
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

bool DeviceXletView::add(const XletData& data, XletType type, QGraphicsItem* parent)
{
    auto xlet = new DeviceXlet(data, type, xlets_.count(), parent);
    xlets_.push_back(xlet);
    return true;
}

qsizetype DeviceXletView::cellCount() const
{
    return nrows_ * ncols_;
}

DeviceXlet* DeviceXletView::xletAtIndex(int index)
{
    if (index < 0 || index >= xlets_.count())
        return nullptr;
    else
        return xlets_[index];
}

const DeviceXlet* DeviceXletView::xletAtIndex(int index) const
{
    if (index < 0 || index >= xlets_.count())
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

int DeviceXletView::realRowCount() const
{
    auto n = xlets_.count();
    return n / ncols_ + (n % ncols_ > 0);
}

bool DeviceXletView::setRowCount(int n)
{
    if (n < MIN_ROW_COUNT || n > MAX_ROW_COUNT)
        return false;

    nrows_ = n;
    return true;
}

bool DeviceXletView::setColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    ncols_ = n;
    return true;
}

std::optional<int> DeviceXletView::cellToIndex(int row, int col) const
{
    if (row < 0
        || row >= rowCount()
        || col < 0
        || col >= columnCount())
        return {};

    int idx = row * columnCount() + col;
    if (idx < xlets_.count())
        return idx;
    else
        return {};
}

std::optional<int> DeviceXletView::cellToIndex(CellIndex cellIdx) const
{
    return cellToIndex(cellIdx.first, cellIdx.second);
}

std::optional<CellIndex> DeviceXletView::indexToCell(int index) const
{
    if (index < 0 || index >= xlets_.count())
        return {};

    return std::make_pair(index / columnCount(), index % columnCount());
}

std::optional<int> DeviceXletView::posToIndex(const QPoint& pos) const
{
    int row = pos.x() / (int)XLET_H;
    int col = pos.y() / (int)XLET_W;

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

QPointF DeviceXletView::connectionPoint(int index) const
{
    if (index < 0 || index >= xlets_.count())
        return {};
    else {
        auto xlet = xlets_[index];
        return xlet->pos() + QPointF(XLET_W / 2, (xlet->xletType() == XletType::Out) * XLET_H);
    }
}

QRect DeviceXletView::xletRect(int index) const
{
    auto cell_pos = indexToCell(index);
    if (!cell_pos)
        return {};

    qWarning() << __FUNCTION__ << *cell_pos;

    return QRect(cell_pos->second * XLET_W, cell_pos->first * XLET_H, XLET_W, XLET_H);
}

void DeviceXletView::placeXlets(const QPointF& pos)
{
    for (int i = 0; i < xlets_.count(); i++) {
        auto pt = pos + xletRect(i).topLeft();
        xlets_[i]->setPos(pt);
    }
}

void DeviceXletView::clear()
{
    for (auto x : xlets_)
        delete x;

    xlets_.clear();
}

QRectF DeviceXletView::boundingRect() const
{
    return QRectF { 0, 0, ncols_ * XLET_W, realRowCount() * XLET_H };
}

} // namespace ceam
