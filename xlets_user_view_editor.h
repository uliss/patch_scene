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
#ifndef XLETS_USER_VIEW_EDITOR_H
#define XLETS_USER_VIEW_EDITOR_H

#include "device_common.h"
#include "device_xlet_view.h"

#include <QDialog>
#include <QGraphicsScene>

namespace Ui {
class XletsUserEditor;
}

namespace ceam {

class XletsUserScene : public QGraphicsScene {
    Q_OBJECT

public:
    XletsUserScene(const QList<XletData>& inlets, const QList<XletData>& outlets);

    const XletsUserViewData& data() const { return data_; }
    void setData(int currentIndex, const XletsUserViewData& data);

    int currentIndex() const { return idx_; }

    void setSize(int rows, int cols);
    void setRows(int v);
    void setCols(int v);

    // update xlet positions
    void placeXlets();

    // clear xlets and create them again
    void reinitXlets();

    void clearAll();

Q_SIGNALS:
    void updated();

private:
    /**
     * remove all cells from the scene and the cell list
     */
    void clearCells();

private:
    QList<QGraphicsItem*> cells_;
    XletsUserViewData data_;
    DeviceXlets xlets_;
    const QList<XletData>&inlets_, &outlets_;
    int idx_ { -1 };
};

class XletsUserViewCell : public QGraphicsRectItem {
    CellIndex cell_;
    XletsUserViewData& data_;
    XletsUserScene* scene_;

public:
    XletsUserViewCell(const QRect& r, const CellIndex& idx, XletsUserViewData& data, XletsUserScene* parent);
    void dragEnterEvent(QGraphicsSceneDragDropEvent* event) final;
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* event) final;
    void dropEvent(QGraphicsSceneDragDropEvent* event) final;

private:
    void resetCell();
    void hoverCell();
};

class XletsUserViewEditor : public QDialog {
    Q_OBJECT

public:
    explicit XletsUserViewEditor(QWidget* parent, const SharedDeviceData& data);
    ~XletsUserViewEditor();

    void setXletViewData(int idx, const XletsUserViewData& data);

public Q_SLOTS:
    void accept() override;

Q_SIGNALS:
    void acceptData(SharedDeviceData);

private:
    void initInlets();
    void initOutlets();
    void initButtons(const SharedDeviceData& data);
    void initUserViewList(const SharedDeviceData& data);
    void initUserViewDataWith(int idx);
    void initUserViewDataWith(const QString& viewName);
    void initRowsAndCols();

    void adjustUserViewSize();

    void enableUserView(bool value);

private:
    Ui::XletsUserEditor* ui;
    SharedDeviceData data_;
    DeviceXlets inlets_, outlets_;
    QGraphicsScene in_scene_, out_scene_;
    XletsUserScene view_scene_;
};

}

#endif // XLETS_USER_VIEW_EDITOR_H
