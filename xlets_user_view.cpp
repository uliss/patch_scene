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
#include "device_xlet_view.h"

#include <QJsonObject>
#include <QPainter>

namespace {

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;

}

namespace ceam {

XletsUserView::XletsUserView(const QString& name, DeviceXlets& xlets)
    : XletsView(name)
    , xlets_(xlets)
{
}

qreal XletsUserView::width() const
{
    return data_.columnCount() * XLET_W;
}

qreal XletsUserView::height() const
{
    return data_.rowCount() * XLET_H;
}

void XletsUserView::paint(QPainter* painter, const QPoint& origin)
{
    // fill with white bg
    auto xrect = boundingRect().translated(origin);
    painter->setBrush(QColor(255, 255, 255));
    painter->drawRect(xrect);
}

std::optional<XletViewIndex> XletsUserView::posToIndex(const QPoint& pos) const
{
    int col = pos.x() / static_cast<int>(XLET_W);
    int row = pos.y() / static_cast<int>(XLET_H);

    int cell_idx = row * data_.columnCount() + col;

    auto idx = data_.xletAt(cell_idx);
    if (idx.isNull())
        return {};
    else
        return idx;
}

std::optional<QPoint> XletsUserView::indexToPos(XletViewIndex vidx) const
{
    for (int i = 0; i < data_.cellCount(); i++) {
        if (data_.xletAt(i) == vidx) {
            int col = i % data_.columnCount();
            int row = i / data_.columnCount();

            return QPoint(col * XLET_W, row * XLET_H);
        }
    }

    return {};
}

void XletsUserView::placeXlets(const QPointF& origin)
{
    xlets_.setVisible(false);

    for (int i = 0; i < data_.cellCount(); i++) {
        auto vi = data_.xletAt(i);
        int col = i % data_.columnCount();
        int row = i / data_.columnCount();

        auto xlet = xlets_.xletAtIndex(vi);
        if (xlet) {
            xlet->setPos(origin + QPoint(col * XLET_W, row * XLET_H));
            xlet->setVisible(true);
        }
    }
}

void XletsUserView::setData(const XletsUserViewData& data)
{
    data_ = data;
}

} // namespace ceam
