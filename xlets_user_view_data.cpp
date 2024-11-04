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
#include "xlets_user_view_data.h"
#include "logging.hpp"

#include <QJsonArray>
#include <QJsonObject>

namespace {
constexpr const char* JSON_KEY_INDEXES = "indexes";
constexpr const char* JSON_KEY_NUM_COLS = "num-cols";
constexpr const char* JSON_KEY_NUM_ROWS = "num-rows";
constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_DEST = "dest";
}

namespace ceam {

XletsUserViewData::XletsUserViewData(int row, int cols)
    : name_("User")
{
    setColumnCount(cols);
    setRowCount(row);
}

void XletsUserViewData::setColumnCount(int n)
{
    col_count_ = qBound(MIN_COL_COUNT, n, MAX_COL_COUNT);
    xlets_idx_.resize(cellCount(), XletViewIndex::null());
}

void XletsUserViewData::setRowCount(int n)
{
    row_count_ = qBound(MIN_ROW_COUNT, n, MAX_ROW_COUNT);
    xlets_idx_.resize(cellCount(), XletViewIndex::null());
}

int XletsUserViewData::cellCount() const
{
    return col_count_ * row_count_;
}

XletViewIndex XletsUserViewData::xletAt(int pos) const
{
    if (pos < 0 || pos >= xlets_idx_.size())
        return XletViewIndex::null();
    else
        return xlets_idx_[pos];
}

bool XletsUserViewData::insertXlet(CellIndex cellIdx, XletViewIndex vidx)
{
    if (!checkCell(cellIdx))
        return false;

    auto idx = cellToIndex(cellIdx);
    if (idx >= cellCount())
        return false;

    auto it = std::find(xlets_idx_.begin(), xlets_idx_.end(), vidx);
    if (it != xlets_idx_.end()) { // xlet exists
        *it = XletViewIndex::null();
    }

    if (xlets_idx_[idx].isNull()) { // insert into free cell
        xlets_idx_[idx] = vidx;
    } else {
        if (xlets_idx_.back().isNull()) { // insert before
            WARN() << "insert before";
            xlets_idx_.insert(xlets_idx_.begin() + idx, vidx);
            xlets_idx_.pop_back();
        } else { // insert info free space
            auto empty_it = std::find(xlets_idx_.begin(), xlets_idx_.end(), XletViewIndex::null());
            if (empty_it == xlets_idx_.end()) // no free space
                return false;

            *empty_it = xlets_idx_[idx];
            xlets_idx_[idx] = vidx;
        }
    }

    return true;
}

bool XletsUserViewData::clearCell(CellIndex cellIdx)
{
    if (!checkCell(cellIdx))
        return false;

    auto idx = cellToIndex(cellIdx);
    if (idx >= cellCount())
        return false;

    xlets_idx_[idx] = XletViewIndex::null();
    return true;
}

XletViewIndex XletsUserViewData::xletAtCell(CellIndex cellIdx) const
{
    if (!checkCell(cellIdx))
        return XletViewIndex::null();

    auto idx = cellToIndex(cellIdx);
    if (idx >= cellCount())
        return XletViewIndex::null();

    return xlets_idx_[idx];
}

bool XletsUserViewData::operator==(const XletsUserViewData& vd) const
{
    if (this == &vd)
        return true;

    return col_count_ == vd.col_count_
        && row_count_ == vd.row_count_
        && name_ == vd.name_
        && xlets_idx_ == vd.xlets_idx_;
}

QJsonValue XletsUserViewData::toJson() const
{
    QJsonObject obj;

    obj[JSON_KEY_NUM_COLS] = col_count_;
    obj[JSON_KEY_NUM_ROWS] = row_count_;
    obj[JSON_KEY_NAME] = name_;

    if (!xlets_idx_.empty()) {
        QJsonArray arr;
        int count = 0;

        for (int i = 0; i < xlets_idx_.size(); i++) {
            auto& x = xlets_idx_[i];
            auto j = x.toJson();
            if (j.isEmpty())
                continue;

            j[JSON_KEY_DEST] = i;
            arr.append(j);
            count++;
        }

        if (count > 0)
            obj[JSON_KEY_INDEXES] = arr;
    }

    return obj;
}

std::optional<XletsUserViewData> XletsUserViewData::fromJson(const QJsonValue& v)
{
    if (!v.isObject())
        return {};

    XletsUserViewData res;
    auto obj = v.toObject();
    res.setColumnCount(obj[JSON_KEY_NUM_COLS].toInt(DEF_COL_COUNT));
    res.setRowCount(obj[JSON_KEY_NUM_ROWS].toInt(DEF_ROW_COUNT));

    res.name_ = obj[JSON_KEY_NAME].toString("User");

    auto idxs = obj[JSON_KEY_INDEXES].toArray();

    std::vector<XletViewIndex> indexes;
    indexes.assign(res.cellCount(), XletViewIndex { 0, XletType::None });

    for (const auto& item : idxs) {
        if (!item.isObject())
            continue;

        auto obj = item.toObject();
        auto idx = XletViewIndex::fromJson(obj);
        if (idx) {
            auto dest_idx = obj[JSON_KEY_DEST].toInt(-1);
            if (dest_idx >= 0 && dest_idx < indexes.size()) {
                indexes[dest_idx] = *idx;
            } else {
                WARN() << "invalid destination index:" << dest_idx;
            }
        }
    }

    res.xlets_idx_ = indexes;

    return res;
}

} // namespace ceam
