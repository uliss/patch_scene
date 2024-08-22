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
#include "connection_item_model.h"
#include "diagram_item_model.h"

namespace {
enum ConnColumnOrder {
    COL_CONN_SRC_NAME = 0,
    COL_CONN_SRC_MODEL,
    COL_CONN_SRC_PLUG,
    COL_CONN_DEST_NAME,
    COL_CONN_DEST_MODEL,
    COL_CONN_DEST_PLUG
};
constexpr int DATA_CONN_NCOLS = 6;

}

using namespace ceam;

ConnectionItemModel::ConnectionItemModel(QObject* parent)
    : QStandardItemModel(0, DATA_CONN_NCOLS, parent)
{
    setHorizontalHeaderLabels({ tr("Source"), tr("Model"), tr("Plug"), tr("Destination"), tr("Model"), tr("Plug") });

    proxy_ = new QSortFilterProxyModel(this);
    proxy_->setDynamicSortFilter(true);
    proxy_->setSourceModel(this);
}

bool ConnectionItemModel::updateDeviceTitle(DeviceId id, const QString& title)
{
    int update_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        if (updateDeviceTitle(index(i, COL_CONN_SRC_NAME), id, title)) {
            update_num++;
            continue;
        }

        if (updateDeviceTitle(index(i, COL_CONN_DEST_NAME), id, title)) {
            update_num++;
            continue;
        }
    }

    return update_num > 0;
}

bool ConnectionItemModel::addConnection(const ConnectionData& data,
    const SharedDeviceData& src,
    const XletData& src_out,
    const SharedDeviceData& dest,
    const XletData& dest_in)
{
    if (!src || !dest)
        return false;

    auto src_name = new QStandardItem(src->title());
    src_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
    src_name->setData(data.source(), DATA_DEVICE_ID);
    src_name->setEditable(false);
    auto src_model = new QStandardItem(src_out.modelString());
    src_model->setEditable(false);
    auto src_plug = new QStandardItem(src_out.typeString());
    src_plug->setEditable(false);

    auto dest_name = new QStandardItem(dest->title());
    dest_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
    dest_name->setData(data.destination(), DATA_DEVICE_ID);
    dest_name->setEditable(false);
    auto dest_model = new QStandardItem(dest_in.modelString());
    dest_model->setEditable(false);
    auto dest_plug = new QStandardItem(dest_in.typeString());
    dest_plug->setEditable(false);

    appendRow({ src_name, src_model, src_plug, dest_name, dest_model, dest_plug });

    return true;
}

bool ConnectionItemModel::removeConnection(const ConnectionData& data)
{
    int remove_num = 0;

    for (int i = 0; i < rowCount(); i++) {
        auto conn = item(i, COL_CONN_SRC_NAME);
        if (conn && conn->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            removeRow(i);
            remove_num++;
        }
    }

    return remove_num > 0;
}

std::optional<DeviceId> ConnectionItemModel::deviceId(const QModelIndex& idx) const
{
    if (idx.column() == COL_CONN_SRC_NAME || idx.column() == COL_CONN_DEST_NAME) {
        bool ok = false;
        auto id = idx.data(DATA_DEVICE_ID).toInt(&ok);
        if (ok)
            return id;
    }

    return {};
}

void ConnectionItemModel::clearItems()
{
    removeRows(0, rowCount());
}

bool ConnectionItemModel::updateDeviceTitle(const QModelIndex& idx, DeviceId id, const QString& title)
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
