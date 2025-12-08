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

#include <QTextDocument>
#include <QTextLength>
#include <QTextTable>
#include <QTextTableFormat>

namespace {
constexpr int DEFAULT_FONT_SIZE = 10;
constexpr int PAR_FONT_SIZE = DEFAULT_FONT_SIZE;
constexpr int TAB_FONT_SIZE = DEFAULT_FONT_SIZE;
constexpr int SECTION_FONT_SIZE = DEFAULT_FONT_SIZE * 1.5;
constexpr int HEADER_FONT_SIZE = DEFAULT_FONT_SIZE * 2;

static QTextCharFormat make_char_fmt(int size, bool bold = false, bool italic = false)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    fmt.setFontPointSize(size);
    fmt.setFontItalic(italic);
    return fmt;
}

QTextBlockFormat make_block_fmt(Qt::Alignment align = Qt::AlignLeft)
{
    QTextBlockFormat fmt;

    fmt.setAlignment(align);
    fmt.setTopMargin(5);
    fmt.setLeftMargin(20);
    fmt.setBottomMargin(5);

    return fmt;
}

QTextTableFormat make_table_fmt(int width)
{
    constexpr int LEFT_MARGIN = 40;
    constexpr int NUM_COL_WIDTH = 35;
    QTextTableFormat fmt;

    fmt.setCellPadding(3);
    fmt.setTopMargin(10);
    fmt.setBottomMargin(10);
    fmt.setHeaderRowCount(1);
    fmt.setCellSpacing(0);
    fmt.setLeftMargin(LEFT_MARGIN);
    fmt.setWidth(width);

    fmt.setBorderBrush(Qt::lightGray);
    fmt.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    fmt.setBorderCollapse(true);
    fmt.setBorder(1);

    fmt.setColumnWidthConstraints({ QTextLength { QTextLength::FixedLength, NUM_COL_WIDTH } });

    return fmt;
}

QSizeF doc_page_size(const QTextCursor& cursor)
{
    auto doc = cursor.document();
    if (!doc)
        return {};

    auto m = doc->documentMargin();
    return doc->pageSize().shrunkBy({ m, m, m, m });
}
} // namespace

void ceam::doc::insert_table(QTextCursor& cursor, const QList<QStringList>& data)
{
    if (data.isEmpty() || cursor.isNull())
        return;

    auto& first_line = data.front();
    auto NCOLS = first_line.size();
    auto NROWS = data.size();

    cursor.movePosition(QTextCursor::End);

    auto doc_margin = cursor.document()->documentMargin();
    auto doc_rect = cursor.document()->pageSize().shrunkBy({ doc_margin, doc_margin, doc_margin, doc_margin });

    auto table = cursor.insertTable(NROWS, NCOLS + 1, make_table_fmt(doc_rect.width()));

    for (int row = 0; row < NROWS; ++row) {
        const bool is_header = (row == 0);

        for (int col = 0; col < NCOLS + 1; ++col) {
            auto cell = table->cellAt(row, col);
            if (!cell.isValid()) {
                qWarning() << "invalid cell:" << row << col;
                continue;
            }

            auto cell_fmt = cell.format();
            cell_fmt.setBackground(row % 2 == 0 ? QColor(240, 240, 240) : Qt::white);
            cell.setFormat(cell_fmt);

            auto tab_cursor = cell.firstCursorPosition();
            if (tab_cursor.isNull()) {
                qWarning() << "invalid cursor:" << row << col;
                continue;
            }

            if (col == 0) {
                QString row_index = is_header ? "#" : QString("%1").arg(row);
                tab_cursor.insertText(row_index, make_char_fmt(TAB_FONT_SIZE, is_header, true));
            } else {
                if (row < data.size() && (col - 1) < data[row].size())
                    tab_cursor.insertText(data[row][col - 1], make_char_fmt(TAB_FONT_SIZE, is_header, false));
            }
        }
    }

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_table(QTextCursor& cursor, const QStandardItemModel* model)
{
    if (!model)
        return;

    auto NCOLS = model->columnCount();
    auto NROWS = model->rowCount();

    QList<QStringList> data;

    QStringList header_data;
    header_data.resize(NCOLS);
    for (int col = 0; col < header_data.size(); ++col) {
        auto header = model->horizontalHeaderItem(col);
        if (header)
            header_data[col] = header->text();
    }
    data.push_back(header_data);

    QStringList data_row;
    for (int row = 0; row < NROWS; ++row) {
        data_row.clear();
        for (int col = 0; col < NCOLS; ++col) {
            auto item = model->item(row, col);
            if (item)
                data_row.push_back(item->text());
        }
        data.push_back(data_row);
    }

    insert_table(cursor, data);
}

void ceam::doc::insert_section(QTextCursor& cursor, const QString& text)
{
    auto block_fmt = make_block_fmt();
    block_fmt.setLeftMargin(0);
    block_fmt.setTopMargin(5);
    block_fmt.setBottomMargin(5);
    block_fmt.setBackground(QColor("#F0F0F0"));

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock(block_fmt);
    cursor.insertText(text, make_char_fmt(SECTION_FONT_SIZE, false));

    QTextFrameFormat line;
    line.setHeight(1.5);
    line.setWidth(doc_page_size(cursor).width());
    line.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    line.setBorderBrush(Qt::black);
    line.setBorder(0.5);
    cursor.insertFrame(line);

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_paragraph(QTextCursor& cursor, const QString& text, Qt::Alignment align)
{
    auto block_fmt = make_block_fmt(align);
    cursor.movePosition(QTextCursor::End);

    cursor.insertBlock(block_fmt);
    cursor.insertText(text, make_char_fmt(PAR_FONT_SIZE, false));

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_header(QTextCursor& cursor, const QString& text)
{
    QTextBlockFormat block_fmt;
    block_fmt.setTopMargin(10);
    block_fmt.setBottomMargin(10);

    cursor.insertBlock(block_fmt);
    cursor.insertText(text, make_char_fmt(HEADER_FONT_SIZE, true));

    cursor.movePosition(QTextCursor::End);
}

void ceam::doc::insert_image(QTextCursor& cursor, const QImage& img)
{
    if (img.isNull())
        return;

    auto doc = cursor.document();
    if (!doc)
        return;

    static int image_counter = 1;
    auto res_id = QString("mydata://image-%1.png").arg(image_counter++);
    doc->addResource(QTextDocument::ImageResource, res_id, QVariant(img));

    QTextImageFormat img_fmt;
    auto page_size = doc_page_size(cursor);
    img_fmt.setName(res_id);
    img_fmt.setWidth(page_size.width());
    img_fmt.setHeight(page_size.width() * img.height() / img.width());
    cursor.insertImage(img_fmt);
}

void ceam::doc::insert_svg_image(QTextCursor& cursor, const std::pair<QByteArray, QSize>& svg_data)
{
    if (cursor.isNull())
        return;

    auto doc = cursor.document();
    if (!doc)
        return;

    static int image_counter = 1;
    auto res_id = QString("mydata://image-%1.svg").arg(image_counter++);
    doc->addResource(QTextDocument::ImageResource, res_id, QVariant(svg_data.first));

    QTextImageFormat img_fmt;
    auto page_size = doc_page_size(cursor);
    auto svg_size = svg_data.second;
    img_fmt.setName(res_id);
    img_fmt.setWidth(page_size.width());
    img_fmt.setHeight(page_size.width() * svg_size.height() / qreal(svg_size.width()));
    cursor.insertImage(img_fmt);
}
