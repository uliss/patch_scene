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
#include "logging.hpp"

#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

namespace {

class BezierEditorHandle : public QGraphicsItem {
    std::function<void(const QPointF& pos)> fn_;
    QPointF src_pos_;
    QPainterPath shape_;
    QRectF ellipse_;

public:
    BezierEditorHandle(const QPoint& srcPoint, const QPoint& bezyOffset,
        QGraphicsItem* parent, std::function<void(const QPointF& pos)> fn);

    QRectF handleRect() const;
    void setHandlePos(const QPointF& pos);

    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) final;
    QRectF boundingRect() const final;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) final;
};

}

namespace ceam {

ConnectionEditor::ConnectionEditor()
    : data_(0, 0, 0, 0)
{
    setZValue(10000);
}

void ConnectionEditor::setConnectionData(const ConnectionData& data)
{
    prepareGeometryChange();

    for (auto x : handles_) {
        scene()->removeItem(x);
        delete x;
    }

    data_ = data;
    handles_.clear();

    switch (data.cordType()) {
    case ConnectionCordType::Linear:
        break;
    case ConnectionCordType::Bezier: {
        auto c0 = new BezierEditorHandle(data.sourcePoint(),
            data.bezyCtlPoint0(),
            this,
            [this](const QPointF& newPos) {
                data_.setBezyCtlPoint0(newPos - data_.sourcePoint());
                emit connectionUpdated(data_);
            });
        shape_.addEllipse(c0->handleRect());
        handles_.append(c0);

        auto c1 = new BezierEditorHandle(
            data.destinationPoint(),
            data.bezyCtlPoint1(),
            this,
            [this](const QPointF& newPos) {
                data_.setBezyCtlPoint1(newPos - data_.destinationPoint());
                emit connectionUpdated(data_);
            });
        shape_.addEllipse(c1->handleRect());
        handles_.append(c1);

    } break;
    case ConnectionCordType::Segmented: {
        shape_.clear();
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
    return shape_.boundingRect();
}

QPainterPath ConnectionEditor::shape() const
{
    return shape_;
}

void ConnectionEditor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // WARN() << "2";
}

} // namespace ceam

BezierEditorHandle::BezierEditorHandle(const QPoint& srcPoint, const QPoint& bezyOffset, QGraphicsItem* parent, std::function<void(const QPointF&)> fn)
    : QGraphicsItem(parent)
    , fn_(fn)
    , src_pos_(srcPoint)
    , ellipse_ { -5, -5, 10, 10 }
{
    setToolTip("Segment point");
    setFlag(ItemIsMovable);
    setHandlePos(src_pos_ + bezyOffset);
}

QRectF BezierEditorHandle::handleRect() const
{
    return ellipse_.translated(pos());
}

void BezierEditorHandle::setHandlePos(const QPointF& pos)
{
    prepareGeometryChange();
    shape_.clear();
    shape_.addEllipse(ellipse_);
    shape_.addPolygon({ mapFromScene(src_pos_), mapFromScene(pos) });
    setPos(pos);
}

void BezierEditorHandle::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    event->accept();
    setHandlePos(event->scenePos());

    if (fn_)
        fn_(event->scenePos());

    update(boundingRect());
}

QRectF BezierEditorHandle::boundingRect() const
{
    return shape_.boundingRect();
}

void BezierEditorHandle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setPen(QPen(Qt::darkGray, 1));
    painter->drawLine({ mapFromScene(src_pos_), mapFromScene(pos()) });

    painter->setPen(QPen(Qt::darkGray, 1));
    painter->setBrush(Qt::green);
    painter->drawEllipse(ellipse_);
}
