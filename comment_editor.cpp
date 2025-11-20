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

using namespace ceam;

CommentEditor::CommentEditor(const SharedDeviceData& data, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::CommentEditor)
    , data_(data)
{
    ui->setupUi(this);

    ui->textEdit->setPlainText(data->title());
    connect(ui->textEdit, &QPlainTextEdit::textChanged, this, [this]() {
        data_->setTitle(ui->textEdit->toPlainText());
    });
    // ui->textEdit->textCursor().movePosition(QTextCursor::End);

    ui->borderWidth->setValue(data->borderWidth());
    connect(ui->borderWidth, &QSpinBox::valueChanged, this, [this](int value) {
        data_->setBorderWidth(value);
    });
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
