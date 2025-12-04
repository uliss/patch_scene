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
#include "diagram_state_machine.h"

#include <QDebug>

using namespace ceam;

namespace {
const char* toString(DiagramState st)
{
    switch (st) {
    case DiagramState::Init:
        return "[INIT]";
    case DiagramState::MoveItem:
        return "[MOVE_ITEM]";
    case DiagramState::ConnectDevice:
        return "[CONNECT_ITEM]";
    case DiagramState::SelectItem:
        return "[SELECT_ITEM]";
    case DiagramState::SelectionRect:
        return "[SELECT_RECT]";
    case DiagramState::EditConnection:
        return "[CONNECT_EDIT]";
    case DiagramState::EditComment:
        return "[CONNECT_COMMENT]";
    default:
        return "[?]";
    }
}
} // namespace

DiagramStateMachine::DiagramStateMachine() { }

void DiagramStateMachine::setState(DiagramState state)
{
    qDebug() << toString(state_) << "->" << toString(state);
    state_ = state;
}

QDebug operator<<(QDebug debug, const ceam::DiagramStateMachine& x)
{
    debug << toString(x.state());
    return debug;
}
