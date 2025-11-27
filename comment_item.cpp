/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
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
#include "comment_item.h"
#include "comment_editor.h"

#include <QDebug>
#include <QGraphicsSceneEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTextCursor>
#include <QTextDocument>

using namespace ceam;

namespace {

constexpr qreal SZ = 8;

} // namespace

CommentItem::CommentItem()
    : SceneItem(ItemData::makeComment(tr("Comment")))
    , text_(new QGraphicsTextItem(this))
{
    auto opt = text_->document()->defaultTextOption();
    opt.setAlignment(Qt::AlignLeft);
    text_->document()->setDefaultTextOption(opt);

    text_->setPlainText(data_->title());
    // text_->setTextInteractionFlags(Qt::TextEditorInteraction);
    text_->setDefaultTextColor(Qt::red);

    auto x = text_->boundingRect().width() * 0.5;
    auto h = text_->boundingRect().height();
    text_->setPos(-x, 0);

    prepareGeometryChange();
    rect_ = text_->boundingRect().adjusted(-SZ, -SZ, SZ, SZ);
    rect_.moveLeft(-x - SZ);

    // connect(title()->document(), &QTextDocument::documentLayoutChanged, this, [this]() {
    //     // syncRect();
    //     qWarning() << "CHNAGE";
    // });
}

QRectF CommentItem::boundingRect() const
{
    // qWarning() << "bb: " << childrenBoundingRect();
    return rect_;
}

void CommentItem::createContextMenu(QMenu& menu)
{
    addLockAction(menu);

    menu.addSeparator();
    addDuplicateAct(menu);
    addRemoveAct(menu);

    menu.addSeparator();
    addEditAct(menu);
}

void CommentItem::addEditAct(QMenu& menu)
{
    auto act = new QAction(tr("Edit"), &menu);

    connect(act, &QAction::triggered, this,
        [this]() {
            std::unique_ptr<CommentEditor> dialog(new CommentEditor(deviceData()));
            connect(dialog.get(), &CommentEditor::acceptData, this, &CommentItem::updateDevice);
            dialog->exec();
        });

    menu.addAction(act);
}

void CommentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto wd = deviceData()->borderWidth();
    auto bd = deviceData()->borderColor();
    if (!bd.isValid())
        bd = QColor::fromRgbF(0.25, 0.25, 0.25);

    auto bg = deviceData()->backgroundColor();
    if (!bg.isValid())
        bg = Qt::white;

    if (option->state & QStyle::State_Selected) {
        bd = Qt::blue;
    }

    painter->setPen(QPen(bd, wd));
    painter->setBrush(bg);
    painter->drawRect(rect_);

    if (state_ != NORMAL) {
        painter->setPen(QPen(Qt::black, 0));
        painter->setBrush(Qt::cyan);

        painter->drawRect(QRectF { rect_.topLeft(), QSizeF { SZ, SZ } });
        painter->drawRect(QRectF { rect_.topRight() - QPointF { SZ, 0 }, QSizeF { SZ, SZ } });
        painter->drawRect(QRectF { rect_.bottomRight() - QPointF { SZ, SZ }, QSizeF { SZ, SZ } });
        painter->drawRect(QRectF { rect_.bottomLeft() - QPointF { 0, SZ }, QSizeF { SZ, SZ } });
    }

    // painter->drawRoundedRect(box, 5, 5);
}

// void CommentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
// {
//     if (!text_->textInteractionFlags().testFlags(Qt::TextEditorInteraction)) {
//         text_->setTextInteractionFlags(Qt::TextEditorInteraction);
//         event->accept();
//     } else
//         SceneItem::mouseDoubleClickEvent(event);
// }

void CommentItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape
        || (event->key() == Qt::Key_Enter && event->modifiers().testFlag(Qt::ControlModifier))) {
        text_->setTextInteractionFlags(Qt::NoTextInteraction);
        auto cursor = text_->textCursor();
        cursor.movePosition(QTextCursor::End);
        text_->setTextCursor(cursor);
        // qWarning() << "new text: " << title()->document()->toPlainText();
        deviceData()->setTitle(text_->document()->toPlainText());
        event->accept();
    } else {
        SceneItem::keyPressEvent(event);
    }
}

void CommentItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    auto w = rect_.width();
    auto h = rect_.height();
    auto y = rect_.top();
    auto x = rect_.left();

    switch (state_) {
    case NORMAL: {
        auto pos = event->pos();
        qWarning() << "press: " << pos << ", rect: " << rect_;

        if (QRectF { x, y, SZ, SZ }.contains(pos)) {
            qWarning() << "left-top";
            state_ = RESIZE_LEFT_TOP;
        } else if (QRectF { rect_.right() - SZ, y, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_RIGHT_TOP;
        } else if (QRectF { rect_.right() - SZ, rect_.bottom() - SZ, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_RIGHT_BOTTOM;
        } else if (QRectF { x, rect_.bottom() - SZ, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_LEFT_BOTTOM;
        } else {
            QGraphicsItem::mousePressEvent(event);
            return;
        }

        qWarning() << "state: " << state_;

        click_pos_ = pos;
        event->accept();
        update();
    } break;
    default:
        QGraphicsItem::mousePressEvent(event);
        break;
    }
}

void CommentItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto delta = event->pos() - click_pos_;

    switch (state_) {
    case RESIZE_LEFT_TOP: {
        qWarning() << "resize left-top";
        prepareGeometryChange();
        setPos(pos() + delta);
        rect_.setHeight(rect_.height() - delta.y());
        rect_.setWidth(rect_.width() - delta.x());
        text_->setTextWidth(rect_.width() - 2 * SZ);
        update();
    } break;
    case RESIZE_RIGHT_BOTTOM: {
        qWarning() << "resize right-bottom";
        prepareGeometryChange();
        rect_.setBottomRight(event->pos());
        text_->setTextWidth(rect_.width() - 2 * SZ);
        update();
    } break;
    case RESIZE_RIGHT_TOP: {
        prepareGeometryChange();
        setY(y() + delta.y());
        rect_.setRight(event->pos().x());
        rect_.setHeight(rect_.height() - delta.y());
        text_->setTextWidth(rect_.width() - 2 * SZ);
        update();
    } break;
    case RESIZE_LEFT_BOTTOM: {
        prepareGeometryChange();
        setX(x() + delta.x());
        rect_.setWidth(rect_.width() - delta.x());
        rect_.setBottom(event->pos().y());
        text_->setTextWidth(rect_.width() - 2 * SZ);
        update();
    } break;
    case NORMAL:
    default:
        break;
    }
}

void CommentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (state_ != NORMAL) {
        state_ = NORMAL;
        event->accept();
        update();
    }
}
