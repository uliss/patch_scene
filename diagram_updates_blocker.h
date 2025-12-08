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
#ifndef DIAGRAM_UPDATES_BLOCKER_H
#define DIAGRAM_UPDATES_BLOCKER_H

namespace ceam {

class Diagram;
class DiagramScene;

class DiagramUpdatesBlocker {
    Diagram* diagram_ { nullptr };
    int view_mode_ { 0 };

public:
    explicit DiagramUpdatesBlocker(Diagram* diagram);
    ~DiagramUpdatesBlocker();
};

class DiagramSceneUpdatesBlocker {
    DiagramScene* diagram_ { nullptr };

public:
    explicit DiagramSceneUpdatesBlocker(DiagramScene* diagram);
    ~DiagramSceneUpdatesBlocker();
};

} // namespace ceam

#endif // DIAGRAM_UPDATES_BLOCKER_H
