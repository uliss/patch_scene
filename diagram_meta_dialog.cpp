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
#include "diagram_meta_dialog.h"
#include "ui_diagram_meta_dialog.h"

DiagramMetaDialog::DiagramMetaDialog(const DiagramMeta& meta, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DiagramMetaDialog)
    , meta_(meta)
{
    ui->setupUi(this);

    ui->titleEdit->setText(meta_.title());
    connect(ui->titleEdit, &QLineEdit::textChanged, this, [this](const QString& title) {
        meta_.setTitle(title);
    });

    ui->infoEdit->setPlainText(meta_.info());
    connect(ui->infoEdit, &QPlainTextEdit::textChanged, this, [this]() {
        meta_.setInfo(ui->infoEdit->toPlainText());
    });

    ui->eventDateEdit->setCalendarPopup(true);
    ui->eventDateEdit->setDate(meta_.eventDate());
    connect(ui->eventDateEdit, &QDateEdit::userDateChanged, this, [this](const QDate& date) {
        meta_.setEventDate(date);
    });
    ui->eventDateEdit->setStyleSheet(
        "#eventDateEdit QWidget#qt_calendar_prevmonth { qproperty-icon: url(\":/icons/arrow_back.svg\");}"
        "#eventDateEdit QWidget#qt_calendar_nextmonth { qproperty-icon : url(\":/icons/arrow_forward.svg\");}");

    ui->contactsTable->setHorizontalHeaderLabels({ tr("Name"), tr("Work"), tr("Phone"), tr("Email") });
    int row = 0;
    for (auto& c : meta_.contacts()) {
        ui->contactsTable->insertRow(row);

        auto name = new QTableWidgetItem(c.name());
        ui->contactsTable->setItem(row, 0, name);

        auto work = new QTableWidgetItem(c.work());
        ui->contactsTable->setItem(row, 1, work);

        auto phone = new QTableWidgetItem(c.phone());
        ui->contactsTable->setItem(row, 2, phone);

        auto email = new QTableWidgetItem(c.email());
        ui->contactsTable->setItem(row, 3, email);

        row++;
    }

    connect(ui->contactsTable, SIGNAL(itemChanged(QTableWidgetItem*)), ui->contactsTable, SLOT(resizeColumnsToContents()));

    ui->contactsTable->resizeColumnsToContents();
}

DiagramMetaDialog::~DiagramMetaDialog()
{
    delete ui;
}
