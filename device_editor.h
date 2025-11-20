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
#ifndef DEVICE_EDITOR_H
#define DEVICE_EDITOR_H

#include "device_common.h"

#include <QDialog>
#include <unordered_map>

namespace Ui {
class DeviceProperties;
}

class QComboBox;
class QTableWidget;
class QTableWidgetItem;

namespace ceam {

class DeviceEditor : public QDialog {
    Q_OBJECT

public:
    explicit DeviceEditor(const SharedDeviceData& data, QWidget* parent = nullptr);
    ~DeviceEditor();

public:
    enum EditorWidgetType {
        Model,
        Vendor,
        Inputs,
        Outputs,
        Views,
        Battery,
        Weight,
        Volume,
    };

    static bool isWidgetEnabled(ItemCategory cat, EditorWidgetType w);

signals:
    void acceptData(SharedDeviceData data);

public Q_SLOTS:
    void accept() override;
    void chooseImageDialog();

private:
    void setupBattery(const SharedDeviceData& data);
    void setupCategories();
    void setupImageMirror(const SharedDeviceData& data);

    void updateImagePreview();

    void enableBatteryWidgets(bool value);
    void enableInputsWidgets(bool value);
    void enableModelWidgets(bool value);
    void enableOutputsWidgets(bool value);
    void enableVendorWidgets(bool value);
    void enableViewsWidgets(bool value);
    void enableVolumeWidgets(bool value);
    void enableWeightWidgets(bool value);

    void enableWidgets(ItemCategory cat);

private:
    Ui::DeviceProperties* ui;
    SharedDeviceData data_;

    static std::unordered_map<EditorWidgetType, void (DeviceEditor::*)(bool)> field_edit_fn_;
};

} // namespace ceam

#endif // DEVICE_EDITOR_H
