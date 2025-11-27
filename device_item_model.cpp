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
#include "device_item_model.h"
#include "diagram_item_model.h"
#include "logging.hpp"

namespace {
enum DeviceColumnOrder {
    COL_DEV_TITLE = 0,
    COL_DEV_VENDOR,
    COL_DEV_MODEL,
};
constexpr int DATA_DEV_NCOLS = 3;
}

using namespace ceam;

QList<QString> DeviceItemModel::headerLabels()
{
    return { tr("Name"), tr("Vendor"), tr("Model") };
}

DeviceItemModel::DeviceItemModel(QObject* parent)
    : QStandardItemModel(0, DATA_DEV_NCOLS, parent)
{
    setHorizontalHeaderLabels(headerLabels());

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setDynamicSortFilter(true);
    proxy_->setSourceModel(this);
}

bool DeviceItemModel::addDevice(const SharedItemData& data)
{
    if (!data || !data->showInDeviceCategory())
        return false;

    auto title = new QStandardItem(data->title());
    title->setData(data->id(), DATA_DEVICE_ID);
    title->setEditable(true);

    auto vendor = new QStandardItem(data->vendor());
    vendor->setData(data->id(), DATA_DEVICE_ID);
    vendor->setEditable(true);

    auto model = new QStandardItem(data->model());
    model->setData(data->id(), DATA_DEVICE_ID);
    model->setEditable(true);

    appendRow({ title, vendor, model });
    return true;
}

bool DeviceItemModel::removeDevice(const SharedItemData& data)
{
    if (!data || data->category() != ItemCategory::Device)
        return false;

    for (int i = 0; i < rowCount(); i++) {
        auto dev = item(i, COL_DEV_TITLE);
        if (dev && dev->data(DATA_DEVICE_ID) == data->id()) {
            removeRow(i);
            return true;
        }
    }

    return false;
}

void DeviceItemModel::clearItems()
{
    removeRows(0, rowCount());
}

void DeviceItemModel::setDeviceData(const QList<SharedItemData>& data)
{
    beginResetModel();

    {
        QSignalBlocker sb(this);

        removeRows(0, rowCount());

        for (auto& dev_data : data)
            addDevice(dev_data);
    }

    endResetModel();
}

QStandardItem* DeviceItemModel::deviceTitle(int idx)
{
    return item(idx, COL_DEV_TITLE);
}

QStandardItem* DeviceItemModel::deviceVendor(int idx)
{
    return item(idx, COL_DEV_VENDOR);
}

QStandardItem* DeviceItemModel::deviceModel(int idx)
{
    return item(idx, COL_DEV_MODEL);
}

std::optional<SceneItemId> DeviceItemModel::deviceId(const QStandardItem* item) const
{
    if (!item)
        return {};

    bool ok = false;
    auto id = item->data(DATA_DEVICE_ID).toInt(&ok);
    if (ok)
        return id;
    else
        WARN() << "id property not found";

    return {};
}

std::optional<SceneItemId> DeviceItemModel::deviceId(int idx) const
{
    return deviceId(item(idx, COL_DEV_TITLE));
}

SharedItemData DeviceItemModel::updateDeviceData(const QStandardItem* item, const SharedItemData& data)
{
    if (!item)
        return data;

    auto new_data = data;
    switch (item->column()) {
    case COL_DEV_TITLE:
        new_data->setTitle(item->text());
        break;
    case COL_DEV_VENDOR:
        new_data->setVendor(item->text());
        break;
    case COL_DEV_MODEL:
        new_data->setModel(item->text());
        break;
    default:
        WARN() << "unknown column:" << item->column();
        return data;
    }

    return new_data;
}
