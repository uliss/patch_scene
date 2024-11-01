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

#include "device_common.h"
#include "socket.h"

#include <QList>
#include <QPointF>
#include <QRectF>
#include <optional>

class QGraphicsItem;
class QPainter;

namespace ceam {

class DeviceXlet;
class DeviceXletsView;
class XletData;

using CellIndex = std::pair<int, int>;

struct XletViewIndex {
    XletIndex index;
    XletType type;

    XletViewIndex(XletIndex i, XletType t)
        : index(i)
        , type(t)
    {
    }

    bool operator==(const XletViewIndex& idx) const
    {
        return index == idx.index
            && type == idx.type;
    }

    bool operator!=(const XletViewIndex& idx) const
    {
        return !operator==(idx);
    }

    bool isInlet() const { return type == XletType::In; }
    bool isOutlet() const { return type == XletType::Out; }
};

class DeviceXlets {
public:
    DeviceXlets();
    ~DeviceXlets();

    /**
     * append xlet to the scene
     */
    bool append(const XletData& data, XletType type, QGraphicsItem* parent);

    /**
     * Checks if there's no inlets and outlets
     */
    bool isEmpty() const;

    DeviceXlet* xletAtIndex(XletViewIndex vidx);
    const DeviceXlet* xletAtIndex(XletViewIndex vidx) const;

    /**
     * @return calc connection point in device coords for given xlet view index
     */
    std::optional<QPointF> connectionPoint(XletViewIndex vidx) const;

    /**
     * Return number of inlets
     */
    qsizetype inletCount() const { return inlets_.count(); }

    /**
     * Return number of inlets
     */
    qsizetype outletCount() const { return outlets_.count(); }

    /**
     * remove all xlets
     */
    void clear();

    /**
     * init default view
     */
    void initDefaultView();

    DeviceXletsView* currentView() { return current_view_; }
    const DeviceXletsView* currentView() const { return current_view_; }
    bool setCurrentView(const QString& name);

    void setData(const SharedDeviceData& data);

private:
    void clearXlets(QList<DeviceXlet*>& xlets);

private:
    QList<DeviceXlet*> inlets_, outlets_;
    std::vector<std::unique_ptr<DeviceXletsView>> views_;
    DeviceXletsView* current_view_ { nullptr };
};

class DeviceXletsView {
    QString name_;

public:
    explicit DeviceXletsView(const QString& name);
    virtual ~DeviceXletsView();

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

    virtual void setData(const SharedDeviceData& data);

    const QString& name() const { return name_; }
    void setName(const QString& name) { name_ = name; }
};

class XletsTableView : public DeviceXletsView {
    XletIndex max_inlets_cols_, max_outlets_cols_;
    DeviceXlets& xlets_;

public:
    XletsTableView(const QString& name, DeviceXlets& xlets);

    qreal width() const final;
    qreal height() const final;
    std::optional<XletViewIndex> posToIndex(const QPoint& pos) const final;
    std::optional<QPoint> indexToPos(XletViewIndex vidx) const final;
    void placeXlets(const QPointF& origin) final;
    void paint(QPainter* painter, const QPoint& origin);
    void setData(const SharedDeviceData& data) final;

    /**
     * convert xlet view index to cell index (row, col)
     */
    std::optional<CellIndex> indexToCell(XletViewIndex vidx) const;

    /**
     * convert cell index to xlet view index
     */
    std::optional<XletViewIndex> cellToIndex(CellIndex cellIdx, XletType type) const;

    /**
     * @return max number of inlets in row
     */
    XletIndex maxInletsCols() const { return max_inlets_cols_; }

    /**
     * @return max number of outlets in row
     */
    XletIndex maxOutletsCols() const { return max_outlets_cols_; }

    /**
     * Set max number of columns in row
     * @note you should call placeXlets() to update xlet positions
     */
    bool setMaxInletsCols(XletIndex n);

    /**
     * Set max number of columns in row
     * @note you should call placeXlets() to update xlet positions
     */
    bool setMaxOutletsCols(XletIndex n);

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

} // namespace ceam

#endif // DEVICE_XLET_VIEW_H
