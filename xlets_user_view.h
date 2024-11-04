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

#include "xlets_view.h"

namespace ceam {

class XletsUserView : public XletsView {
    DeviceXlets& xlets_;
    XletsUserViewData data_;

public:
    XletsUserView(const QString& name, DeviceXlets& xlets);

    XletsUserViewData& data() { return data_; }
    const XletsUserViewData& data() const { return data_; }

    qreal width() const final;
    qreal height() const final;
    void paint(QPainter* painter, const QPoint& origin) final;
    std::optional<XletViewIndex> posToIndex(const QPoint& pos) const final;
    std::optional<QPoint> indexToPos(XletViewIndex vidx) const final;
    void placeXlets(const QPointF& origin) final;
    void setData(const XletsUserViewData& data);
};

} // namespace ceam

#endif // XLETS_USER_VIEW_H
