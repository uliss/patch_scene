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
     * Number of view cells
     */
    qsizetype cellCount() const;

    /**
     * @return pointer to device xlet or nullptr if notfound
     */
    DeviceXlet* xletAtIndex(int index);
    const DeviceXlet* xletAtIndex(int index) const;

    /**
     * @return pointer to device xlet by given cell index or nullptr if notfound
     */
    DeviceXlet* xletAtCell(int row, int col);
    const DeviceXlet* xletAtCell(int row, int col) const;

    /**
     * @return max number of xlets in row
     */
    int maxColumnCount() const { return max_cols_; }

    /**
     * Set max number of columns in row
     * @note you should call placeXlets() to update xlet positions
     */
    bool setMaxColumnCount(int n);

    /**
     * @return current number of filled rows
     */
    int rowCount() const;

    /**
     * convert cell index to linear xlet index
     */
    std::optional<int> cellToIndex(int row, int col) const;

    /**
     * convert cell index to linear xlet index
     */
    std::optional<int> cellToIndex(CellIndex cellIdx) const;

    /**
     * convert linear xlet index to cell index (row, col)
     */
    std::optional<CellIndex> indexToCell(int index) const;

    /**
     * convert point position to linear xlet index
     * @param pos - point position. First xlet starts at QPoint(0, 0)
     * @note index is calculated, no DeviceXlet scene position is checked!
     * @return xlet index or empty
     */
    std::optional<int> posToIndex(const QPoint& pos) const;

    /**
     * convert point position to cell xlet index
     * @param pos - point position. First xlet starts at QPoint(0, 0)
     * @note index is calculated, no DeviceXlet scene position is checked!
     * @return xlet index or empty
     */
    std::optional<CellIndex> posToCell(const QPoint& pos) const;

    /**
     * @return connection point relative to parent Device graphics item
     * @note xlets should be placed, return real DeviceXlet scene position!
     */
    QPointF connectionPoint(int index) const;

    /**
     * @return calculated xlet bounding rectangle
     * @param index - xlet linear index
     * @return rect, relative to QPoint(0, 0)
     */
    QRect xletRect(int index) const;

    /**
     * place xlet DeviceXlet graphics item relative to given origin point
     * @param origin - origin point, relative to parent Device
     */
    void placeXlets(const QPointF& origin);

    /**
     * remove all xlets
     */
    void clear();

    /**
     * @return bounding rect of all xlets relative to QPoint(0, 0)
     */
    QRectF boundingRect() const;

private:
    QList<DeviceXlet*> xlets_;
    int max_cols_;
};

} // namespace ceam

#endif // DEVICE_XLET_VIEW_H
