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
#include "device_pixmap.h"
#include "ui_device_pixmap.h"

#include <QDirIterator>
#include <QLabel>

constexpr const char* PROP_FILEPATH = "filepath";
constexpr const char* PROP_ICONNAME = "iconname";

DevicePixmap::DevicePixmap(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DevicePixmap)
{
    ui->setupUi(this);

    constexpr int NCOL = 6;
    const auto CELL_SIZE = 60;
    ui->imageTable->setMinimumWidth(NCOL * CELL_SIZE);
    ui->imageTable->setMinimumHeight(4 * CELL_SIZE);
    ui->imageTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    int pix_idx = 0;

    QDirIterator dir(":/ceam/cables/resources/devices/", QDirIterator::NoIteratorFlags);
    while (dir.hasNext()) {
        int row = pix_idx / NCOL;
        int col = pix_idx % NCOL;
        pix_idx++;

        if (row >= ui->imageTable->rowCount()) {
            ui->imageTable->insertRow(row);
            ui->imageTable->setRowHeight(row, CELL_SIZE);
        }

        if (col >= ui->imageTable->columnCount()) {
            ui->imageTable->insertColumn(col);
            ui->imageTable->setColumnWidth(col, CELL_SIZE);
        }

        dir.next();

        QIcon icon(dir.filePath());
        if (!icon.isNull()) {
            auto svg = new QLabel();
            svg->setPixmap(icon.pixmap(CELL_SIZE, CELL_SIZE));
            svg->setToolTip(dir.fileName());
            svg->setProperty(PROP_FILEPATH, dir.filePath());
            svg->setProperty(PROP_ICONNAME, dir.fileInfo().baseName());
            ui->imageTable->setCellWidget(row, col, svg);
        }
    }

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(selectImage()));
}

DevicePixmap::~DevicePixmap()
{
    delete ui;
}

void DevicePixmap::setCurrent(const QString& file)
{
    const auto nrows = ui->imageTable->rowCount();
    const auto ncols = ui->imageTable->columnCount();

    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            auto w = qobject_cast<QLabel*>(ui->imageTable->cellWidget(i, j));
            if (w) {
                auto filename = w->property(PROP_FILEPATH).toString();
                if (filename == file) {
                    ui->imageTable->setCurrentCell(i, j);
                    return;
                }
            }
        }
    }
}

void DevicePixmap::selectImage()
{
    const auto row = ui->imageTable->currentRow();
    const auto col = ui->imageTable->currentColumn();

    if (row >= 0 && col >= 0) {
        auto w = qobject_cast<QLabel*>(ui->imageTable->cellWidget(row, col));
        if (w) {
            auto icon_name = w->property(PROP_ICONNAME).toString();
            emit choosePixmap(icon_name);
        } else {
            emit choosePixmap({});
        }
    }
}
