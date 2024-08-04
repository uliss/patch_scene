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
#include <QSvgWidget>

DevicePixmap::DevicePixmap(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DevicePixmap)
{
    ui->setupUi(this);

    constexpr int NCOL = 4;

    int pix_idx = 0;

    QDirIterator dir(":/ceam/cables/resources/devices/", QDirIterator::NoIteratorFlags);
    while (dir.hasNext()) {
        int row = pix_idx / NCOL;
        int col = pix_idx % NCOL;
        pix_idx++;

        if (row >= ui->imageTable->rowCount()) {
            ui->imageTable->insertRow(row);
            ui->imageTable->setRowHeight(row, 50);
        }

        if (col >= ui->imageTable->columnCount()) {
            ui->imageTable->insertColumn(col);
            ui->imageTable->setColumnWidth(col, 50);
        }

        dir.next();

        auto svg = new QSvgWidget(dir.filePath());
        svg->setProperty("filename", dir.filePath());
        ui->imageTable->setCellWidget(row, col, svg);
    }

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(selectImage()));
}

DevicePixmap::~DevicePixmap()
{
    delete ui;
}

void DevicePixmap::setCurrent(const QString& file)
{
    auto nrows = ui->imageTable->rowCount();
    auto ncols = ui->imageTable->columnCount();

    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < ncols; j++) {
            auto w = qobject_cast<QSvgWidget*>(ui->imageTable->cellWidget(i, j));
            if (w) {
                auto filename = w->property("filename").toString();
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
    qDebug() << __FUNCTION__;
    auto row = ui->imageTable->currentRow();
    auto col = ui->imageTable->currentColumn();

    if (row >= 0 && col >= 0) {
        auto w = qobject_cast<QSvgWidget*>(ui->imageTable->cellWidget(row, col));
        if (w) {
            auto filename = w->property("filename").toString();
            qDebug() << filename;
            emit choosePixmap(filename);
        } else {
            emit choosePixmap({});
        }
    }

    qDebug() << ui->imageTable->currentRow() << ui->imageTable->currentColumn();
}
