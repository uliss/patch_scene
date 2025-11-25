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
#ifndef XLET_INFO_H
#define XLET_INFO_H

#include "socket.h"
#include "item_id.h"

namespace ceam {

class XletInfo {
    DeviceId id_;
    XletIndex index_;
    XletType type_;

public:
    XletInfo(DeviceId id, int index, XletType type)
        : id_(id)
        , type_(type)
        , index_(index)
    {
    }

    bool operator==(const XletInfo& xi) const
    {
        return xi.id_ == id_ && xi.type_ == type_ && xi.index_ == index_;
    }

    bool operator!=(const XletInfo& xi) const { return !operator==(xi); }

    DeviceId id() const { return id_; }
    XletType type() const { return type_; }
    XletIndex index() const { return index_; }

    bool isInlet() const { return type_ == XletType::In; }
    bool isOutlet() const { return type_ == XletType::Out; }
};

uint qHash(const XletInfo& key, size_t seed = 0);

} // namespace ceam

#endif // XLET_INFO_H
