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
#ifndef DEVICEPROPERTIES_H
#define DEVICEPROPERTIES_H

#include "device_common.h"
#include "device_xlet.h"

#include <QDialog>

namespace Ui {
class DeviceProperties;
}

class QComboBox;
class QTableWidget;
class QTableWidgetItem;

namespace ceam {

class DeviceProperties : public QDialog {
    Q_OBJECT

public:
    explicit DeviceProperties(const SharedDeviceData& data, QWidget* parent = nullptr);
    ~DeviceProperties();

signals:
    void acceptData(SharedDeviceData data);

public Q_SLOTS:
    void accept() override;
    void chooseImageDialog();

private:
    void setupBattery(const SharedDeviceData& data);
    void setupCategories();
    void setupImageMirror(const SharedDeviceData& data);
    void setupXletTable(QTableWidget* tab, size_t rows);
    void setupXlets(const SharedDeviceData& data);

    void insertXlet(QTableWidget* tab, int row, const XletData& data, bool resize = true);
    bool duplicateXlet(QTableWidget* tab, int row);
    bool moveXlet(QTableWidget* table, int row, bool up);

    void updateImagePreview();

    void enableCategoryWidgets(bool value, ItemCategory cat);
    void enableInputButtons(int currentRow);
    void enableOutputButtons(int currentRow);

private:
    static bool getXletData(const QTableWidget* table, int row, XletData& data);
    static bool removeXlet(QTableWidget* table, int row);
    static void syncXlets(const QTableWidget* table, QList<XletData>& xlets);
    static bool selectXletRow(QTableWidget* table, int row);

private:
    Ui::DeviceProperties* ui;
    SharedDeviceData data_;
};

}

#endif // DEVICEPROPERTIES_H
