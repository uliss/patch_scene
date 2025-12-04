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
        auto color = data_->borderColor();
        if (showColorDialog(color, ui->borderColorBtn))
            data_->setBorderColor(color);
    });

    connect(ui->backgroundColorBtn, &QToolButton::clicked, this, [this]() {
        auto color = data_->backgroundColor();
        if (showColorDialog(color, ui->backgroundColorBtn))
            data_->setBackgroundColor(color);
    });

    connect(ui->textColorBtn, &QToolButton::clicked, this, [this]() {
        auto color = data_->textColor();
        if (showColorDialog(color, ui->textColorBtn))
            data_->setTextColor(color);
    });

    updateButtonColor(data_->borderColor(), ui->borderColorBtn);
    updateButtonColor(data_->backgroundColor(), ui->backgroundColorBtn);
    updateButtonColor(data_->textColor(), ui->textColorBtn);
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

bool CommentEditor::showColorDialog(QColor& color, QToolButton* btn)
{
    if (!btn)
        return false;

    QColorDialog dialog(btn);
    if (color.isValid())
        dialog.setCurrentColor(color);

    dialog.setOption(QColorDialog::ShowAlphaChannel, true);

    if (dialog.exec() == QDialog::Accepted) {
        QColor new_color = dialog.selectedColor();
        if (color != new_color && updateButtonColor(new_color, btn)) {
            color = new_color;
            return true;
        }
    }

    return false;
}

bool CommentEditor::updateButtonColor(const QColor& c, QToolButton* btn)
{
    if (c.isValid()) {
        QPixmap pix(64, 64);
        pix.fill(c);
        btn->setIcon(pix);
        return true;
    } else {
        return false;
    }
}
