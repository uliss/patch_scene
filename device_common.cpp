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

const char* toString(ItemCategory cat)
{
    switch (cat) {
    case ItemCategory::Instrument:
        return "instrument";
    case ItemCategory::Human:
        return "human";
    case ItemCategory::Furniture:
        return "furniture";
    case ItemCategory::Send:
        return "send";
    case ItemCategory::Return:
        return "return";
    case ItemCategory::Device:
    default:
        return "device";
    }
}

bool fromQString(const QString& str, ItemCategory& cat)
{
    if (str.isEmpty()) {
        cat = ItemCategory::Device;
        return true;
    }

    auto icat = str.toLower();
    if (icat == "device") {
        cat = ItemCategory::Device;
        return true;
    } else if (icat == "return") {
        cat = ItemCategory::Return;
        return true;
    } else if (icat == "send") {
        cat = ItemCategory::Send;
        return true;
    } else if (icat == "furniture") {
        cat = ItemCategory::Furniture;
        return true;
    } else if (icat == "human") {
        cat = ItemCategory::Human;
        return true;
    } else if (icat == "instruments") {
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
