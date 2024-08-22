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
    tab->setSelectionMode(QTableWidget::NoSelection);
    tab->setHorizontalHeaderLabels({ tr("Type"), tr("Show"), tr("Name"), tr("Socket"), tr("Phantom") });
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
    name->setText(data.name_);
    tab->setItem(row, COL_NAME, name);

    // model choose
    auto model = new TableCellConnector(tab);
    model->setConnectorModel(data.model_);
    tab->setCellWidget(row, COL_MODEL, model);

    // show/hide
    auto show = new TableCellCheckBox(data.visible_);
    tab->setCellWidget(row, COL_VISIBLE, show);

    // socket
    auto socket = new QComboBox();
    socket->addItem(tr("Female"), static_cast<int>(ConnectorType::Socket_Female));
    socket->addItem(tr("Male"), static_cast<int>(ConnectorType::Socket_Male));
    socket->setCurrentIndex(data.type_ != ConnectorType::Socket_Female);
    tab->setCellWidget(row, COL_SOCKET, socket);

    // phantom power
    auto phantom = new TableCellCheckBox(data.phantom_power_);
    phantom->setEnabled(connectSupportsPhantomPower(data.model_));
    tab->setCellWidget(row, COL_PHANTOM, phantom);

    connect(model, &TableCellConnector::currentIndexChanged, this, [tab, row, phantom](int idx) {
        auto model = qobject_cast<TableCellConnector*>(tab->cellWidget(row, COL_MODEL));
        if (model)
            phantom->setEnabled(connectSupportsPhantomPower(model->connectorModel()));
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

void DeviceProperties::setupXlets(const SharedDeviceData& data)
{
    connect(ui->addInlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->inlets->currentRow();
        if (row >= 0) {
            duplicateXlet(ui->inlets, row);
        } else { // no selection
            auto nrows = ui->inlets->rowCount();
            if (nrows > 0) {
                duplicateXlet(ui->inlets, nrows - 1);
            } else {
                ui->inlets->setRowCount(1);
                insertXlet(ui->inlets, 0, XletData { {}, ConnectorModel::XLR });
            }
        }
    });
    connect(ui->removeInlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->inlets->currentRow();
        if (row >= 0) {
            removeXlet(ui->inlets, row);
        } else { // no selection
            auto nrows = ui->inlets->rowCount();
            if (nrows > 0) {
                removeXlet(ui->inlets, nrows - 1);
            }
        }
    });
    connect(ui->addOutlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->outlets->currentRow();
        if (row >= 0) {
            duplicateXlet(ui->outlets, row);
        } else { // no selection
            auto nrows = ui->outlets->rowCount();
            if (nrows > 0) {
                duplicateXlet(ui->outlets, nrows - 1);
            } else {
                ui->outlets->setRowCount(1);
                insertXlet(ui->outlets, 0, XletData { {}, ConnectorModel::XLR });
            }
        }
    });
    connect(ui->removeOutlet, &QToolButton::clicked, this, [this](bool) {
        auto row = ui->outlets->currentRow();
        if (row >= 0) {
            removeXlet(ui->outlets, row);
        } else { // no selection
            auto nrows = ui->outlets->rowCount();
            if (nrows > 0) {
                removeXlet(ui->outlets, nrows - 1);
            }
        }
    });

    setupXletTable(ui->inlets, data->inputs().size());
    setupXletTable(ui->outlets, data->outputs().size());

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

bool DeviceProperties::getXletData(const QTableWidget* table, int row, XletData& data)
{
    if (row < 0 || row >= table->rowCount())
        return false;

    data.name_ = table->item(row, COL_NAME)->text();
    auto chk = qobject_cast<TableCellCheckBox*>(table->cellWidget(row, COL_VISIBLE));
    if (chk)
        data.visible_ = chk->isChecked();

    auto model = qobject_cast<TableCellConnector*>(table->cellWidget(row, COL_MODEL));
    if (model)
        data.model_ = model->connectorModel();

    auto phantom = qobject_cast<TableCellCheckBox*>(table->cellWidget(row, COL_PHANTOM));
    if (phantom && connectSupportsPhantomPower(data.model_))
        data.phantom_power_ = phantom->isChecked();

    auto socket_type = qobject_cast<QComboBox*>(table->cellWidget(row, COL_SOCKET));
    if (socket_type)
        data.type_ = static_cast<ConnectorType>(socket_type->currentData().toInt());
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
