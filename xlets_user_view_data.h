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
#ifndef XLETS_USER_VIEW_DATA_H
#define XLETS_USER_VIEW_DATA_H

#include "cell_index.h"
#include "xlet_view_index.h"

namespace ceam {

class XletsUserViewData {
public:
    static constexpr int MIN_COL_COUNT = 1;
    static constexpr int MAX_COL_COUNT = 24;
    static constexpr int DEF_COL_COUNT = 6;
    static constexpr int MIN_ROW_COUNT = 1;
    static constexpr int MAX_ROW_COUNT = 24;
    static constexpr int DEF_ROW_COUNT = 3;

public:
    XletsUserViewData(int row = DEF_ROW_COUNT, int cols = DEF_COL_COUNT);

    int columnCount() const { return col_count_; }
    int rowCount() const { return row_count_; }

    void setColumnCount(int n);
    void setRowCount(int n);
    int cellCount() const;

    XletViewIndex xletAt(int pos) const;
    bool insertXlet(CellIndex cellIdx, XletViewIndex vidx);
    bool clearCell(CellIndex cellIdx);
    XletViewIndex xletAtCell(CellIndex cellIdx) const;

    const QString& name() const { return name_; }
    void setName(const QString& name) { name_ = name; }

    bool operator==(const XletsUserViewData& vd) const;

public:
    QJsonValue toJson() const;
    static std::optional<XletsUserViewData> fromJson(const QJsonValue& v);

private:
    int cellToIndex(CellIndex cellIdx) const
    {
        return cellIdx.row * col_count_ + cellIdx.column;
    }

    bool checkCell(CellIndex cellIdx) const
    {
        if (cellIdx.row < 0
            || cellIdx.row >= row_count_
            || cellIdx.column < 0
            || cellIdx.column >= col_count_) //
        {
            return false;
        } else
            return true;
    }

private:
    int col_count_ { DEF_COL_COUNT }, row_count_ { DEF_ROW_COUNT };
    std::vector<XletViewIndex> xlets_idx_;
    QString name_;
};

} // namespace ceam

#endif // XLETS_USER_VIEW_DATA_H
