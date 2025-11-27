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
#ifndef FAVORITES_WIDGET_H
#define FAVORITES_WIDGET_H

#include "device_common.h"
#include "diagram_item_model.h"

#include <QTreeView>

namespace ceam {

class FavoritesWidget : public QTreeView {
    Q_OBJECT
public:
    explicit FavoritesWidget(QWidget* parent = nullptr);

    void addItem(const SharedItemData& data);

    void setFromVariant(const QList<QVariant>& items);
    QList<QVariant> toVariant() const;

    bool importElements(const QString& filename);

signals:
    void requestExportAll(const QList<SharedItemData>& data);
    void requestImportAll();
    void requestItemExport(const SharedItemData& data);

private:
    DiagramItemModel* model_ { nullptr };
    void initContextMenu();
};
} // namespace ceam

#endif // FAVORITES_WIDGET_H
