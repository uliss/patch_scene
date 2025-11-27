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
#include "xlets_logic_view_editor.h"
#include "device_common.h"
#include "logging.hpp"
#include "psc_utils.h"
#include "table_cell_power.h"
#include "table_cell_socket.h"
#include "tablecellcheckbox.h"
#include "tablecellconnector.h"
#include "ui_xlets_logic_view_editor.h"

#include <QHeaderView>
#include <QTableWidget>

namespace {
enum {
    COL_CONNECTOR,
    COL_NAME,
    COL_CONNECTOR_TYPE,
    COL_POWER_TYPE,
    COL_PHANTOM,
    COL_BIDIRECT,
    COL_MAX,
};
}

using namespace ceam;

XletLogicalEditor::XletLogicalEditor(QWidget* parent, QList<XletData>& data, XletsLogicViewData& viewData, XletType type)
    : QDialog(parent)
    , ui(new Ui::XletLogicalEditor)
    , data_(data)
    , view_data_(viewData)
    , type_(type)
{
    ui->setupUi(this);

    ui->addXlet->setEnabled(true);
    ui->moveXletDown->setEnabled(false);
    ui->moveXletUp->setEnabled(false);
    ui->removeXlet->setEnabled(false);

    connect(ui->closeButton, &QPushButton::clicked, this, [this]() {
        close();
    });

    ui->maxColumns->setRange(ItemData::MIN_COL_COUNT, ItemData::MAX_COL_COUNT);

    switch (type_) {
    case XletType::In:
        ui->maxColumns->setValue(view_data_.maxInputColumnCount());
        break;
    case XletType::Out:
        ui->maxColumns->setValue(view_data_.maxOutputColumnCount());
        break;
    default:
        break;
    }

    connect(ui->maxColumns, &QSpinBox::valueChanged, this, [this](int value) {
        switch (type_) {
        case XletType::In:
            view_data_.setMaxInputColumnCount(value);
            break;
        case XletType::Out:
            view_data_.setMaxOutputColumnCount(value);
            break;
        default:
            break;
        }
    });

    connect(this, &XletLogicalEditor::finished, this, [this]() {
        syncXlets();
    });

    setupTable(data_.size());
    setupXlets();
    fillTable();
}

XletLogicalEditor::~XletLogicalEditor()
{
    delete ui;
}

void XletLogicalEditor::setupTable(size_t rows)
{
    auto header = ui->tableWidget->horizontalHeader();
    if (header) {
        header->setMinimumSectionSize(70);
    }

    ui->tableWidget->setRowCount(rows);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QTableWidget::ContiguousSelection);
    ui->tableWidget->setHorizontalHeaderLabels({ tr("Type"), tr("Name"), tr("Socket"), tr("Power"), tr("Phantom"), tr("Bidirect") });
    ui->tableWidget->setColumnWidth(COL_CONNECTOR, 100);
    ui->tableWidget->setColumnWidth(COL_NAME, 100);
    // tab->setColumnWidth(COL_VISIBLE, 60);
    // tab->setColumnWidth(COL_SOCKET, 60);
    ui->tableWidget->setColumnWidth(COL_PHANTOM, 50);
    // adjust minimal height
    if (rows < 6)
        ui->tableWidget->setMinimumHeight(rows * 30);
}

void XletLogicalEditor::fillTable()
{
    int xlet_idx = 0;
    for (auto& x : data_) {
        insertXlet(xlet_idx++, x, false);
    }
    ui->tableWidget->resizeColumnsToContents();
}

void XletLogicalEditor::insertXlet(int row, const XletData& data, bool resize)
{
    // xlet name
    auto name = new QTableWidgetItem();
    name->setText(data.name());
    ui->tableWidget->setItem(row, COL_NAME, name);

    // model choose
    auto model = new TableCellConnector(ui->tableWidget);
    model->setConnectorModel(data.connectorModel());
    ui->tableWidget->setCellWidget(row, COL_CONNECTOR, model);

    // socket
    auto socket = new TableCellConnectorType(data.connectorType(), this);
    ui->tableWidget->setCellWidget(row, COL_CONNECTOR_TYPE, socket);

    // power
    auto power = new TableCellPower(data.powerType(), this);
    ui->tableWidget->setCellWidget(row, COL_POWER_TYPE, power);

    // phantom power
    auto phantom = new TableCellCheckBox(data.isPhantomOn());
    phantom->setEnabled(data.supportsPhantomPower());
    ui->tableWidget->setCellWidget(row, COL_PHANTOM, phantom);

    // bidirect
    auto bidi = new TableCellCheckBox(data.isBidirect());
    ui->tableWidget->setCellWidget(row, COL_BIDIRECT, bidi);

    connect(power, &TableCellConnector::currentIndexChanged, this, [this, row, phantom](int idx) {
        auto power = qobject_cast<TableCellPower*>(ui->tableWidget->cellWidget(row, COL_POWER_TYPE));
        if (power)
            phantom->setEnabled(power->powerType() == PowerType::Phantom);
    });

    auto nrows = std::max<int>(2, ui->tableWidget->rowCount());
    // adjust minimal height
    if (nrows < 6)
        ui->tableWidget->setMinimumHeight((nrows + 1) * 30);

    if (resize)
        ui->tableWidget->resizeColumnsToContents();
}

void XletLogicalEditor::setupXlets()
{
    connect(ui->addXlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->tableWidget->currentRow();
        if (row >= 0) {
            if (duplicateXlet(row))
                selectXletRow(row + 1);
        } else { // no selection
            auto nrows = ui->tableWidget->rowCount();
            if (nrows > 0) {
                if (duplicateXlet(nrows - 1))
                    selectXletRow(nrows);
            } else {
                ui->tableWidget->setRowCount(1);
                XletData data { {}, ConnectorModel::XLR };
                data.setName(XletData::defaultName(type_, 1));
                insertXlet(0, data);
                ui->tableWidget->selectRow(0);
            }
        }
    });
    connect(ui->removeXlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->tableWidget->currentRow();
        if (row >= 0) {
            if (removeXlet(row))
                selectXletRow(row);
        } else { // no selection
            auto nrows = ui->tableWidget->rowCount();
            if (nrows > 0) { // remove last inlet
                if (removeXlet(nrows - 1))
                    selectXletRow(nrows - 2);
            }
        }

        // disable inlet button
        ui->removeXlet->setEnabled(ui->tableWidget->rowCount() > 0);
    });
    connect(ui->moveXletUp, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->tableWidget->currentRow(), true);
    });
    connect(ui->moveXletDown, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->tableWidget->currentRow(), false);
    });

    connect(ui->tableWidget, &QTableWidget::currentCellChanged, this,
        [this](int curRow, int, int, int) {
            updateButtonState(curRow);
        });

    ui->btnContainer->setAlignment(ui->addXlet, Qt::AlignLeft);
    ui->btnContainer->setAlignment(ui->removeXlet, Qt::AlignLeft);
    ui->btnContainer->addStretch(10);
}

void XletLogicalEditor::syncXlets()
{
    data_.clear();
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        XletData data;
        if (getXletData(i, data))
            data_.push_back(data);
    }
}

void XletLogicalEditor::updateButtonState(int currentRow)
{
    ui->removeXlet->setEnabled(true);
    ui->moveXletDown->setEnabled(currentRow + 1 != ui->tableWidget->rowCount());
    ui->moveXletUp->setEnabled(currentRow != 0);
}

bool XletLogicalEditor::duplicateXlet(int row)
{
    XletData data;
    if (getXletData(row, data)) {
        if (data.name().isEmpty()) {
            data.setName(XletData::defaultName(type_, 1));
        } else {
            data.setName(utils::incrementString(data.name()));
        }

        ui->tableWidget->insertRow(row + 1);
        insertXlet(row + 1, data);
        return true;
    }

    return false;
}

bool XletLogicalEditor::moveXlet(int row, bool up)
{
    if (up) {
        if (row > 0 && row < ui->tableWidget->rowCount()) {
            XletData prev_data;
            if (getXletData(row - 1, prev_data)) {
                ui->tableWidget->removeRow(row - 1);
                ui->tableWidget->insertRow(row);
                insertXlet(row, prev_data, false);
            }

            selectXletRow(row - 1);
            return true;
        } else
            return false;
    } else {
        if (row >= 0 && (row + 1) < ui->tableWidget->rowCount()) {
            XletData next_data;
            if (getXletData(row + 1, next_data)) {
                ui->tableWidget->removeRow(row + 1);
                ui->tableWidget->insertRow(row);
                insertXlet(row, next_data, false);
            }

            selectXletRow(row + 1);
            return true;

        } else
            return false;
    }
}

bool XletLogicalEditor::getXletData(int row, XletData& data) const
{
    if (row < 0 || row >= ui->tableWidget->rowCount())
        return false;

    data.setName(ui->tableWidget->item(row, COL_NAME)->text());

    auto model = qobject_cast<TableCellConnector*>(ui->tableWidget->cellWidget(row, COL_CONNECTOR));
    if (model)
        data.setConnectorModel(model->connectorModel());

    auto power = qobject_cast<TableCellPower*>(ui->tableWidget->cellWidget(row, COL_POWER_TYPE));
    if (power)
        data.setPowerType(power->powerType());

    auto phantom = qobject_cast<TableCellCheckBox*>(ui->tableWidget->cellWidget(row, COL_PHANTOM));
    if (phantom && data.supportsPhantomPower())
        data.setPhantom(phantom->isChecked());

    auto socket_type = qobject_cast<TableCellConnectorType*>(ui->tableWidget->cellWidget(row, COL_CONNECTOR_TYPE));
    if (socket_type)
        data.setConnectorType(socket_type->connectorType());

    auto bidirect = qobject_cast<TableCellCheckBox*>(ui->tableWidget->cellWidget(row, COL_BIDIRECT));
    if (bidirect)
        data.setBidirect(bidirect->isChecked());

    return true;
}

bool XletLogicalEditor::removeXlet(int row)
{
    if (row < 0 || row >= ui->tableWidget->rowCount()) {
        WARN() << "invalid row" << row;
        return false;
    }

    ui->tableWidget->removeRow(row);
    return true;
}

bool XletLogicalEditor::selectXletRow(int row)
{
    if (!ui->tableWidget || row < 0 || row >= ui->tableWidget->rowCount())
        return false;

    auto model = ui->tableWidget->model();
    if (!model)
        return false;

    ui->tableWidget->selectRow(row);
    auto idx = model->index(row, 0);
    ui->tableWidget->scrollTo(idx);
    emit ui->tableWidget->currentCellChanged(row, 0, -1, -1);
    return true;
}
