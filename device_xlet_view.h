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
#ifndef DEVICE_XLET_VIEW_H
#define DEVICE_XLET_VIEW_H

#include "socket.h"

#include <QList>
#include <QPointF>
#include <QRectF>
#include <optional>

class QGraphicsItem;

namespace ceam {

class DeviceXlet;
class XletData;

using CellIndex = std::pair<int, int>;

class DeviceXletView {
public:
    DeviceXletView();
    ~DeviceXletView();

    bool add(const XletData& data, XletType type, QGraphicsItem* parent);

    /**
     * Return number of xlets
     */
    qsizetype count() const { return xlets_.count(); }

    /**
     * Number ov fiew cells
     */
    qsizetype cellCount() const;

    DeviceXlet* xletAtIndex(int index);
    const DeviceXlet* xletAtIndex(int index) const;

    DeviceXlet* xletAtCell(int row, int col);
    const DeviceXlet* xletAtCell(int row, int col) const;

    int rowCount() const { return nrows_; }
    int columnCount() const { return ncols_; }

    int realRowCount() const;

    bool setRowCount(int n);
    bool setColumnCount(int n);

    std::optional<int> cellToIndex(int row, int col) const;
    std::optional<int> cellToIndex(CellIndex cellIdx) const;
    std::optional<CellIndex> indexToCell(int index) const;
    std::optional<int> posToIndex(const QPoint& pos) const;
    std::optional<CellIndex> posToCell(const QPoint& pos) const;

    QPointF connectionPoint(int index) const;

    QRect xletRect(int index) const;

    void placeXlets(const QPointF& pos);

    void clear();

    QRectF boundingRect() const;

private:
    QList<DeviceXlet*> xlets_;
    int nrows_ { 1 }, ncols_ { 4 };
};

} // namespace ceam

#endif // DEVICE_XLET_VIEW_H
