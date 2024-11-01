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
#ifndef XLETS_USER_VIEW_H
#define XLETS_USER_VIEW_H

#include "device_xlet_view.h"

namespace ceam {

class XletsUserView : public DeviceXletsView {
    int num_cols_, num_rows_;
    std::vector<XletViewIndex> xlets_;

public:
    XletsUserView(const QString& name);

    int columnCount() const { return num_cols_; }
    void setColumnCount(int n);

    int rowCount() const { return num_rows_; }
    void setRowCount(int n);

    int cellCount() const;

    qreal width() const final;
    qreal height() const final;
    void paint(QPainter* painter, const QPoint& origin) final;
    std::optional<XletViewIndex> posToIndex(const QPoint& pos) const final;
};

} // namespace ceam

#endif // XLETS_USER_VIEW_H
