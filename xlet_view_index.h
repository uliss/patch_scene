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
#ifndef XLET_VIEW_INDEX_H
#define XLET_VIEW_INDEX_H

#include "socket.h"

namespace ceam {

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

    bool isNull() const;

public:
    QJsonObject toJson() const;
    static std::optional<XletViewIndex> fromJson(const QJsonValue& v);
    static XletViewIndex null() { return { 0, XletType::None }; }
};

} // namespace ceam

#endif // XLET_VIEW_INDEX_H
