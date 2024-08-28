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
#include "battery_item_model.h"

namespace {
constexpr int COL_BATTERY_TYPE = 0;
constexpr int COL_BATTERY_COUNT = 1;
}

using namespace ceam;

BatteryItemModel::BatteryItemModel(QObject* parent)
    : QStandardItemModel { parent }
{
    setHorizontalHeaderLabels({ tr("Type"), tr("Count") });

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setDynamicSortFilter(true);
    proxy_->setSourceModel(this);
}

bool BatteryItemModel::addDeviceData(const SharedDeviceData& data)
{
    if (!data || data->batteryCount() == 0)
        return false;

    batteries_[data->batteryType()] += data->batteryCount();
    updateData();

    return true;
}

void BatteryItemModel::removeDeviceData(const SharedDeviceData& data)
{
    if (!data || data->batteryCount() == 0)
        return;

    auto type = data->batteryType();
    batteries_[type] -= data->batteryCount();
    if (batteries_[type] <= 0)
        batteries_.erase(type);

    updateData();
}

void BatteryItemModel::updateDeviceData(const BatteryChange& data)
{
    if (data) {
        int updates = 0;

        if (data.typeA() != BatteryType::None) {
            batteries_[data.typeA()] -= data.typeADelta();
            updates++;
        }

        if (data.typeB() != BatteryType::None) {
            batteries_[data.typeB()] += data.typeBDelta();
            updates++;
        }

        if (updates > 0)
            updateData();
    }
}

void BatteryItemModel::clearItems()
{
    batteries_.clear();
    removeRows(0, rowCount());
}

void BatteryItemModel::setFullData(const QList<SharedDeviceData>& data)
{
    beginResetModel();

    {
        QSignalBlocker sb(this);

        clearItems();

        for (auto& x : data)
            addDeviceData(x);
    }

    endResetModel();
}

void BatteryItemModel::updateData()
{
    setRowCount(batteries_.size());

    int row = 0;
    for (auto& kv : batteries_) {
        auto type = item(row, COL_BATTERY_TYPE);
        if (!type) {
            type = new QStandardItem(toString(kv.first));
            setItem(row, COL_BATTERY_TYPE, type);
        } else
            type->setText(toString(kv.first));

        auto count = item(row, COL_BATTERY_COUNT);
        if (!count) {
            count = new QStandardItem(QString("%1").arg(kv.second));
            count->setData(kv.second);
            setItem(row, COL_BATTERY_COUNT, count);
        } else {
            count->setData(kv.second);
            count->setText(QString("%1").arg(kv.second));
        }

        row++;
    }
}
