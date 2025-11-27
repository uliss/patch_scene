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

#include <unordered_map>
#include <unordered_set>

#include <QCompleter>
#include <QFileInfo>
#include <QStandardItemModel>

using namespace ceam;

#define DECLARE_EDIT(name) { DeviceEditor::name, &DeviceEditor::enable##name##Widgets }

std::unordered_map<DeviceEditor::EditorWidgetType, void (DeviceEditor::*)(bool)> DeviceEditor::field_edit_fn_ {
    DECLARE_EDIT(Model),
    DECLARE_EDIT(Vendor),
    DECLARE_EDIT(Additional),
    DECLARE_EDIT(Battery),
    DECLARE_EDIT(Volume),
    DECLARE_EDIT(Weight),
    DECLARE_EDIT(Power),
    DECLARE_EDIT(Views),
    DECLARE_EDIT(Inputs),
    DECLARE_EDIT(Outputs),
};

DeviceEditor::DeviceEditor(const SharedItemData& data, QWidget* parent)
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

    connect(ui->currentImage, SIGNAL(clicked()), this, SLOT(chooseImageDialog()));
    ui->currentImage->setImagePath(data_->imageIconPath());

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
    connect(ui->showTitle, &QCheckBox::stateChanged, this, [this](int state) {
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
        connect(dialog, &XletsUserViewEditor::acceptData, this, [this](const SharedItemData& data) {
            data_ = data;
        });

        dialog->setWindowTitle(tr("User views editor"));
        dialog->exec();
    });

    setupCategories();
    setupBattery(data);
    setupImageMirror(data);
    setupPhysics();
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
            ui->currentImage->setImagePath(data_->imageIconPath());
        });
    dev_pix->show();
}

bool DeviceEditor::isWidgetEnabled(ItemCategory cat, EditorWidgetType w)
{
    using ItemWidgets = std::unordered_set<ceam::DeviceEditor::EditorWidgetType>;

    static const std::unordered_map<ceam::ItemCategory, ItemWidgets> cat_widget_map {
        {
            ItemCategory::Device,
            ItemWidgets { { Model, Vendor, Additional, Inputs, Outputs, Views, Battery, Weight, Volume, Power } },
        },
        {
            ItemCategory::Instrument,
            ItemWidgets { { Inputs, Outputs, Battery, Weight, Volume, Power } },
        },
        {
            ItemCategory::Return,
            ItemWidgets { { Model, Vendor, Additional, Outputs, Views } },
        },
        {
            ItemCategory::Send,
            ItemWidgets { { Model, Vendor, Additional, Inputs, Views } },
        },
        {
            ItemCategory::Furniture,
            ItemWidgets { { Battery, Weight, Volume, Power } },
        },
        {
            ItemCategory::Human,
            {},
        },
        {
            ItemCategory::Comment,
            {},
        },
    };

    auto x = cat_widget_map.find(cat);
    if (x == cat_widget_map.end())
        return false;

    return x->second.find(w) != x->second.end();
}

void DeviceEditor::setupCategories()
{
    foreachItemCategory([this](ItemCategory cat, const char* name, int i) {
        switch (cat) {
        case ItemCategory::Comment:
            return;
        default:
            break;
        }

        ui->category->addItem(QCoreApplication::translate("ceam", name), i);
    });
    connect(ui->category, &QComboBox::currentIndexChanged, this, [this](int) {
        bool ok = false;
        auto idx = ui->category->currentData().toInt(&ok);
        if (ok) {
            data_->setCategoryIndex(idx);
            enableWidgets(data_->category());
        } else
            WARN() << "can't get category index";
    });
    ui->category->setCurrentIndex(data_->categoryIndex());
}

void DeviceEditor::enableAdditionalWidgets(bool value)
{
    ui->additionalInfo->setVisible(value);
}

void DeviceEditor::enableBatteryWidgets(bool value)
{
    ui->batteryCapacity->setVisible(value);
    ui->batteryCapacityLabel->setVisible(value);
    ui->batteryCount->setVisible(value);
    ui->batteryCountLabel->setVisible(value);
    ui->batteryLabel->setVisible(value);
    ui->batteryType->setVisible(value);
}

void DeviceEditor::enableModelWidgets(bool value)
{
    ui->model->setVisible(value);
    ui->modelLabel->setVisible(value);
}

void DeviceEditor::enableInputsWidgets(bool value)
{
    ui->inputsEditLogical->setVisible(value);
}

void DeviceEditor::enableOutputsWidgets(bool value)
{
    ui->outputsEditLogical->setVisible(value);
}

void DeviceEditor::enablePowerWidgets(bool value)
{
    ui->powerLabel->setVisible(value);
    ui->powerInput->setVisible(value);
}

void DeviceEditor::enableVendorWidgets(bool value)
{
    ui->vendor->setVisible(value);
    ui->vendorLabel->setVisible(value);
}

void DeviceEditor::enableWeightWidgets(bool value)
{
    ui->weightLabel->setVisible(value);
    ui->weightInput->setVisible(value);
}

void DeviceEditor::enableVolumeWidgets(bool value)
{
    ui->volumeInput->setVisible(value);
    ui->volumeLabel->setVisible(value);
}

void DeviceEditor::enableViewsWidgets(bool value)
{
    ui->viewsEdit->setVisible(value);
}

void DeviceEditor::enableWidgets(ItemCategory cat)
{
    for (auto& kv : field_edit_fn_) {
        auto mem_fn = kv.second;
        (this->*mem_fn)(isWidgetEnabled(cat, kv.first));
    }

    ui->commutationLabel->setVisible(isWidgetEnabled(cat, Inputs)
        || isWidgetEnabled(cat, Outputs)
        || isWidgetEnabled(cat, Views));

    ui->physicsLabel->setVisible(isWidgetEnabled(cat, Weight)
        || isWidgetEnabled(cat, Volume)
        || isWidgetEnabled(cat, Power));

    adjustSize();
    adjustSize();
}

void DeviceEditor::setupBattery(const SharedItemData& data)
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
            ui->batteryCapacity->setEnabled(true);

            if (ui->batteryCount->value() == 0)
                ui->batteryCount->setValue(1);
        } else {
            ui->batteryCount->setEnabled(false);
            ui->batteryCount->setValue(0);
            ui->batteryCapacity->setEnabled(false);
        }
    });

    ui->batteryType->setCurrentText(toString(data->batteryType()));
    ui->batteryCount->setEnabled(data->batteryType() != BatteryType::None);
    ui->batteryCount->setValue(data->batteryCount());
    connect(ui->batteryCount, &QSpinBox::valueChanged, this, [this](int v) {
        data_->setBatteryCount(v);
    });

    ui->batteryCapacity->setValue(data->batteryCapacity());
    ui->batteryCapacity->setEnabled(data->batteryType() != BatteryType::None);
    connect(ui->batteryCapacity, &QSpinBox::valueChanged, this, [this](int v) {
        data_->setBatteryCapacity(v);
    });
}

void DeviceEditor::setupPhysics()
{
    ui->weightInput->setValue(data_->weight());
    ui->volumeInput->setValue(data_->volume());
    ui->powerInput->setValue(data_->power());

    connect(ui->weightInput, &QDoubleSpinBox::valueChanged, this, [this](qreal v) {
        data_->setWeight(v);
    });

    connect(ui->volumeInput, &QDoubleSpinBox::valueChanged, this, [this](qreal v) {
        data_->setVolume(v);
    });

    connect(ui->powerInput, &QSpinBox::valueChanged, this, [this](int v) {
        data_->setPower(v);
    });
}

void DeviceEditor::setupImageMirror(const SharedItemData& data)
{
    ui->imageMirror->setChecked(data->imageMirror() != ImageMirrorType::None);
    connect(ui->imageMirror, &QCheckBox::stateChanged, this,
        [this](int state) {
            data_->setImageMirror(state == Qt::Checked
                    ? ImageMirrorType::Horizontal
                    : ImageMirrorType::None);
        });
}
