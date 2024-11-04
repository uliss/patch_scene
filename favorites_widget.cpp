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
#include "favorites_widget.h"
#include "device_common.h"
#include "device_library.h"
#include "device_editor.h"
#include "logging.hpp"

#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>

using namespace ceam;

FavoritesWidget::FavoritesWidget(QWidget* parent)
    : QTreeView(parent)
{
    setIndentation(0);

    model_ = new DiagramItemModel(this);
    setModel(model_);

    setMinimumWidth(150);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setSortingEnabled(true);
    sortByColumn(0, Qt::AscendingOrder);
    header()->setVisible(false);

    initContextMenu();
}

void FavoritesWidget::addItem(const SharedDeviceData& data)
{
    if (data->isNull())
        return;

    model_->addDeviceItem(data);
}

void FavoritesWidget::setFromVariant(const QList<QVariant>& items)
{
    for (auto& x : items) {
        SharedDeviceData data(new DeviceData(DEV_NULL_ID));

        if (data->setJson(x.toJsonValue()))
            model_->addDeviceItem(data);
    }
}

QList<QVariant> FavoritesWidget::toVariant() const
{
    QList<QVariant> items;

    for (int i = 0; i < model_->rowCount(); i++) {
        auto item = model_->item(i);
        if (!item) {
            WARN() << "NULL model item";
            continue;
        }

        auto data = item->data(DATA_DEVICE_DATA);
        if (data.isNull()) {
            WARN() << "empty data";
            continue;
        }

        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(data.toByteArray(), &err);
        if (!doc.isObject()) {
            WARN() << "json error:" << err.errorString();
            continue;
        }

        items.push_back(doc.object());
    }

    return items;
}

bool FavoritesWidget::importElements(const QString& filename)
{
    DeviceLibrary lib;
    auto ok = lib.readFile(filename);
    if (!ok)
        return false;

    for (auto& x : lib.devices())
        model_->addDeviceItem(x);

    for (auto& x : lib.instruments())
        model_->addDeviceItem(x);

    for (auto& x : lib.humans())
        model_->addDeviceItem(x);

    for (auto& x : lib.returns())
        model_->addDeviceItem(x);

    for (auto& x : lib.sends())
        model_->addDeviceItem(x);

    return true;
}

void FavoritesWidget::initContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pos) {
        auto item_idx = indexAt(pos);

        QMenu menu(this);

        auto exportToLib = new QAction(tr("Export to library file"), this);
        connect(exportToLib, &QAction::triggered, this,
            [this, item_idx]() { emit requestExportAll(model_->allDeviceData()); });
        auto exportFromLib = new QAction(tr("Import from library file"), this);
        connect(exportFromLib, &QAction::triggered, this,
            [this, item_idx]() { emit requestImportAll(); });

        if (item_idx.isValid()) {

            auto editAct = new QAction(tr("Edit"), this);
            connect(editAct, &QAction::triggered, this,
                [this, item_idx]() {
                    auto item_data = item_idx.data(DATA_DEVICE_DATA).toByteArray();
                    if (!item_data.isNull()) {
                        SharedDeviceData dev_data(new DeviceData(DEV_NULL_ID));

                        if (dev_data->setJson(item_data)) {
                            auto dialog = new DeviceEditor(dev_data, this);
                            connect(dialog, &DeviceEditor::acceptData, this,
                                [this, item_idx](const SharedDeviceData& data) {
                                    auto item = model_->deviceItem(item_idx.row(), item_idx.column());
                                    if (item)
                                        item->setDeviceData(data);
                                });
                            dialog->exec();
                        }
                    }
                });
            menu.addAction(editAct);

            auto removeAct = new QAction(tr("Remove"), this);
            connect(removeAct, &QAction::triggered, this,
                [this, item_idx]() { model_->removeRow(item_idx.row()); });
            menu.addAction(removeAct);

            auto exportAct = new QAction(tr("Export"), this);
            connect(exportAct, &QAction::triggered, this,
                [this, item_idx]() {
                    auto dev = model_->deviceItem(item_idx.row(), item_idx.column());
                    if (dev)
                        emit requestItemExport(dev->deviceData());
                });
            menu.addAction(exportAct);
            menu.addSeparator();
        }

        menu.addAction(exportToLib);
        menu.addAction(exportFromLib);
        menu.exec(mapToGlobal(pos));
    });
}
