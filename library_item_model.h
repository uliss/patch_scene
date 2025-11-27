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
#ifndef LIBRARY_ITEM_MODEL_H
#define LIBRARY_ITEM_MODEL_H

#include "diagram_item_model.h"

#include <QSortFilterProxyModel>

namespace ceam {

class LibraryItemModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit LibraryItemModel(QObject* parent = nullptr);

    void readFile(const QString& file);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const final;

private:
    void loadSection(QStandardItem* parent, const QList<SharedItemData>& data);

private:
    DiagramItemModel* model_;
};

} // namespace ceam

#endif // LIBRARY_ITEM_MODEL_H
