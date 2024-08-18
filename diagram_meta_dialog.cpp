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

namespace {
enum ContactTableOrder {
    COL_NAME,
    COL_WORK,
    COL_PHONE,
    COL_EMAIL
};
}

using namespace ceam;

DiagramMetaDialog::DiagramMetaDialog(const DiagramMeta& meta, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DiagramMetaDialog)
    , meta_(meta)
{
    ui->setupUi(this);

    initTitle();
    initInfo();
    initEventDate();
    initContacts();

    connect(this, &QDialog::accepted, this, [this]() {
        syncContacts();
    });
}

DiagramMetaDialog::~DiagramMetaDialog()
{
    delete ui;
}

void DiagramMetaDialog::initTitle()
{
    ui->titleEdit->setText(meta_.title());
    connect(ui->titleEdit, &QLineEdit::textChanged, this, [this](const QString& title) {
        meta_.setTitle(title);
    });
}

void DiagramMetaDialog::initInfo()
{
    ui->infoEdit->setPlainText(meta_.info());
    connect(ui->infoEdit, &QPlainTextEdit::textChanged, this, [this]() {
        meta_.setInfo(ui->infoEdit->toPlainText());
    });
}

void DiagramMetaDialog::initEventDate()
{
    ui->eventDateEdit->setCalendarPopup(true);
    ui->eventDateEdit->setDate(meta_.eventDate());
    connect(ui->eventDateEdit, &QDateEdit::userDateChanged, this, [this](const QDate& date) {
        meta_.setEventDate(date);
    });
    ui->eventDateEdit->setStyleSheet(
        "#eventDateEdit QWidget#qt_calendar_prevmonth { qproperty-icon: url(\":/icons/arrow_back.svg\");}"
        "#eventDateEdit QWidget#qt_calendar_nextmonth { qproperty-icon : url(\":/icons/arrow_forward.svg\");}");
}

void DiagramMetaDialog::initContacts()
{
    ui->contactsTable->setHorizontalHeaderLabels({ tr("Name"), tr("Work"), tr("Phone"), tr("Email") });
    int row = 0;
    for (auto& c : meta_.contacts()) {
        ui->contactsTable->insertRow(row);

        auto name = new QTableWidgetItem(c.name());
        ui->contactsTable->setItem(row, COL_NAME, name);

        auto work = new QTableWidgetItem(c.work());
        ui->contactsTable->setItem(row, COL_WORK, work);

        auto phone = new QTableWidgetItem(c.phone());
        ui->contactsTable->setItem(row, COL_PHONE, phone);

        auto email = new QTableWidgetItem(c.email());
        ui->contactsTable->setItem(row, COL_EMAIL, email);

        row++;
    }

    connect(ui->contactsTable, SIGNAL(itemChanged(QTableWidgetItem*)), ui->contactsTable, SLOT(resizeColumnsToContents()));
    ui->contactsTable->resizeColumnsToContents();

    connect(ui->addContact, &QToolButton::clicked, this, [this]() {
        auto row = ui->contactsTable->currentRow();
        if (row < 0)
            row = ui->contactsTable->rowCount();

        ui->contactsTable->insertRow(row);

        auto name = new QTableWidgetItem(tr("John Doe"));
        ui->contactsTable->setItem(row, COL_NAME, name);

        auto work = new QTableWidgetItem(tr("manager"));
        ui->contactsTable->setItem(row, COL_WORK, work);

        auto phone = new QTableWidgetItem(tr("+7XXX XXX-XX-XX"));
        ui->contactsTable->setItem(row, COL_PHONE, phone);

        auto email = new QTableWidgetItem();
        ui->contactsTable->setItem(row, COL_EMAIL, email);
    });

    connect(ui->removeContact, &QToolButton::clicked, this, [this]() {
        if (ui->contactsTable->rowCount() < 1)
            return;

        auto row = ui->contactsTable->currentRow();
        if (row < 0)
            row = ui->contactsTable->rowCount() - 1;

        ui->contactsTable->removeRow(row);
    });
}

void DiagramMetaDialog::syncContacts()
{
    meta_.contacts().clear();

    const int NROWS = ui->contactsTable->rowCount();
    for (int i = 0; i < NROWS; i++) {
        Contact ct;

        auto name = ui->contactsTable->item(i, COL_NAME);
        if (name)
            ct.setName(name->text());

        auto work = ui->contactsTable->item(i, COL_WORK);
        if (work)
            ct.setWork(work->text());

        auto phone = ui->contactsTable->item(i, COL_PHONE);
        if (phone)
            ct.setPhone(phone->text());

        auto email = ui->contactsTable->item(i, COL_EMAIL);
        if (email)
            ct.setEmail(email->text());

        meta_.contacts().push_back(ct);
    }
}
