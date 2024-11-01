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
#include "xlets_user_view.h"

namespace {
constexpr int MIN_COL_COUNT = 1;
constexpr int MAX_COL_COUNT = 24;
constexpr int DEF_COL_COUNT = 6;
constexpr int MIN_ROW_COUNT = 1;
constexpr int MAX_ROW_COUNT = 24;
constexpr int DEF_ROW_COUNT = 6;

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

const ceam::XletViewIndex NO_XLET_IDX(0, ceam::XletType::None);

}

namespace ceam {

XletsUserView::XletsUserView(const QString& name)
    : DeviceXletsView(name)
    , num_cols_(DEF_COL_COUNT)
    , num_rows_(DEF_ROW_COUNT)
{
    // XletsUserView d("");

    xlets_.resize(cellCount(), NO_XLET_IDX);
}

void XletsUserView::setColumnCount(int n)
{
    num_cols_ = qBound(MIN_COL_COUNT, n, MAX_COL_COUNT);
    xlets_.resize(cellCount(), NO_XLET_IDX);
}

void XletsUserView::setRowCount(int n)
{
    num_rows_ = qBound(MIN_ROW_COUNT, n, MAX_ROW_COUNT);
    xlets_.resize(cellCount(), NO_XLET_IDX);
}

int XletsUserView::cellCount() const
{
    return num_cols_ * num_rows_;
}

qreal XletsUserView::width() const
{
    return num_cols_ * XLET_W;
}

qreal XletsUserView::height() const
{
    return num_rows_ * XLET_H;
}

void XletsUserView::paint(QPainter* painter, const QPoint& origin)
{
}

std::optional<XletViewIndex> XletsUserView::posToIndex(const QPoint& pos) const
{
    int col = static_cast<int>(pos.x()) % static_cast<int>(XLET_W);
    int row = pos.y() / XLET_H;

    int cell_idx = col * num_cols_ + row;
    if (cell_idx < 0 || cell_idx >= xlets_.size())
        return {};

    auto idx = xlets_[cell_idx];
    if (idx == NO_XLET_IDX)
        return {};
    else
        return idx;
}

} // namespace ceam
