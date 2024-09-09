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
#include "library_item_model.h"
#include "device_library.h"

#include <QJsonDocument>

namespace ceam {

LibraryItemModel::LibraryItemModel(QObject* parent)
    : QSortFilterProxyModel { parent }
{
    model_ = new DiagramItemModel(this);

    setSourceModel(model_);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setAutoAcceptChildRows(true);
    setRecursiveFilteringEnabled(true);
    setSortLocaleAware(true);
}

void LibraryItemModel::readFile(const QString& file)
{
    DeviceLibrary dev_lib;
    if (!dev_lib.readFile(file))
        return;

    auto parentItem = model_->invisibleRootItem();

    auto devices = new QStandardItem(tr("devices"));
    devices->setEditable(false);
    parentItem->appendRow(devices);
    loadSection(devices, dev_lib.devices());

    auto instr = new QStandardItem({ tr("instruments") });
    instr->setEditable(false);
    parentItem->appendRow(instr);
    loadSection(instr, dev_lib.instruments());

    auto sends = new QStandardItem({ tr("sends") });
    sends->setEditable(false);
    parentItem->appendRow(sends);
    loadSection(sends, dev_lib.sends());

    auto returns = new QStandardItem({ tr("returns") });
    returns->setEditable(false);
    parentItem->appendRow(returns);
    loadSection(returns, dev_lib.returns());

    auto furniture = new QStandardItem({ tr("furniture") });
    furniture->setEditable(false);
    parentItem->appendRow(furniture);
    loadSection(furniture, dev_lib.furniture());

    auto humans = new QStandardItem({ tr("humans") });
    humans->setEditable(false);
    parentItem->appendRow(humans);
    loadSection(humans, dev_lib.humans());
}

void LibraryItemModel::loadSection(QStandardItem* parent, const QList<SharedDeviceData>& data)
{
    for (auto& x : data) {
        auto item = new QStandardItem(x->title());
        item->setEditable(false);
        item->setToolTip(x->title());

        QJsonDocument doc(x->toJson());
        item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
        item->setDragEnabled(true);
        item->setDropEnabled(false);
        parent->appendRow(item);
    }
}

} // namespace ceam
