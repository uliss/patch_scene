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
#include "diagram_item_model.h"
#include "device_common.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>

using namespace ceam;

DiagramItemModel::DiagramItemModel(QObject* parent)
    : QStandardItemModel(parent)
{
}

QMimeData* DiagramItemModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* data = new QMimeData();

    for (auto& idx : indexes) {
        if (idx.column() == 0)
            data->setText(idx.data(DATA_DEVICE_DATA).toString());
    }

    return data;
}

DiagramDataItem* DiagramItemModel::deviceItem(int row, int column) const
{
    return static_cast<DiagramDataItem*>(item(row, column));
}

void DiagramItemModel::addDeviceItem(const SharedDeviceData& data)
{
    if (data)
        appendRow(new DiagramDataItem(data));
}

QList<SharedDeviceData> DiagramItemModel::allDeviceData() const
{
    QList<SharedDeviceData> res;

    for (int row = 0; row < rowCount(); row++) {
        for (int col = 0; col < columnCount(); col++) {
            auto dev = deviceItem(row, col);
            res << dev->deviceData();
        }
    }

    return res;
}

DiagramDataItem::DiagramDataItem(const SharedDeviceData& data)
{
    setEditable(false);
    setDragEnabled(true);
    setDropEnabled(false);

    setDeviceData(data);
}

SharedDeviceData DiagramDataItem::deviceData() const
{
    auto dev = SharedDeviceData(new DeviceData(DEV_NULL_ID));
    dev->setId(DEV_NULL_ID);

    auto var = data(DATA_DEVICE_DATA);
    if (var.canConvert<QByteArray>())
        dev->setJson(var.toByteArray());

    return dev;
}

void DiagramDataItem::setDeviceData(const SharedDeviceData& data)
{
    if (data) {
        setText(data->title());
        QJsonDocument doc(data->toJson());
        setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
    }
}
