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
#include "diagram_updates_blocker.h"

#include "diagram.h"
#include "diagram_scene.h"

namespace ceam {

DiagramUpdatesBlocker::DiagramUpdatesBlocker(Diagram* diagram)
    : diagram_(diagram)
{
    if (!diagram_)
        return;

    diagram_->blockSignals(true);
    diagram_->setUpdatesEnabled(false);
    view_mode_ = diagram_->viewportUpdateMode();
    diagram_->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    diagram_->scene()->blockSignals(true);
}

DiagramUpdatesBlocker::~DiagramUpdatesBlocker()
{
    if (!diagram_)
        return;

    diagram_->scene()->blockSignals(false);
    diagram_->setViewportUpdateMode(static_cast<QGraphicsView::ViewportUpdateMode>(view_mode_));
    diagram_->setUpdatesEnabled(true);
    diagram_->blockSignals(false);
}

DiagramSceneUpdatesBlocker::DiagramSceneUpdatesBlocker(DiagramScene* diagram)
    : diagram_(diagram)
{
    if (!diagram_)
        return;

    diagram_->blockSignals(true);
    diagram_->itemScene().blockSignals(true);
}

DiagramSceneUpdatesBlocker::~DiagramSceneUpdatesBlocker()
{
    if (!diagram_)
        return;

    diagram_->blockSignals(false);
    diagram_->itemScene().blockSignals(false);
}

} // namespace ceam
