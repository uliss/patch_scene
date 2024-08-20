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
#include "furniture_item_model.h"

namespace {
constexpr int COL_FURNITURE_TITLE = 0;
constexpr int COL_FURNITURE_COUNT = 1;
}

using namespace ceam;

FurnitureItemModel::FurnitureItemModel(QObject* parent)
    : QStandardItemModel { parent }
{
    setHorizontalHeaderLabels({ tr("Title"), tr("Count") });
}

bool FurnitureItemModel::addFurniture(const SharedDeviceData& data)
{
    if (!data || data->category() != ItemCategory::Furniture)
        return false;

    // auto id = data->calcModelId();
    // auto it = furniture_map_.find(id);
    // if (it == furniture_map_.end()) {
    //     furniture_map_.insert(id, { data->title(), 1 });
    // } else {
    //     it->second++;
    // }

    return true;
}

bool FurnitureItemModel::removeFurniture(const SharedDeviceData& data)
{
    if (!data || data->category() != ItemCategory::Furniture)
        return false;

    auto id = data->calcModelId();
    auto it = furniture_map_.find(id);
    if (it != furniture_map_.end()) {
        it->second--;
        if (it->second <= 0)
            furniture_map_.remove(id);

        return true;
    } else {
        return false;
    }
}
