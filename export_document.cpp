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
#include "export_document.h"

#include <QTextLength>
#include <QTextTable>
#include <QTextTableFormat>

void ceam::doc::insert_table(QTextCursor& cursor, const QList<QStringList>& data)
{
    if (data.isEmpty())
        return;

    auto first_line = data.front();
    auto NCOLS = first_line.size();
    auto NROWS = data.size();

    QTextTableFormat tab_fmt;
    tab_fmt.setCellPadding(4);
    tab_fmt.setBottomMargin(10);
    tab_fmt.setHeaderRowCount(1);
    tab_fmt.setCellSpacing(0);
    tab_fmt.setCellSpacing(0);

    tab_fmt.setBorderBrush(QBrush(Qt::SolidPattern));
    tab_fmt.setBorderCollapse(true);

    cursor.movePosition(QTextCursor::End);
    auto table = cursor.insertTable(NROWS, NCOLS + 1, tab_fmt);

    QTextCharFormat char_fmt;
    char_fmt.setFontPointSize(10);

    for (int row = 0; row < NROWS; ++row) {
        QString row_index;

        if (row == 0) {
            char_fmt.setFontWeight(QFont::Bold);
            row_index = "#";
        } else {
            char_fmt.setFontWeight(QFont::Normal);
            row_index = QString("%1").arg(row);
        }

        for (int col = -1; col < NCOLS; ++col) {
            auto cell = table->cellAt(row, col + 1);
            auto cell_cursor = cell.firstCursorPosition();

            if (col == -1) {
                char_fmt.setFontItalic(true);
                cell_cursor.insertText(row_index, char_fmt);
            } else {
                char_fmt.setFontItalic(false);
                cell_cursor.insertText(data[row][col], char_fmt);
            }
        }
    }

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_table(QTextCursor& cursor,
    QStandardItemModel* model,
    const QList<int>& columnContraints)
{
    auto NCOLS = model->columnCount();
    auto NROWS = model->rowCount();

    QTextTableFormat tab_fmt;
    tab_fmt.setCellPadding(4);
    tab_fmt.setBottomMargin(10);
    tab_fmt.setHeaderRowCount(1);
    tab_fmt.setCellSpacing(0);
    tab_fmt.setCellSpacing(0);

    tab_fmt.setBorderBrush(QBrush(Qt::SolidPattern));
    tab_fmt.setBorderCollapse(true);

    QList<QTextLength> col_constraints;
    col_constraints.push_back(QTextLength { QTextLength::FixedLength, 20 });

    for (auto w : columnContraints)
        col_constraints.push_back(QTextLength { QTextLength::PercentageLength, qreal(w) });

    qDebug() << col_constraints;

    tab_fmt.setColumnWidthConstraints(col_constraints);

    cursor.movePosition(QTextCursor::End);
    auto table = cursor.insertTable(1, NCOLS + 1, tab_fmt);

    for (int col = -1; col < NCOLS; col++) {
        auto cell = table->cellAt(0, col + 1);
        auto cell_cursor = cell.firstCursorPosition();
        auto fmt = cell.format();
        fmt.setFontWeight(QFont::Bold);
        cell.setFormat(fmt);

        auto item = model->horizontalHeaderItem(col);
        if (item) {
            cell_cursor.insertText(item->text());
        } else {
            // cell_cursor.insertText("№");
        }
    }

    table->appendRows(NROWS);
    for (int row = 0; row < NROWS; ++row) {
        for (int col = -1; col < NCOLS; ++col) {
            auto cell = table->cellAt(row + 1, col + 1);
            auto cell_cursor = cell.firstCursorPosition();

            if (col == -1) {
                cell_cursor.insertText(QString("%1").arg(row + 1));
            } else {
                auto item = model->item(row, col);
                if (item)
                    cell_cursor.insertText(item->text());
            }
        }
    }

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_section(QTextCursor& cursor, const QString& text)
{
    QTextBlockFormat block_fmt;
    block_fmt.setTopMargin(5);
    block_fmt.setBottomMargin(5);
    block_fmt.setBackground(QColor("#F0F0F0"));

    QTextCharFormat char_fmt;
    char_fmt.setFontWeight(QFont::Bold);
    char_fmt.setFontPointSize(12);

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_fmt);
    cursor.insertText(text, char_fmt);

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_paragraph(QTextCursor& cursor, const QString& text, Qt::Alignment align)
{
    QTextBlockFormat block_fmt;
    block_fmt.setAlignment(align);
    block_fmt.setTopMargin(5);
    block_fmt.setLeftMargin(20);
    block_fmt.setBottomMargin(5);

    QTextCharFormat char_fmt;
    char_fmt.setFontWeight(QFont::Normal);
    char_fmt.setFontPointSize(12);

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_fmt);
    cursor.insertText(text, char_fmt);

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_header(QTextCursor& cursor, const QString& text)
{
    QTextBlockFormat block_fmt;
    block_fmt.setTopMargin(10);
    block_fmt.setBottomMargin(10);

    QTextCharFormat char_fmt;
    char_fmt.setFontWeight(QFont::Bold);
    char_fmt.setFontPointSize(24);

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_fmt);
    cursor.insertText(text, char_fmt);

    cursor.movePosition(QTextCursor::End);
}
