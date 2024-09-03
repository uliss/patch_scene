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
#ifndef CEAM_ABOUT_SHORTCUTS_H
#define CEAM_ABOUT_SHORTCUTS_H

#include <QDialog>

namespace Ui {
class AboutShortcuts;
}

namespace ceam {

class AboutShortcuts : public QDialog {
    Q_OBJECT

public:
    explicit AboutShortcuts(QWidget* parent = nullptr);
    ~AboutShortcuts();

private:
    void addEntry(const QString& txt, const QKeySequence& seq, bool click = false);
    void addEntry(const QString& txt, const QList<QKeySequence>& seq);
    void addEntry(const QString& txt, const QString& seq);

    void addSection(const QString& name);

private:
    Ui::AboutShortcuts* ui;
};

} // namespace ceam
#endif // CEAM_ABOUT_SHORTCUTS_H
