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
#include "deviceproperties.h"
#include "device_pixmap.h"
#include "table_cell_power.h"
#include "tablecellcheckbox.h"
#include "tablecellconnector.h"
#include "ui_deviceproperties.h"

#include <QFileInfo>

namespace {
enum {
    COL_MODEL,
    COL_VISIBLE,
    COL_NAME,
    COL_SOCKET,
    COL_POWER_TYPE,
    COL_PHANTOM,
};

constexpr int IMG_PREVIEW_SIZE = 30;
}

using namespace ceam;

DeviceProperties::DeviceProperties(QWidget* parent, const SharedDeviceData& data)
    : QDialog(parent)
    , ui(new Ui::DeviceProperties)
    , data_(data)
{
    data_.detach();

    setWindowTitle(tr("'%1' properties").arg(data->title()));
    ui->setupUi(this);

    connect(ui->imageChooseButton, SIGNAL(clicked()), this, SLOT(chooseImageDialog()));
    ui->currentImage->setStyleSheet("background-color: white;");
    ui->currentImage->setFrameShape(QFrame::Box);
    ui->currentImage->setFixedSize(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE);
    ui->currentImage->setAlignment(Qt::AlignCenter);
    updateImagePreview();

    ui->zoom->setValue(data->zoom());
    connect(ui->zoom, &QDoubleSpinBox::valueChanged, this, [this](qreal v) { data_->setZoom(v); });

    ui->deviceName->setText(data->title());
    connect(ui->deviceName, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setTitle(txt); });

    ui->vendor->setText(data->vendor());
    connect(ui->vendor, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setVendor(txt); });

    ui->model->setText(data->model());
    connect(ui->model, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setModel(txt); });

    ui->showTitle->setCheckState(data->showTitle() ? Qt::Checked : Qt::Unchecked);
    connect(ui->showTitle, &QCheckBox::stateChanged, this, [this](int state) {
        data_->setShowTitle(state == Qt::Checked);
    });

    setupCategories();
    setupXlets(data);
    setupBattery(data);
}

DeviceProperties::~DeviceProperties()
{
    delete ui;
}

void DeviceProperties::accept()
{
    syncXlets(ui->inlets, data_->inputs());
    syncXlets(ui->outlets, data_->outputs());

    emit acceptData(data_);

    QDialog::accept();
}

void DeviceProperties::chooseImageDialog()
{
    auto dev_pix = new DevicePixmap(this);
    dev_pix->setCurrent(data_->imageIconPath());
    connect(dev_pix, &DevicePixmap::choosePixmap, this,
        [this](const QString& iconName) {
            data_->setImage(iconName);
            updateImagePreview();
        });
    dev_pix->show();
}

void DeviceProperties::setupXletTable(QTableWidget* tab, size_t rows)
{
    auto header = tab->horizontalHeader();
    if (header) {
        header->setMinimumSectionSize(80);
    }

    tab->setRowCount(rows);
    tab->setSelectionBehavior(QAbstractItemView::SelectRows);
    tab->setSelectionMode(QTableWidget::ContiguousSelection);
    tab->setHorizontalHeaderLabels({ tr("Type"), tr("Show"), tr("Name"), tr("Socket"), tr("Power"), tr("Phantom") });
    tab->setColumnWidth(COL_MODEL, 100);
    // tab->setColumnWidth(COL_NAME, 100);
    // tab->setColumnWidth(COL_VISIBLE, 60);
    // tab->setColumnWidth(COL_SOCKET, 60);
    // tab->setColumnWidth(COL_PHANTOM, 50);
    // adjust minimal height
    // if (rows < 6)
    // tab->setMinimumHeight(rows * 30);
}

void DeviceProperties::setupCategories()
{
    foreachItemCategory([this](const char* name, int i) {
        ui->category->addItem(tr(name), i);
    });
    connect(ui->category, &QComboBox::currentIndexChanged, this, [this](int) {
        bool ok = false;
        auto idx = ui->category->currentData().toInt(&ok);
        if (ok) {
            data_->setCategoryIndex(idx);
            switch (data_->category()) {
            case ItemCategory::Human:
            case ItemCategory::Furniture:
                enableCategoryWidgets(false, data_->category());
                break;
            case ItemCategory::Device:
            case ItemCategory::Instrument:
            case ItemCategory::Send:
            case ItemCategory::Return:
            case ItemCategory::MaxCategory:
                enableCategoryWidgets(true, data_->category());
                break;
            }
        } else
            qWarning() << __FUNCTION__ << "can't get category index";
    });
    ui->category->setCurrentIndex(data_->categoryIndex());
}

void DeviceProperties::insertXlet(QTableWidget* tab, int row, const XletData& data, bool resize)
{
    // xlet name
    auto name = new QTableWidgetItem();
    name->setText(data.name());
    tab->setItem(row, COL_NAME, name);

    // model choose
    auto model = new TableCellConnector(tab);
    model->setConnectorModel(data.connectorModel());
    tab->setCellWidget(row, COL_MODEL, model);

    // show/hide
    auto show = new TableCellCheckBox(data.isVisible());
    tab->setCellWidget(row, COL_VISIBLE, show);

    // socket
    auto socket = new QComboBox();
    // TODO
    socket->addItem(tr("Female"), static_cast<int>(ConnectorType::Socket_Female));
    socket->addItem(tr("Male"), static_cast<int>(ConnectorType::Socket_Male));
    socket->setCurrentIndex(data.connectorType() != ConnectorType::Socket_Female);
    tab->setCellWidget(row, COL_SOCKET, socket);

    // power
    auto power = new TableCellPower(data.powerType(), this);
    tab->setCellWidget(row, COL_POWER_TYPE, power);

    // phantom power
    auto phantom = new TableCellCheckBox(data.isPhantomOn());
    phantom->setEnabled(data.supportsPhantomPower());
    tab->setCellWidget(row, COL_PHANTOM, phantom);

    connect(power, &TableCellConnector::currentIndexChanged, this, [tab, row, phantom](int idx) {
        auto power = qobject_cast<TableCellPower*>(tab->cellWidget(row, COL_POWER_TYPE));
        if (power)
            phantom->setEnabled(power->powerType() == PowerType::Phantom);
    });

    auto nrows = std::max<int>(2, tab->rowCount());
    // adjust minimal height
    if (nrows < 6)
        tab->setMinimumHeight((nrows + 1) * 30);

    if (resize)
        tab->resizeColumnsToContents();
}

bool DeviceProperties::duplicateXlet(QTableWidget* tab, int row)
{
    XletData data;
    if (getXletData(tab, row, data)) {
        tab->insertRow(row + 1);
        insertXlet(tab, row + 1, data);
        return true;
    }

    return false;
}

void DeviceProperties::updateImagePreview()
{
    if (data_->image().isEmpty()) {
        ui->currentImage->setText("?");
    } else {
        QIcon icon(data_->imageIconPath());
        if (!icon.isNull())
            ui->currentImage->setPixmap(icon.pixmap(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE));
    }
}

void DeviceProperties::enableCategoryWidgets(bool value, ItemCategory cat)
{
    ui->inlets->setVisible(value);
    ui->outlets->setVisible(value);
    ui->addInlet->setVisible(value);
    ui->removeInlet->setVisible(value);
    ui->addOutlet->setVisible(value);
    ui->removeOutlet->setVisible(value);
    ui->inletsLabel->setVisible(value);
    ui->outletsLabel->setVisible(value);
    ui->batteryType->setVisible(value);
    ui->batteryCount->setVisible(value);
    ui->batteryLabel->setVisible(value);

    const bool is_human = (cat == ItemCategory::Human);
    ui->model->setHidden(is_human);
    ui->modelLabel->setHidden(is_human);
    ui->vendor->setHidden(is_human);
    ui->vendorLabel->setHidden(is_human);

    adjustSize();
    adjustSize();
}

void DeviceProperties::enableInputButtons(int currentRow)
{
    ui->removeInlet->setEnabled(true);
    ui->moveInletDown->setEnabled(currentRow + 1 != ui->inlets->rowCount());
    ui->moveInletUp->setEnabled(currentRow != 0);
}

void DeviceProperties::enableOutputButtons(int currentRow)
{
    ui->removeOutlet->setEnabled(true);
    ui->moveOutletDown->setEnabled(currentRow + 1 != ui->outlets->rowCount());
    ui->moveOutletUp->setEnabled(currentRow != 0);
}

void DeviceProperties::setupXlets(const SharedDeviceData& data)
{
    connect(ui->addInlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->inlets->currentRow();
        if (row >= 0) {
            if (duplicateXlet(ui->inlets, row))
                selectXletRow(ui->inlets, row + 1);
        } else { // no selection
            auto nrows = ui->inlets->rowCount();
            if (nrows > 0) {
                if (duplicateXlet(ui->inlets, nrows - 1))
                    selectXletRow(ui->inlets, nrows);
            } else {
                ui->inlets->setRowCount(1);
                insertXlet(ui->inlets, 0, XletData { {}, ConnectorModel::XLR });
                ui->inlets->selectRow(0);
            }
        }
    });
    connect(ui->removeInlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->inlets->currentRow();
        if (row >= 0) {
            if (removeXlet(ui->inlets, row))
                selectXletRow(ui->inlets, row);
        } else { // no selection
            auto nrows = ui->inlets->rowCount();
            if (nrows > 0) { // remove last inlet
                if (removeXlet(ui->inlets, nrows - 1))
                    selectXletRow(ui->inlets, nrows - 2);
            }
        }

        // remove inlet button
        ui->removeInlet->setEnabled(ui->inlets->rowCount() > 0);
    });
    connect(ui->moveInletUp, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->inlets, ui->inlets->currentRow(), true);
    });
    connect(ui->moveInletDown, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->inlets, ui->inlets->currentRow(), false);
    });
    connect(ui->addOutlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->outlets->currentRow();
        if (row >= 0) {
            if (duplicateXlet(ui->outlets, row))
                selectXletRow(ui->outlets, row + 1);
        } else { // no selection
            auto nrows = ui->outlets->rowCount();
            if (nrows > 0) {
                if (duplicateXlet(ui->outlets, nrows - 1))
                    selectXletRow(ui->outlets, nrows);
            } else {
                ui->outlets->setRowCount(1);
                insertXlet(ui->outlets, 0, XletData { {}, ConnectorModel::XLR });
                ui->outlets->selectRow(0);
            }
        }
    });
    connect(ui->removeOutlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->outlets->currentRow();
        if (row >= 0) {
            if (removeXlet(ui->outlets, row))
                selectXletRow(ui->outlets, row);
        } else { // no selection
            auto nrows = ui->outlets->rowCount();
            if (nrows > 0) {
                if (removeXlet(ui->outlets, nrows - 1))
                    selectXletRow(ui->outlets, nrows - 2);
            }
        }

        ui->removeOutlet->setEnabled(ui->outlets->rowCount() > 0);
    });
    connect(ui->moveOutletUp, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->outlets, ui->outlets->currentRow(), true);
    });
    connect(ui->moveOutletDown, &QToolButton::clicked, this, [this](bool) {
        moveXlet(ui->outlets, ui->outlets->currentRow(), false);
    });

    setupXletTable(ui->inlets, data->inputs().size());
    setupXletTable(ui->outlets, data->outputs().size());

    ui->removeInlet->setEnabled(false);
    ui->moveInletDown->setEnabled(false);
    ui->moveInletUp->setEnabled(false);

    ui->removeOutlet->setEnabled(false);
    ui->moveOutletDown->setEnabled(false);
    ui->moveOutletUp->setEnabled(false);

    connect(ui->inlets, &QTableWidget::currentCellChanged, this,
        [this](int curRow, int, int, int) { enableInputButtons(curRow); });
    connect(ui->outlets, &QTableWidget::currentCellChanged, this,
        [this](int curRow, int curCol, int, int) { enableOutputButtons(curRow); });

    int in_idx = 0;
    for (auto& in : data->inputs()) {
        insertXlet(ui->inlets, in_idx++, in, false);
    }
    ui->inlets->resizeColumnsToContents();

    int out_idx = 0;
    for (auto& out : data->outputs()) {
        insertXlet(ui->outlets, out_idx++, out, false);
    }
    ui->outlets->resizeColumnsToContents();

    ui->inletContainer->setAlignment(ui->addInlet, Qt::AlignLeft);
    ui->inletContainer->setAlignment(ui->removeInlet, Qt::AlignLeft);
    ui->inletContainer->addStretch(10);

    ui->outletContainer->setAlignment(ui->addOutlet, Qt::AlignLeft);
    ui->outletContainer->setAlignment(ui->removeOutlet, Qt::AlignLeft);
    ui->outletContainer->addStretch(10);
}

bool DeviceProperties::removeXlet(QTableWidget* table, int row)
{
    if (row < 0 || row >= table->rowCount()) {
        qWarning() << __FUNCTION__ << "invalid row" << row;
        return false;
    }

    table->removeRow(row);
    return true;
}

void DeviceProperties::syncXlets(const QTableWidget* table, QList<XletData>& xlets)
{
    xlets.clear();
    for (int i = 0; i < table->rowCount(); i++) {
        XletData data;
        if (getXletData(table, i, data))
            xlets.push_back(data);
    }
}

bool DeviceProperties::selectXletRow(QTableWidget* table, int row)
{
    if (!table || row < 0 || row >= table->rowCount())
        return false;

    auto model = table->model();
    if (!model)
        return false;

    table->selectRow(row);
    auto idx = model->index(row, 0);
    table->scrollTo(idx);
    emit table->currentCellChanged(row, 0, -1, -1);
    return true;
}

bool DeviceProperties::moveXlet(QTableWidget* table, int row, bool up)
{
    if (up) {
        if (row > 0 && row < table->rowCount()) {
            XletData prev_data;
            if (getXletData(table, row - 1, prev_data)) {
                table->removeRow(row - 1);
                table->insertRow(row);
                insertXlet(table, row, prev_data, false);
            }

            selectXletRow(table, row - 1);
            return true;
        } else
            return false;
    } else {
        if (row >= 0 && (row + 1) < table->rowCount()) {
            XletData next_data;
            if (getXletData(table, row + 1, next_data)) {
                table->removeRow(row + 1);
                table->insertRow(row);
                insertXlet(table, row, next_data, false);
            }

            selectXletRow(table, row + 1);
            return true;

        } else
            return false;
    }
}

bool DeviceProperties::getXletData(const QTableWidget* table, int row, XletData& data)
{
    if (row < 0 || row >= table->rowCount())
        return false;

    data.setName(table->item(row, COL_NAME)->text());
    auto chk = qobject_cast<TableCellCheckBox*>(table->cellWidget(row, COL_VISIBLE));
    if (chk)
        data.setVisible(chk->isChecked());

    auto model = qobject_cast<TableCellConnector*>(table->cellWidget(row, COL_MODEL));
    if (model)
        data.setConnectorModel(model->connectorModel());

    auto power = qobject_cast<TableCellPower*>(table->cellWidget(row, COL_POWER_TYPE));
    if (power)
        data.setPowerType(power->powerType());

    auto phantom = qobject_cast<TableCellCheckBox*>(table->cellWidget(row, COL_PHANTOM));
    if (phantom && data.supportsPhantomPower())
        data.setPhantom(phantom->isChecked());

    auto socket_type = qobject_cast<QComboBox*>(table->cellWidget(row, COL_SOCKET));
    if (socket_type)
        data.setConnectorType(static_cast<ConnectorType>(socket_type->currentData().toInt()));
    return true;
}

void DeviceProperties::setupBattery(const SharedDeviceData& data)
{
    foreachBatteryType(
        [this](const char* name, int value) {
            ui->batteryType->addItem(name, value);
        });
    connect(ui->batteryType, &QComboBox::currentIndexChanged, this, [this](int v) {
        data_->setBatteryType(ui->batteryType->currentData().toInt());

        // set single battery if has battery and battery count == 0
        if (v != 0) {
            ui->batteryCount->setEnabled(true);

            if (ui->batteryCount->value() == 0)
                ui->batteryCount->setValue(1);
        } else {
            ui->batteryCount->setEnabled(false);
            ui->batteryCount->setValue(0);
        }
    });

    ui->batteryType->setCurrentText(toString(data->batteryType()));
    ui->batteryCount->setEnabled(data->batteryType() != BatteryType::None);
    ui->batteryCount->setValue(data->batteryCount());
    connect(ui->batteryCount, &QSpinBox::valueChanged, this, [this](int v) {
        data_->setBatteryCount(v);
    });
}
