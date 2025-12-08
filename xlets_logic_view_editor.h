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
#ifndef XLETS_LOGIC_VIEW_EDITOR_H
#define XLETS_LOGIC_VIEW_EDITOR_H

#include "device_xlet.h"
#include "xlets_logic_view_data.h"

#include <QDialog>

namespace Ui {
class XletLogicalEditor;
}

namespace ceam {

class XletLogicalEditor : public QDialog {
    Q_OBJECT

public:
    explicit XletLogicalEditor(QWidget* parent, QList<XletData>& data, XletsLogicViewData& viewData, XletType type);
    ~XletLogicalEditor();

    void setupTable(size_t rows);

private:
    Ui::XletLogicalEditor* ui;
    QList<XletData>& data_;
    XletsLogicViewData& view_data_;
    XletType type_;

private:
    void fillTable();

    bool duplicateXlet(int row);
    bool getXletData(int row, XletData& data) const;
    bool moveXlet(int row, bool up);
    bool removeXlet(int row);
    bool selectXletRow(int row);
    void insertXlet(int row, const XletData& data, bool resize = true);
    void setupXlets();
    void syncXlets();
    void updateButtonState(int currentRow);
};

} // namespace ceam

#endif // XLETS_LOGIC_VIEW_EDITOR_H
