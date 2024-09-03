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
#include "about_shortcuts.h"
#include "ui_about_shortcuts.h"

namespace {

QKeySequence stripModifiers(const QKeySequence& seq)
{
    QKeyCombination keys[4];

    for (int i = 0; i < seq.count(); i++) {
        auto mods = seq[i].keyboardModifiers();
        if (mods != Qt::NoModifier)
            keys[i] = QKeyCombination(seq[i].key());
        else
            keys[i] = seq[i];
    }

    switch (seq.count()) {
    case 1:
        return QKeySequence(keys[0]);
    case 2:
        return QKeySequence(keys[0], keys[1]);
    case 3:
        return QKeySequence(keys[0], keys[1], keys[2]);
    case 4:
        return QKeySequence(keys[0], keys[1], keys[2], keys[3]);
    default:
        return seq;
    }
}

QSet<QKeySequence> getModifiers(const QKeySequence& seq)
{
    QSet<QKeySequence> res;

    for (int i = 0; i < seq.count(); i++) {
        if (seq[i] != QKeyCombination::fromCombined(0)) {
            auto mods = seq[i].keyboardModifiers();
            if (mods != Qt::NoModifier) {
                QKeySequence x(mods);
                res << x;
            }
        }
    }

    return res;
}

QString toString(const QList<QKeySequence>& seq)
{
    QSet<QKeySequence> mods;
    QString keys_txt;
    for (auto& x : seq) {
        mods |= getModifiers(x);

#ifdef Q_OS_WINDOWS
        if (!keys_txt.isEmpty())
            keys_txt += ' ';
#endif

        keys_txt += stripModifiers(x).toString(QKeySequence::NativeText);
    }

    QString seq_txt;
    for (auto& mod : std::as_const(mods))
        seq_txt += mod.toString(QKeySequence::NativeText);

#ifdef Q_OS_DARWIN
    if (!seq_txt.isEmpty())
        seq_txt += " ";
#endif

    seq_txt += keys_txt;
    return seq_txt;
}

}

namespace ceam {

AboutShortcuts::AboutShortcuts(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutShortcuts)
{
    ui->setupUi(this);

    ui->tableWidget->setAlternatingRowColors(false);
    ui->tableWidget->setWordWrap(true);

    setStyleSheet(R"(
QDialog            { background-color: white; }
QTableWidget       { background-color: none; border: none; gridline-color: white; }
QTableWidget::item { background-color: none; border-bottom: 1px solid #EEE; }
)");

    addSection(tr("General"));
    addEntry(tr("Copy"), QKeySequence::Copy);
    addEntry(tr("Paste"), QKeySequence::Paste);
    addEntry(tr("Cut"), QKeySequence::Cut);
    addEntry(tr("Duplicate"), QKeySequence(Qt::CTRL | Qt::Key_D));
    addEntry(tr("Select All"), QKeySequence::SelectAll);
    addEntry(tr("Select item"), QKeySequence(), AdditonalActs::Click);
    addEntry(tr("Add item to selection"), QKeySequence(Qt::SHIFT), AdditonalActs::Click);
    addEntry(tr("Toggle item selection"), QKeySequence(Qt::CTRL), AdditonalActs::Click);
    addEntry(tr("Delete"), QKeySequence(Qt::CTRL | Qt::Key_Backspace));
    addEntry(tr("Undo"), QKeySequence::Undo);
    addEntry(tr("Redo"), QKeySequence::Redo);

    addSection(tr("Navigation"));
    addEntry(tr("Move items"), QKeySequence(Qt::SHIFT), AdditonalActs::Drag);
    addEntry(tr("Move items") //
        ,
        {
            QKeySequence(Qt::Key_Left),
            QKeySequence(Qt::Key_Right),
            QKeySequence(Qt::Key_Up),
            QKeySequence(Qt::Key_Down),
        });

    addEntry(tr("Move items with step") //
        ,
        {
            QKeySequence(Qt::SHIFT | Qt::Key_Left),
            QKeySequence(Qt::SHIFT | Qt::Key_Right),
            QKeySequence(Qt::SHIFT | Qt::Key_Up),
            QKeySequence(Qt::SHIFT | Qt::Key_Down),
        });
    addEntry(tr("Move items with big step") //
        ,
        {
            QKeySequence(Qt::CTRL | Qt::Key_Left),
            QKeySequence(Qt::CTRL | Qt::Key_Right),
            QKeySequence(Qt::CTRL | Qt::Key_Up),
            QKeySequence(Qt::CTRL | Qt::Key_Down),
        });

    addEntry(tr("Zoom in"), QKeySequence::ZoomIn);
    addEntry(tr("Zoom out"), QKeySequence::ZoomOut);
    addEntry(tr("Zoom to 100%"), QKeySequence(Qt::CTRL | Qt::Key_0));
    addEntry(tr("Zoom to fit"), QKeySequence(Qt::ALT | Qt::Key_1));
    addEntry(tr("Zoom to selected"), QKeySequence(Qt::ALT | Qt::Key_2));
    addEntry(tr("Toggle background"), QKeySequence(Qt::CTRL | Qt::Key_B));
    addEntry(tr("Toggle connections"), QKeySequence(Qt::CTRL | Qt::Key_K));
    addEntry(tr("Toggle grid"), QKeySequence(Qt::CTRL | Qt::Key_G));

    addSection(tr("Connections"));
    addEntry(tr("Disconnect"), QKeySequence(Qt::ALT), AdditonalActs::Click);
    addEntry(tr("Reconnect to next"), QKeySequence(Qt::SHIFT), AdditonalActs::Drag);

    ui->tableWidget->resizeRowsToContents();
    adjustSize();
    setMinimumHeight(this->height());
}

AboutShortcuts::~AboutShortcuts()
{
    delete ui;
}

void AboutShortcuts::addEntry(const QString& txt, const QKeySequence& seq, AdditonalActs acts)
{
    auto seq_txt = seq.toString(QKeySequence::NativeText);

    switch (acts) {
    case AdditonalActs::Click:
#ifdef Q_OS_DARWIN
        if (!seq_txt.isEmpty())
            seq_txt += " + ";
#endif

        seq_txt += tr("click");
        break;
    case AdditonalActs::Drag:
#ifdef Q_OS_DARWIN
        if (!seq_txt.isEmpty())
            seq_txt += " + ";
#endif

        seq_txt += tr("drag");
        break;
    case AdditonalActs::None:
    default:
        break;
    }

    addEntry(txt, seq_txt);
}

void AboutShortcuts::addEntry(const QString& txt, const QList<QKeySequence>& seq)
{
    addEntry(txt, toString(seq));
}

void AboutShortcuts::addEntry(const QString& txt, const QString& seq)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(txt));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(seq));
}

void AboutShortcuts::addSection(const QString& name)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setSpan(row, 0, 1, 2);
    auto item = new QTableWidgetItem(name);
    auto font = item->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 2);
    item->setFont(font);
    ui->tableWidget->setItem(row, 0, item);
}

} // namespace ceam
