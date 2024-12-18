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
#include "return_item_model.h"
#include "diagram_item_model.h"

namespace {
enum ReturnColumnOrder {
    COL_RETURN_NAME = 0,
    COL_RETURN_OUTPUT,
    COL_RETURN_DEST_NAME,
    COL_RETURN_DEST_INPUT,
};
constexpr int DATA_RETURN_NCOLS = 4;
constexpr int RETURN_SORT_ROLE = Qt::UserRole + 1;

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
            if (a.column() == COL_RETURN_OUTPUT || a.column() == COL_RETURN_DEST_INPUT)
                return a.data(RETURN_SORT_ROLE).toInt() < b.data(RETURN_SORT_ROLE).toInt();

        return QSortFilterProxyModel::lessThan(a, b);
    }
};
}

using namespace ceam;

ReturnItemModel::ReturnItemModel(QObject* parent)
    : QStandardItemModel { 0, DATA_RETURN_NCOLS, parent }
{
    setHorizontalHeaderLabels({ tr("Return"), tr("Output"), tr("Device"), tr("Input") });

    proxy_ = new SortProxy(this);
    proxy_->setDynamicSortFilter(true);
    proxy_->setSourceModel(this);
}

bool ReturnItemModel::addConnection(const DeviceConnectionData& info)
{
    if (!info.isValid() || info.src_data->category() != ItemCategory::Return)
        return false;

    auto src_name = new QStandardItem(info.src_data->title());
    src_name->setData(QVariant::fromValue(info.connectionId()), DATA_CONNECTION);
    src_name->setData(info.src_data->id(), DATA_DEVICE_ID);
    src_name->setEditable(false);

    auto dest_name = new QStandardItem(info.dest_data->title());
    dest_name->setData(QVariant::fromValue(info.connectionId()), DATA_CONNECTION);
    dest_name->setData(info.dest_data->id(), DATA_DEVICE_ID);
    dest_name->setEditable(false);

    auto src_idx = new QStandardItem(QString("%1").arg(info.src_out_idx + 1));
    src_idx->setData(info.src_out_idx, RETURN_SORT_ROLE);
    src_idx->setEditable(false);
    auto dest_idx = new QStandardItem(QString("%1").arg(info.dest_in_idx + 1));
    dest_idx->setData(info.dest_in_idx, RETURN_SORT_ROLE);
    dest_idx->setEditable(false);

    appendRow({ src_name, src_idx, dest_name, dest_idx });

    return true;
}

bool ReturnItemModel::removeConnection(const ConnectionId& data)
{
    int remove_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        auto conn = item(i, COL_RETURN_DEST_NAME);
        if (conn && conn->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            removeRow(i);
            remove_num++;
        }
    }

    return remove_num > 0;
}

bool ReturnItemModel::updateDeviceTitle(DeviceId id, const QString& title)
{
    int update_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        if (updateDeviceTitle(index(i, COL_RETURN_DEST_NAME), id, title)) {
            update_num++;
            continue;
        }

        if (updateDeviceTitle(index(i, COL_RETURN_NAME), id, title)) {
            update_num++;
            continue;
        }
    }

    return update_num > 0;
}

void ReturnItemModel::clearItems()
{
    removeRows(0, rowCount());
}

std::optional<DeviceId> ReturnItemModel::deviceId(const QModelIndex& idx) const
{
    if (idx.column() == COL_RETURN_NAME || idx.column() == COL_RETURN_DEST_NAME) {
        bool ok = false;
        auto id = idx.data(DATA_DEVICE_ID).toInt(&ok);
        if (ok)
            return id;
    }

    return {};
}

void ReturnItemModel::setFullData(const QList<DeviceConnectionData>& info)
{
    beginResetModel();

    {
        QSignalBlocker sb(this);

        removeRows(0, rowCount());
        for (auto& i : info)
            addConnection(i);
    }

    endResetModel();
}

bool ReturnItemModel::updateDeviceTitle(const QModelIndex& idx, DeviceId id, const QString& title)
{
    if (!idx.isValid())
        return false;

    auto dev_id = deviceId(idx);
    if (dev_id && dev_id.value() == id) {
        auto cell = itemFromIndex(idx);
        if (cell) {
            cell->setText(title);
            return true;
        }
    }

    return false;
}
