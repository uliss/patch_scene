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

#include <QHeaderView>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>

FavoritesWidget::FavoritesWidget(QWidget* parent)
    : QTreeView(parent)
{
    model_ = new DiagramItemModel(this);
    setModel(model_);

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

    auto dev = new DiagramDataItem(*data);
    model_->appendRow(dev);
}

void FavoritesWidget::setFromVariant(const QList<QVariant>& items)
{
    auto parent = model_->invisibleRootItem();

    for (auto& x : items) {
        DeviceData data(DEV_NULL_ID);

        if (data.setJson(x.toJsonValue())) {
            auto dev = new DiagramDataItem(data);
            parent->appendRow(dev);
        }
    }
}

QList<QVariant> FavoritesWidget::toVariant() const
{
    QList<QVariant> items;

    for (int i = 0; i < model_->rowCount(); i++) {
        auto item = model_->item(i);
        if (!item) {
            qWarning() << __FUNCTION__ << "NULL model item";
            continue;
        }

        auto data = item->data(DATA_DEVICE_DATA);
        if (data.isNull()) {
            qWarning() << __FUNCTION__ << "empty data";
            continue;
        }

        QJsonParseError err;
        auto doc = QJsonDocument::fromJson(data.toByteArray(), &err);
        if (!doc.isObject()) {
            qWarning() << __FUNCTION__ << "json error:" << err.errorString();
            continue;
        }

        items.push_back(doc.object());
    }

    return items;
}

void FavoritesWidget::initContextMenu()
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QTreeView::customContextMenuRequested, this, [this](const QPoint& pos) {
        auto item = indexAt(pos);
        if (item.isValid()) {
            QMenu menu(this);
            auto removeAct = new QAction(tr("Remove"), this);
            connect(removeAct, &QAction::triggered, this,
                [this, item]() {
                    model_->removeRow(item.row());
                });
            menu.addAction(removeAct);
            menu.exec(mapToGlobal(pos));
        }
    });
}
