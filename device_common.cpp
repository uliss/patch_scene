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
#include "device_common.h"

#include <QDebug>

constexpr const char* STR_DEVICE = "device";
constexpr const char* STR_SEND = "send";
constexpr const char* STR_RETURN = "return";
constexpr const char* STR_INSTRUMENT = "instrument";

const char* toString(ItemCategory cat)
{
    switch (cat) {
    case ItemCategory::Instrument:
        return STR_INSTRUMENT;
    case ItemCategory::Human:
        return "human";
    case ItemCategory::Furniture:
        return "furniture";
    case ItemCategory::Send:
        return STR_SEND;
    case ItemCategory::Return:
        return STR_RETURN;
    case ItemCategory::Device:
    default:
        return STR_DEVICE;
    }
}

bool fromQString(const QString& str, ItemCategory& cat)
{
    if (str.isEmpty()) {
        cat = ItemCategory::Device;
        return true;
    }

    auto icat = str.toLower();
    if (icat == STR_DEVICE) {
        cat = ItemCategory::Device;
        return true;
    } else if (icat == STR_RETURN) {
        cat = ItemCategory::Return;
        return true;
    } else if (icat == STR_SEND) {
        cat = ItemCategory::Send;
        return true;
    } else if (icat == "furniture") {
        cat = ItemCategory::Furniture;
        return true;
    } else if (icat == "human") {
        cat = ItemCategory::Human;
        return true;
    } else if (icat == STR_INSTRUMENT) {
        cat = ItemCategory::Instrument;
        return true;
    } else {
        qWarning() << __FUNCTION__ << "unknown category:" << str;
        return false;
    }
}

void foreachItemCategory(std::function<void(const char*, int)> fn)
{
    for (int i = static_cast<int>(ItemCategory::Device);
         i <= static_cast<int>(ItemCategory::Return);
         i++) //
    {
        fn(toString(static_cast<ItemCategory>(i)), i);
    }
}
