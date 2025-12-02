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
#ifndef TEST_DIAGRAM_H
#define TEST_DIAGRAM_H

#include <QObject>

class TestDiagram : public QObject {
    Q_OBJECT

private slots:
    void addItem();
    void cmdPlaceInColumnSelected();
    void cmdPlaceInRowSelected();
    void duplicateSelected();
    void mousePress();
    void mouseSelect();
    void moveSelectedBy();
    void moveSelectedFrom();
    void removeItem();
};

#endif // TEST_DIAGRAM_H
