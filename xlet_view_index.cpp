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
#include "xlet_view_index.h"
#include "logging.hpp"

#include <QJsonObject>

namespace {
constexpr const char* JSON_KEY_SRC = "src";
constexpr const char* JSON_KEY_TYPE = "type";
constexpr const char* JSON_STR_IN = "in";
constexpr const char* JSON_STR_OUT = "out";
}

namespace ceam {

bool XletViewIndex::isNull() const
{
    return (type == XletType::None) || *this == XletViewIndex::null();
}

QJsonObject XletViewIndex::toJson() const
{
    if (isNull())
        return {};

    QJsonObject res;
    res[JSON_KEY_SRC] = index;

    switch (type) {
    case XletType::In:
        res[JSON_KEY_TYPE] = JSON_STR_IN;
        break;
    case XletType::Out:
        res[JSON_KEY_TYPE] = JSON_STR_OUT;
        break;
    default:
        return {};
    }

    return res;
}

std::optional<XletViewIndex> XletViewIndex::fromJson(const QJsonValue& v)
{
    if (!v.isObject())
        return {};

    auto obj = v.toObject();

    XletViewIndex res(0, XletType::None);

    auto idx = obj[JSON_KEY_SRC].toInteger(-1);
    if (idx < 0 || idx > std::numeric_limits<typeof(XletViewIndex::index)>::max()) {
        WARN() << "invalid xlet index:" << idx;
        return {};
    }

    res.index = idx;

    auto type = obj[JSON_KEY_TYPE].toString();
    if (type == JSON_STR_IN) {
        res.type = XletType::In;
    } else if (type == JSON_STR_OUT) {
        res.type = XletType::Out;
    } else {
        WARN() << "invalid xlet type:" << type;
        return {};
    }

    return res;
}

} // namespace ceam
