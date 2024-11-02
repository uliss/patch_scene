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
#ifndef XLETS_VIEW_H
#define XLETS_VIEW_H

#include "device_common.h"

namespace ceam {

class DeviceXlets;

class XletsView {
    QString name_;

public:
    explicit XletsView(const QString& name);
    virtual ~XletsView();

    virtual qreal width() const = 0;
    virtual qreal height() const = 0;
    virtual void paint(QPainter* painter, const QPoint& origin) = 0;

    /**
     * @return calculated xlet bounding rectangle
     * @param index - xlet view linear index
     * @return rect, relative to QPoint(0, 0)
     */
    QRect xletRect(XletViewIndex idx) const;

    /**
     * convert point position to xlet view index
     * @param pos - point position. First xlet starts at QPoint(0, 0)
     * @note info is calculated, no DeviceXlet scene position is checked!
     * @return xlet view index or empty
     */
    virtual std::optional<XletViewIndex> posToIndex(const QPoint& pos) const = 0;

    /**
     * convert xlet view index to position (top left corner)
     * @param pos - point position. First xlet starts at QPoint(0, 0)
     * @note info is calculated, no DeviceXlet scene position is checked!
     * @return xlet position or empty
     */
    virtual std::optional<QPoint> indexToPos(XletViewIndex vidx) const = 0;

    /**
     * place xlet DeviceXlet graphics item relative to given origin point
     * @param origin - origin point, relative to parent Device
     */
    virtual void placeXlets(const QPointF& origin) = 0;

    /**
     * @return bounding rect of all xlets relative to QPoint(0, 0)
     */
    QRectF boundingRect() const;

    virtual bool setData(const SharedDeviceData& data);

    const QString& name() const { return name_; }
    void setName(const QString& name) { name_ = name; }
};

class XletsLogicView : public XletsView {
    DeviceXlets& xlets_;
    XletsLogicViewData data_;

public:
    XletsLogicView(const QString& name, DeviceXlets& xlets);

    qreal width() const final;
    qreal height() const final;
    std::optional<XletViewIndex> posToIndex(const QPoint& pos) const final;
    std::optional<QPoint> indexToPos(XletViewIndex vidx) const final;
    void placeXlets(const QPointF& origin) final;
    void paint(QPainter* painter, const QPoint& origin);

    bool setData(const SharedDeviceData& data) final;
    XletsLogicViewData& data() { return data_; }
    const XletsLogicViewData& data() const { return data_; }

    /**
     * convert xlet view index to cell index (row, col)
     */
    std::optional<CellIndex> indexToCell(XletViewIndex vidx) const;

    /**
     * convert cell index to xlet view index
     */
    std::optional<XletViewIndex> cellToIndex(CellIndex cellIdx, XletType type) const;

private:
    qreal inletsHeight() const;
    qreal inletsWidth() const;
    qreal outletsHeight() const;
    qreal outletsWidth() const;

    int inletsColCount() const;
    int inletsRowCount() const;
    int outletsColCount() const;
    int outletsRowCount() const;

    bool checkCellIndex(CellIndex idx, XletType type) const;

    QRectF inletsBRect() const;
    QRectF outletsBRect() const;
};

}

#endif // XLETS_VIEW_H
