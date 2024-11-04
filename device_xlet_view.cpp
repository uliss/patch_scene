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
#include "device_xlet_view.h"
#include "device_common.h"
#include "device_xlet.h"
#include "xlets_user_view.h"
#include "xlets_view.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QPainter>

namespace {

constexpr const char* DEFAULT_VIEW = "logical";
constexpr const char* JSON_KEY_CURRENT_VIEW = "current";
constexpr const char* JSON_KEY_VIEWS = "views";
constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_TYPE = "type";

}

namespace ceam {

DeviceXlets::DeviceXlets()
{
}

DeviceXlets::~DeviceXlets()
{
    clearXlets();
}

DeviceXlet* DeviceXlets::append(const XletData& data, XletType type, QGraphicsItem* parent)
{
    switch (type) {
    case XletType::In: {
        auto xlet = new DeviceXlet(data, XletInfo { DEV_NULL_ID, (XletIndex)inlets_.count(), type }, parent);
        inlets_.push_back(xlet);
        return inlets_.back();
    } break;
    case XletType::Out: {
        auto xlet = new DeviceXlet(data, XletInfo { DEV_NULL_ID, (XletIndex)outlets_.count(), type }, parent);
        outlets_.push_back(xlet);
        return outlets_.back();
    } break;
    default:
        return nullptr;
    }

    return nullptr;
}

bool DeviceXlets::isEmpty() const
{
    return inlets_.isEmpty() && outlets_.isEmpty();
}

DeviceXlet* DeviceXlets::xletAtIndex(XletViewIndex vidx)
{
    switch (vidx.type) {
    case XletType::In:
        if (vidx.index < inlets_.count())
            return inlets_[vidx.index];

        break;
    case XletType::Out:
        if (vidx.index < outlets_.count())
            return outlets_[vidx.index];

        break;
    default:
        break;
    }

    return nullptr;
}

const DeviceXlet* DeviceXlets::xletAtIndex(XletViewIndex vidx) const
{
    switch (vidx.type) {
    case XletType::In:
        if (vidx.index < inlets_.count())
            return inlets_[vidx.index];

        break;
    case XletType::Out:
        if (vidx.index < outlets_.count())
            return outlets_[vidx.index];

        break;
    default:
        break;
    }

    return nullptr;
}

DeviceXlet* DeviceXlets::inletAt(XletIndex idx)
{
    return (idx < inlets_.size()) ? inlets_[idx] : nullptr;
}

const DeviceXlet* DeviceXlets::inletAt(XletIndex idx) const
{
    return (idx < inlets_.size()) ? inlets_[idx] : nullptr;
}

DeviceXlet* DeviceXlets::outletAt(XletIndex idx)
{
    return (idx < outlets_.size()) ? outlets_[idx] : nullptr;
}

const DeviceXlet* DeviceXlets::outletAt(XletIndex idx) const
{
    return (idx < outlets_.size()) ? outlets_[idx] : nullptr;
}

void DeviceXlets::clearXlets()
{
    clearXlets(inlets_);
    clearXlets(outlets_);
}

bool DeviceXlets::removeXlet(XletViewIndex idx)
{
    switch (idx.type) {
    case XletType::In:
        return removeXlet(idx, inlets_);
    case XletType::Out:
        return removeXlet(idx, outlets_);
    default:
        return false;
    }
}

void DeviceXlets::clearViews()
{
    user_views_.clear();
    current_view_ = logic_view_.get();
}

void DeviceXlets::initDefaultView()
{
    logic_view_ = std::make_unique<XletsLogicView>(DEFAULT_VIEW, *this);
    current_view_ = logic_view_.get();
}

bool DeviceXlets::setCurrentView(const QString& name)
{
    if (name.isEmpty()) {
        current_view_ = logic_view_.get();
        return true;
    }

    for (auto& v : user_views_) {
        if (v && v->name() == name) {
            current_view_ = v.get();
            return true;
        }
    }

    return false;
}

void DeviceXlets::setData(const SharedDeviceData& data)
{
    if (!data)
        return;

    if (logic_view_)
        logic_view_->setData(data);

    clearViews();
    for (auto& x : data->userViewData()) {
        auto view = new XletsUserView(x.name(), *this);
        view->setData(x);
        user_views_.emplace_back(view);
    }

    if (data->currentUserView().isEmpty() || !setCurrentView(data->currentUserView()))
        current_view_ = logic_view_.get();
}

void DeviceXlets::setVisible(bool value)
{
    for (auto x : inlets_)
        x->setVisible(value);

    for (auto x : outlets_)
        x->setVisible(value);
}

bool DeviceXlets::appendView(std::unique_ptr<XletsView> view)
{
    if (!view)
        return false;

    user_views_.push_back(std::move(view));
    return true;
}

void DeviceXlets::clearXlets(QList<DeviceXlet*>& xlets)
{
    for (auto x : xlets) {
        auto scene = x->scene();
        if (scene) {
            scene->removeItem(x);
            delete x;
        }
    }

    xlets.clear();
}

bool DeviceXlets::removeXlet(XletViewIndex idx, QList<DeviceXlet*>& xlets)
{
    if (idx.index < xlets.count()) {
        auto xlet = xlets[idx.index];
        if (xlet) {
            xlet->scene()->removeItem(xlet);
            delete xlet;
            xlets.removeAll(xlet);
        }
        return true;
    } else
        return false;
}

} // namespace ceam
