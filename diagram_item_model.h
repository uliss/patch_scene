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

#include "device_common.h"

#include <QStandardItemModel>

namespace ceam {

enum {
    DATA_DEVICE_DATA = Qt::UserRole + 1,
    DATA_DEVICE_ID,
    DATA_CONNECTION,
    DATA_DEVICE_VENDOR,
    DATA_DEVICE_MODEL,
    DATA_DEVICE_TITLE_LATIN,
};

class DiagramDataItem : public QStandardItem {
public:
    DiagramDataItem(const SharedItemData& data);
    SharedItemData deviceData() const;
    void setDeviceData(const SharedItemData& data);

    bool match(const QRegularExpression& re) const;
};

class DiagramItemModel : public QStandardItemModel {
public:
    DiagramItemModel(QObject* parent = nullptr);
    QMimeData* mimeData(const QModelIndexList& indexes) const override;

    DiagramDataItem* deviceItem(int row, int column) const;
    void addDeviceItem(const SharedItemData& data);

    QList<SharedItemData> allDeviceData() const;
};
}

#endif // DIAGRAM_ITEM_MODEL_H
