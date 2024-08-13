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

DiagramDataItem::DiagramDataItem(const DeviceData& data)
    : QStandardItem(data.title())
{
    setEditable(false);
    setDragEnabled(true);
    setDropEnabled(false);

    QJsonDocument doc(data.toJson());
    setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
}
