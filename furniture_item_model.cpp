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

#include <QSortFilterProxyModel>

namespace {
constexpr int COL_FURNITURE_TITLE = 0;
constexpr int COL_FURNITURE_COUNT = 1;
constexpr int FURNITURE_SORT_ROLE = Qt::UserRole + 1;

class SortProxy : public QSortFilterProxyModel {
public:
    SortProxy(QObject* parent)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
    }

    bool lessThan(const QModelIndex& a, const QModelIndex& b) const final
    {
        if (a.isValid() && b.isValid())
            if (a.column() == COL_FURNITURE_COUNT)
                return a.data(FURNITURE_SORT_ROLE).toInt() < b.data(FURNITURE_SORT_ROLE).toInt();

        return QSortFilterProxyModel::lessThan(a, b);
    }
};
}

using namespace ceam;

FurnitureItemModel::FurnitureItemModel(QObject* parent)
    : QStandardItemModel { parent }
{
    setHorizontalHeaderLabels({ tr("Title"), tr("Count") });

    proxy_ = new SortProxy(this);
    proxy_->setDynamicSortFilter(true);
    proxy_->setSourceModel(this);
}

bool FurnitureItemModel::addFurniture(const SharedDeviceData& data)
{
    if (!data || data->category() != ItemCategory::Furniture || data->title().isEmpty())
        return false;

    auto id = data->calcModelId();
    auto it = furniture_map_.find(id);
    if (it == furniture_map_.end()) {
        furniture_map_.insert(id, { data->title(), 1 });
    } else {
        it->second++;
    }

    updateData();

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
        if (it->second <= 0) {
            furniture_map_.remove(id);
            updateData();
        }

        return true;
    } else {
        return false;
    }
}

void FurnitureItemModel::clearItems()
{
    removeRows(0, rowCount());
}

void FurnitureItemModel::updateData()
{
    setRowCount(furniture_map_.size());

    int row = 0;
    for (auto it = furniture_map_.keyValueBegin(); it != furniture_map_.keyValueEnd(); ++it) {
        auto type = item(row, COL_FURNITURE_TITLE);
        if (!type) {
            type = new QStandardItem(it->second.first);
            setItem(row, COL_FURNITURE_TITLE, type);
        } else
            type->setText(it->second.first);

        auto count = item(row, COL_FURNITURE_COUNT);
        if (!count) {
            count = new QStandardItem(QString("%1").arg(it->second.second));
            count->setData(it->second.second, FURNITURE_SORT_ROLE);
            setItem(row, COL_FURNITURE_COUNT, count);
        } else {
            count->setData(it->second.second, FURNITURE_SORT_ROLE);
            count->setText(QString("%1").arg(it->second.second));
        }

        row++;
    }
}
