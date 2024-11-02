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
class DeviceXlets;
class XletsView;
class XletData;
class XletsLogicView;

class DeviceXlets {
public:
    DeviceXlets();
    ~DeviceXlets();

    /**
     * append xlet to the scene
     * @return pointer to added xlet
     */
    DeviceXlet* append(const XletData& data, XletType type, QGraphicsItem* parent);

    /**
     * Checks if there's no inlets and outlets
     */
    bool isEmpty() const;

    DeviceXlet* xletAtIndex(XletViewIndex vidx);
    const DeviceXlet* xletAtIndex(XletViewIndex vidx) const;

    DeviceXlet* inletAt(XletIndex idx);
    const DeviceXlet* inletAt(XletIndex idx) const;

    DeviceXlet* outletAt(XletIndex idx);
    const DeviceXlet* outletAt(XletIndex idx) const;

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
     * @return number of views
     */
    size_t userViewCount() const;

    /**
     * remove all xlets
     */
    void clearXlets();

    /**
     * remove all views and clear current view pointer
     */
    void clearViews();

    /**
     * init default view
     */
    void initDefaultView();

    XletsView* currentView() { return current_view_; }
    const XletsView* currentView() const { return current_view_; }
    bool setCurrentView(const QString& name);

    void setData(const SharedDeviceData& data);
    void setVisible(bool value);

    bool appendView(std::unique_ptr<XletsView> view);

private:
    void clearXlets(QList<DeviceXlet*>& xlets);

private:
    QList<DeviceXlet*> inlets_, outlets_;
    std::unique_ptr<XletsLogicView> logic_view_;
    std::vector<std::unique_ptr<XletsView>> user_views_;
    XletsView* current_view_ { nullptr };
};

} // namespace ceam

#endif // DEVICE_XLET_VIEW_H
