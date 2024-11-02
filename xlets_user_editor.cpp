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
#include "xlets_user_editor.h"
#include "device_xlet_view.h"
#include "ui_xlets_user_editor.h"
#include "xlets_view.h"

namespace {
constexpr int XW = 22;
constexpr int XH = 20;
}

namespace ceam {

XletsUserEditor::XletsUserEditor(QWidget* parent, const SharedDeviceData& data)
    : QDialog(parent)
    , ui(new Ui::XletsUserEditor)
    , data_(data)
{
    ui->setupUi(this);

    auto NCOLS = data->logicViewData().maxInputColumnCount();
    auto in_cols = data->inputs().size() % NCOLS;
    auto in_rows = data->inputs().size() / NCOLS;
    ui->inletsView->setScene(&in_scene_);
    ui->outletsView->setScene(&out_scene_);

    XletsLogicView view({}, inlets_);
    for (auto& x : data_->inputs()) {
        auto in = inlets_.append(x, XletType::In, nullptr);
        in_scene_.addItem(in);
    }
    view.placeXlets({});
    ui->inletsView->setFixedSize(in_scene_.itemsBoundingRect().size().toSize().grownBy({3, 3, 3, 3}));
    ui->inletsView->centerOn(in_scene_.sceneRect().center());

    auto NROWS = data->logicViewData().maxOutputColumnCount();
    XletsLogicView view2({}, outlets_);
    for (auto& x : data_->outputs()) {
        auto in = outlets_.append(x, XletType::Out, nullptr);
        out_scene_.addItem(in);
    }
    view2.placeXlets({});
    ui->outletsView->setFixedSize(out_scene_.itemsBoundingRect().size().toSize().grownBy({3, 3, 3, 3}));
    ui->outletsView->centerOn(out_scene_.sceneRect().center());

    ui->numCols->setValue(6);
    ui->numRows->setValue(3);

    ui->userView->setFixedSize(6 * XW, 3 * XH);
}

XletsUserEditor::~XletsUserEditor()
{
    delete ui;
}
}
