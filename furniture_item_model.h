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
#ifndef FURNITURE_ITEM_MODEL_H
#define FURNITURE_ITEM_MODEL_H

#include "device_common.h"

#include <QSortFilterProxyModel>
#include <QStandardItemModel>

namespace ceam {

class FurnitureItemModel : public QStandardItemModel {
    Q_OBJECT
public:
    explicit FurnitureItemModel(QObject* parent = nullptr);

    bool addFurniture(const SharedDeviceData& data);
    bool removeFurniture(const SharedDeviceData& data);

    QSortFilterProxyModel* proxyModel() { return proxy_; }

private:
    QMap<size_t, std::pair<QString, int>> furniture_map_;
    QSortFilterProxyModel* proxy_;

private:
    void updateData();
};

}

#endif // FURNITURE_ITEM_MODEL_H
