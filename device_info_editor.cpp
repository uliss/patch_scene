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
#include "device_info_editor.h"
#include "ui_device_info_editor.h"

namespace {
constexpr int COLUMN_NAME = 0;
constexpr int COLUMN_VALUE = 1;
}

using namespace ceam;

DeviceInfoEditor::DeviceInfoEditor(const ItemDataInfo& data, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DeviceInfoEditor)
    , data_(data)
{
    ui->setupUi(this);
    ui->infoTable->setHorizontalHeaderLabels({ tr("Name"), tr("Value") });

    for (auto& kv : data_) {
        auto rc = ui->infoTable->rowCount();
        ui->infoTable->insertRow(rc);
        ui->infoTable->setItem(rc, COLUMN_NAME, new QTableWidgetItem(kv.first));
        ui->infoTable->setItem(rc, COLUMN_VALUE, new QTableWidgetItem(kv.second));
    }

    connect(ui->addEntry, &QToolButton::clicked, this,
        [this]() {
            auto rc = ui->infoTable->rowCount();
            insertEntry(rc, "Name", {});
        });

    connect(ui->removeEntry, &QToolButton::clicked, this,
        [this]() {
            auto row = ui->infoTable->currentRow();
            if (row >= 0 && row < data_.count()) {
                ui->infoTable->removeRow(row);
                data_.remove(row);
            }
        });

    connect(ui->infoTable, &QTableWidget::cellChanged, this,
        [this](int row, int col) {
            auto name_item = ui->infoTable->item(row, COLUMN_NAME);
            auto value_item = ui->infoTable->item(row, COLUMN_VALUE);
            if (name_item && value_item && row < data_.count()) {
                data_[row] = { name_item->text(), value_item->text() };
            }
        });

    connect(ui->addIpAddress, &QToolButton::clicked, this,
        [this]() {
            auto rc = ui->infoTable->rowCount();
            insertEntry(rc, "ip", "192.168.0.0");
            insertEntry(++rc, "mask", "255.255.255.0");
            insertEntry(++rc, "gateway", "192.168.0.1");
        });
}

DeviceInfoEditor::~DeviceInfoEditor()
{
    delete ui;
}

bool DeviceInfoEditor::insertEntry(int row, const QString& name, const QString& value)
{
    if (row < 0 || row > data_.count())
        return false;

    ui->infoTable->insertRow(row);
    ui->infoTable->setItem(row, COLUMN_NAME, new QTableWidgetItem(name));
    ui->infoTable->setItem(row, COLUMN_VALUE, new QTableWidgetItem(value));
    data_.insert(row, { name, value });
    return true;
}
