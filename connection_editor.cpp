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
#include "connection_editor.h"
#include "bezier_editor_handle.h"

#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace ceam {

ConnectionEditor::ConnectionEditor()
    : id_(0, 0, 0, 0)
{
    setZValue(10000);
}

void ConnectionEditor::setConnectionData(const ConnectionId& id, const ConnectionViewData& viewData)
{
    prepareGeometryChange();

    for (auto x : handles_) {
        scene()->removeItem(x);
        delete x;
    }

    id_ = id;
    view_data_ = viewData;
    handles_.clear();

    switch (view_data_.cordType()) {
    case ConnectionCordType::Linear:
        break;
    case ConnectionCordType::Bezier: {
        auto h0 = new BezierEditorHandle(view_data_.sourcePoint(),
            view_data_.bezyCtlPoint0(),
            this,
            [this](const QPointF& newPos) {
                view_data_.setBezyCtlPoint0(newPos - view_data_.sourcePoint());
                emit connectionUpdated(id_, view_data_);
            });
        handles_.append(h0);

        auto h1 = new BezierEditorHandle(
            view_data_.destinationPoint(),
            view_data_.bezyCtlPoint1(),
            this,
            [this](const QPointF& newPos) {
                view_data_.setBezyCtlPoint1(newPos - view_data_.destinationPoint());
                emit connectionUpdated(id_, view_data_);
            });
        handles_.append(h1);

    } break;
    case ConnectionCordType::Segmented: {
        // WARN() << data.segmentPoints();
        // for (auto& pt : data.segments()) {
        //     auto c = new SegmentHandle(pt, this);
        //     auto r = c->rect().translated(pt);
        //     shape_.addEllipse(r);
        //     handles_.append(c);
        // }
    } break;
    }
}

QRectF ConnectionEditor::boundingRect() const
{
    return childrenBoundingRect();
}

void ConnectionEditor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // WARN() << "2";
}

} // namespace ceam
