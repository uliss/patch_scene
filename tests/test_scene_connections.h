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
#ifndef TEST_SCENE_CONNECTIONS_H
#define TEST_SCENE_CONNECTIONS_H

#include <QObject>

class TestSceneConnections : public QObject {
    Q_OBJECT

private slots:
    void add();
    void remove();
    void clear();
    void removeAll();
    void findConnection();
    void findConnections();
    void setVisible();
    void checkConnection();
};

#endif // TEST_SCENE_CONNECTIONS_H
