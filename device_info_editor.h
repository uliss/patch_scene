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
#ifndef DEVICE_INFO_EDITOR_H
#define DEVICE_INFO_EDITOR_H

#include "device_common.h"

#include <QDialog>

namespace Ui {
class DeviceInfoEditor;
}

namespace ceam {
class DeviceInfoEditor : public QDialog {
    Q_OBJECT

public:
    explicit DeviceInfoEditor(const DeviceDataInfo& data, QWidget* parent = nullptr);
    ~DeviceInfoEditor();

    const DeviceDataInfo& data() const { return data_; }

private:
    bool insertEntry(int row, const QString& name, const QString& value);

private:
    Ui::DeviceInfoEditor* ui;
    DeviceDataInfo data_;
};
}

#endif // DEVICE_INFO_EDITOR_H
