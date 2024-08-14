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
#ifndef DIAGRAM_ITEM_MODEL_H
#define DIAGRAM_ITEM_MODEL_H

#include <QStandardItemModel>

class DeviceData;

constexpr int DATA_DEVICE_DATA = Qt::UserRole + 1;
constexpr int DATA_DEVICE_ID = Qt::UserRole + 2;
constexpr int DATA_CONNECTION = Qt::UserRole + 3;

class DiagramItemModel : public QStandardItemModel {
public:
    DiagramItemModel(QObject* parent = nullptr);
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
};

class DiagramDataItem : public QStandardItem {
public:
    DiagramDataItem(const DeviceData& data);
};

#endif // DIAGRAM_ITEM_MODEL_H