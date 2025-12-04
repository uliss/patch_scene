/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
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
#include "comment_editor.h"
#include "ui_comment_editor.h"

#include <QColorDialog>

using namespace ceam;

CommentEditor::CommentEditor(const SharedItemData& data, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CommentEditor)
    , data_(data)
{
    ui->setupUi(this);

    ui->textEdit->setPlainText(data->title());
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, [this]() {
        data_->setTitle(ui->textEdit->toPlainText());
    });

    ui->borderWidth->setValue(data->borderWidth());
    connect(ui->borderWidth, &QSpinBox::valueChanged, this, [this](int value) {
        data_->setBorderWidth(value);
    });

    connect(ui->borderColorBtn, &QToolButton::clicked, this, [this]() {
        QColorDialog cd(ui->borderColorBtn);
        cd.setCurrentColor(data_->borderColor());
        connect(&cd, &QColorDialog::accepted, this, [this, &cd]() {
            data_->setBorderColor(cd.currentColor());
            updateButtonColors();
        });
        cd.exec();
    });

    connect(ui->backgroundColorBtn, &QToolButton::clicked, this, [this]() {
        QColorDialog cd(ui->backgroundColorBtn);
        cd.setCurrentColor(data_->backgroundColor());
        connect(&cd, &QColorDialog::accepted, this, [this, &cd]() {
            data_->setBackgroundColor(cd.currentColor());
            updateButtonColors();
        });
        cd.exec();
    });

    connect(ui->textColorBtn, &QToolButton::clicked, this, [this]() {
        QColorDialog cd(ui->textColorBtn);
        cd.setCurrentColor(data_->textColor());
        connect(&cd, &QColorDialog::accepted, this, [this, &cd]() {
            data_->setTextColor(cd.currentColor());
            updateButtonColors();
        });
        cd.exec();
    });

    updateButtonColors();
}

CommentEditor::~CommentEditor()
{
    delete ui;
}

void CommentEditor::accept()
{
    emit acceptData(data_);
    QDialog::accept();
}

void CommentEditor::updateButtonColors()
{
    QPixmap pix(64, 64);

    if (data_->textColor().isValid()) {
        pix.fill(data_->textColor());
        ui->textColorBtn->setIcon(pix);
    }

    if (data_->borderColor().isValid()) {
        pix.fill(data_->borderColor());
        ui->borderColorBtn->setIcon(pix);
    }

    if (data_->backgroundColor().isValid()) {
        pix.fill(data_->backgroundColor());
        ui->backgroundColorBtn->setIcon(pix);
    }
}
