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
#ifndef EXPORT_DOCUMENT_H
#define EXPORT_DOCUMENT_H

#include <QStandardItemModel>
#include <QTextCursor>

namespace ceam {
namespace doc {

    void insert_table(QTextCursor& cursor, const QStandardItemModel* model);
    void insert_table(QTextCursor& cursor, const QList<QStringList>& data);

    void insert_header(QTextCursor& cursor, const QString& text);
    void insert_section(QTextCursor& cursor, const QString& text);
    void insert_paragraph(QTextCursor& cursor, const QString& text, Qt::Alignment align = Qt::AlignLeft);
}
}

#endif // EXPORT_DOCUMENT_H
