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

#include <QJsonObject>

namespace {

constexpr qreal XLET_W = 22;
constexpr qreal XLET_H = 20;
constexpr qreal XLET_BOX_W = 8;
constexpr qreal XLET_BOX_H = 2;

constexpr const char* JSON_KEY_NUM_COLS = "ncols";
constexpr const char* JSON_KEY_NUM_ROWS = "nrows";
constexpr const char* JSON_KEY_INDEXES = "indexes";
constexpr const char* JSON_KEY_SRC = "src";
constexpr const char* JSON_KEY_TYPE = "type";
constexpr const char* JSON_KEY_DEST = "dest";
constexpr const char* VIEW_NAME = "user-grid";

}

namespace ceam {

XletsUserView::XletsUserView(const QString& name, DeviceXlets& xlets)
    : DeviceXletsView(name)
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

    // auto& data = data->userViewData();
    // if (!jv.isObject())
    //     return false;

    // auto vobj = jv.toObject();
    // auto view_data = vobj[VIEW_NAME];
    // if (!view_data.isObject())
    //     return false;

    // auto view = view_data.toObject();
    // setColumnCount(view[JSON_KEY_NUM_COLS].toInt(DEF_COL_COUNT));
    // setRowCount(view[JSON_KEY_NUM_ROWS].toInt(DEF_ROW_COUNT));

    // auto idxs = view[JSON_KEY_INDEXES].toArray();
    // if (idxs.isEmpty())
    //     return false;

    // std::vector<XletViewIndex> indexes;
    // indexes.assign(cellCount(), XletViewIndex { 0, XletType::None });

    // for (const auto& item : idxs) {
    //     if (!item.isObject())
    //         continue;

    //     auto obj = item.toObject();

    //     XletViewIndex vidx(0, XletType::None);
    //     auto src_xlet_idx = obj[JSON_KEY_SRC].toInteger(-1);
    //     if (src_xlet_idx < 0 || src_xlet_idx > std::numeric_limits<typeof(vidx.index)>::max()) {
    //         WARN() << "invalid xlet index:" << src_xlet_idx;
    //         continue;
    //     } else {
    //         vidx.index = src_xlet_idx;
    //     }

    //     auto src_xlet_type = obj[JSON_KEY_TYPE].toString();
    //     if (src_xlet_type == "in") {
    //         vidx.type = XletType::In;
    //     } else if (src_xlet_type == "out") {
    //         vidx.type = XletType::Out;
    //     } else {
    //         WARN() << "invalid xlet type:" << src_xlet_type;
    //         continue;
    //     }

    //     auto dest_idx = obj[JSON_KEY_DEST].toInteger(-1);
    //     if (dest_idx < 1 || dest_idx >= cellCount()) {
    //         WARN() << "invalid xlet index:" << src_xlet_idx;
    //         continue;
    //     }

    //     indexes[dest_idx] = vidx;
    // }

    // xlets_idx_ = indexes;
    // return true;
}

} // namespace ceam
