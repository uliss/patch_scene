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
#include "device_editor.h"
#include "device_info_editor.h"
#include "device_pixmap.h"
#include "logging.hpp"
#include "tablecellcheckbox.h"
#include "ui_device_editor.h"
#include "vendor_list.h"
#include "xlets_logic_view_editor.h"
#include "xlets_user_view_editor.h"

#include <QCompleter>
#include <QFileInfo>
#include <QStandardItemModel>

namespace {

constexpr int IMG_PREVIEW_SIZE = 30;

}

using namespace ceam;

DeviceEditor::DeviceEditor(const SharedDeviceData& data, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DeviceProperties)
    , data_(data)
{
    data_.detach();

    setWindowTitle(tr("'%1' properties").arg(data->title()));
    ui->setupUi(this);

    auto vend_comp = new QCompleter(VendorList::instance().all(), this);
    vend_comp->setCaseSensitivity(Qt::CaseInsensitive);
    ui->vendor->setCompleter(vend_comp);

    connect(ui->imageChooseButton, SIGNAL(clicked()), this, SLOT(chooseImageDialog()));
    ui->currentImage->setStyleSheet("background-color: white;");
    ui->currentImage->setFrameShape(QFrame::Box);
    ui->currentImage->setFixedSize(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE);
    ui->currentImage->setAlignment(Qt::AlignCenter);
    updateImagePreview();

    connect(ui->additionalInfo, &QToolButton::clicked, this,
        [this]() {
            auto dialog = new DeviceInfoEditor(data_->info(), this);
            connect(dialog, &DeviceInfoEditor::finished, this,
                [this, dialog](int rc) {
                    data_->info() = dialog->data();
                });
            dialog->exec();
        });

    ui->viewsEdit->setEnabled(data_->hasAnyXput());

    ui->zoom->setValue(data->zoom());
    connect(ui->zoom, &QDoubleSpinBox::valueChanged, this, [this](qreal v) { data_->setZoom(v); });

    ui->deviceName->setText(data->title());
    connect(ui->deviceName, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setTitle(txt); });

    ui->vendor->setText(data->vendor());
    connect(ui->vendor, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setVendor(txt); });

    ui->model->setText(data->model());
    connect(ui->model, &QLineEdit::textChanged, this, [this](const QString& txt) { data_->setModel(txt); });

    ui->showTitle->setCheckState(data->showTitle() ? Qt::Checked : Qt::Unchecked);
    connect(ui->showTitle, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        data_->setShowTitle(state == Qt::Checked);
    });

    connect(ui->inputsEditLogical, &QToolButton::clicked, this, [this]() {
        auto dialog = new XletLogicalEditor(this, data_->inputs(), data_->logicViewData(), XletType::In);
        connect(dialog, &XletLogicalEditor::finished, this, [this](int) {
            ui->viewsEdit->setEnabled(data_->hasAnyXput());
        });
        dialog->setWindowTitle(tr("Logical inputs editor"));
        dialog->exec();
    });
    connect(ui->outputsEditLogical, &QToolButton::clicked, this, [this]() {
        auto dialog = new XletLogicalEditor(this, data_->outputs(), data_->logicViewData(), XletType::Out);
        connect(dialog, &XletLogicalEditor::finished, this, [this](int) {
            ui->viewsEdit->setEnabled(data_->hasAnyXput());
        });
        dialog->setWindowTitle(tr("Logical outputs editor"));
        dialog->exec();
    });

    connect(ui->viewsEdit, &QToolButton::clicked, this, [this]() {
        auto dialog = new XletsUserViewEditor(this, data_);
        connect(dialog, &XletsUserViewEditor::acceptData, this, [this](const SharedDeviceData& data) {
            data_ = data;
        });

        dialog->setWindowTitle(tr("User views editor"));
        dialog->exec();
    });

    setupCategories();
    setupBattery(data);
    setupImageMirror(data);
}

DeviceEditor::~DeviceEditor()
{
    delete ui;
}

void DeviceEditor::accept()
{
    emit acceptData(data_);
    QDialog::accept();
}

void DeviceEditor::chooseImageDialog()
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

void DeviceEditor::setupCategories()
{
    foreachItemCategory([this](const char* name, int i) {
        ui->category->addItem(QCoreApplication::translate("ceam", name), i);
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
            WARN() << "can't get category index";
    });
    ui->category->setCurrentIndex(data_->categoryIndex());
}

void DeviceEditor::updateImagePreview()
{
    if (data_->image().isEmpty()) {
        ui->currentImage->setText("?");
    } else {
        QIcon icon(data_->imageIconPath());
        if (!icon.isNull())
            ui->currentImage->setPixmap(icon.pixmap(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE));
    }
}

void DeviceEditor::enableCategoryWidgets(bool value, ItemCategory cat)
{
    ui->batteryCount->setVisible(value);
    ui->batteryLabel->setVisible(value);
    ui->batteryType->setVisible(value);

    const bool is_human = (cat == ItemCategory::Human);
    ui->model->setHidden(is_human);
    ui->modelLabel->setHidden(is_human);
    ui->vendor->setHidden(is_human);
    ui->vendorLabel->setHidden(is_human);
    ui->inputsEditLogical->setHidden(is_human);
    ui->outputsEditLogical->setHidden(is_human);
    ui->viewsEdit->setHidden(is_human);

    adjustSize();
    adjustSize();
}

void DeviceEditor::setupBattery(const SharedDeviceData& data)
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

void DeviceEditor::setupImageMirror(const SharedDeviceData& data)
{
    ui->imageMirror->setChecked(data->imageMirror() != ImageMirrorType::None);
    connect(ui->imageMirror, &QCheckBox::checkStateChanged, this,
        [this](Qt::CheckState state) {
                data_->setImageMirror(state == Qt::Checked
                                  ? ImageMirrorType::Horizontal
                                  : ImageMirrorType::None);
        });
}
