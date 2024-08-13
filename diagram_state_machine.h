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
#ifndef DIAGRAM_STATE_MACHINE_H
#define DIAGRAM_STATE_MACHINE_H

enum class DiagramState {
    Init,
    Move,
    ConnectDevice,
    SelectDevice,
    SelectionRect,
};

class DiagramStateMachine {
    DiagramState state_ { DiagramState::Init };

public:
    DiagramStateMachine();
    DiagramState state() const { return state_; }
    void setState(DiagramState state);
};

#endif // DIAGRAM_STATE_MACHINE_H
