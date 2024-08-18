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
#include "send_item_model.h"
#include "diagram_item_model.h"

namespace {
enum SendColumnOrder {
    COL_SEND_NAME = 0,
    COL_SEND_INPUT,
    COL_SEND_SRC_NAME,
    COL_SEND_SRC_OUTPUT
};
constexpr int DATA_SEND_NCOLS = 4;
constexpr int SEND_SORT_ROLE = Qt::UserRole + 1;

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
            if (a.column() == COL_SEND_INPUT || a.column() == COL_SEND_SRC_OUTPUT)
                return a.data(SEND_SORT_ROLE).toInt() < b.data(SEND_SORT_ROLE).toInt();

        return QSortFilterProxyModel::lessThan(a, b);
    }
};
}

SendItemModel::SendItemModel(QObject* parent)
    : QStandardItemModel(0, DATA_SEND_NCOLS, parent)
{
    setHorizontalHeaderLabels({ tr("Send"), tr("Input"), tr("Device"), tr("Output") });

    proxy_ = new SortProxy(this);
    proxy_->setSourceModel(this);
}

bool SendItemModel::addConnection(const ConnectionData& data,
    const SharedDeviceData& src,
    const SharedDeviceData& dest)
{
    if (!src || !dest || dest->category() != ItemCategory::Send)
        return false;

    auto src_name = new QStandardItem(src->title());
    src_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
    src_name->setData(data.src, DATA_DEVICE_ID);
    src_name->setEditable(false);

    auto dest_name = new QStandardItem(dest->title());
    dest_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
    dest_name->setData(data.dest, DATA_DEVICE_ID);
    dest_name->setEditable(false);

    auto src_idx = new QStandardItem(QString("%1").arg((int)data.out + 1));
    src_idx->setEditable(false);
    src_idx->setData(data.out, SEND_SORT_ROLE);
    auto dest_idx = new QStandardItem(QString("%1").arg((int)data.in + 1));
    dest_idx->setData(data.in, SEND_SORT_ROLE);
    dest_idx->setEditable(false);

    appendRow({ dest_name, dest_idx, src_name, src_idx });

    return true;
}

bool SendItemModel::removeConnection(const ConnectionData& data)
{
    int remove_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        auto conn = item(i, COL_SEND_SRC_NAME);
        if (conn && conn->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            removeRow(i);
            remove_num++;
        }
    }

    return remove_num > 0;
}

bool SendItemModel::updateDeviceTitle(DeviceId id, const QString& title)
{
    int update_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        if (updateDeviceTitle(index(i, COL_SEND_SRC_NAME), id, title)) {
            update_num++;
            continue;
        }

        if (updateDeviceTitle(index(i, COL_SEND_NAME), id, title)) {
            update_num++;
            continue;
        }
    }

    return update_num > 0;
}

void SendItemModel::clearItems()
{
    removeRows(0, rowCount());
}

std::optional<DeviceId> SendItemModel::deviceId(const QModelIndex& idx) const
{
    if (idx.column() == COL_SEND_NAME || idx.column() == COL_SEND_SRC_NAME) {
        bool ok = false;
        auto id = idx.data(DATA_DEVICE_ID).toInt(&ok);
        if (ok)
            return id;
    }

    return {};
}

bool SendItemModel::updateDeviceTitle(const QModelIndex& idx, DeviceId id, const QString& title)
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