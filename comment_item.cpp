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
#include "logging.hpp"

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

CommentTextItem::CommentTextItem(QGraphicsItem* parent)
    : QGraphicsTextItem(parent)
{
    auto opt = document()->defaultTextOption();
    opt.setAlignment(Qt::AlignLeft);
    document()->setDefaultTextOption(opt);

    setDefaultTextColor(Qt::gray);
}

void CommentTextItem::setEditable(bool value)
{
    if (value) {
        setTextInteractionFlags(Qt::TextEditorInteraction);
        setFocus(Qt::MouseFocusReason);
        auto cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);

        static_assert(std::is_base_of<QGraphicsItem, CommentItem>::value, "");
        auto parent = static_cast<CommentItem*>(parentItem());
        emit editComment(parent->id());
    } else {
        setTextInteractionFlags(Qt::NoTextInteraction);
        auto cursor = textCursor();
        cursor.clearSelection();
        setTextCursor(cursor);
        emit editComment(SCENE_ITEM_NULL_ID);
    }
}

bool CommentTextItem::isEdited() const
{
    return textInteractionFlags().testFlags(Qt::TextEditorInteraction);
}

void CommentTextItem::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape
        || (event->key() == Qt::Key_Return
            && event->modifiers().testFlag(Qt::ControlModifier))) {
        setEditable(false);
    } else {
        QGraphicsTextItem::keyPressEvent(event);
    }

    return event->accept();
}

void CommentTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!isEdited()) {
        setEditable(true);
        event->accept();
    } else {
        // allow select words on double click
        QGraphicsTextItem::mouseDoubleClickEvent(event);
    }
}

CommentItem::CommentItem()
    : SceneItem(ItemData::makeComment(tr("Comment")))
    , text_(new CommentTextItem(this))
{
    text_->setPlainText(data_->title());
    connect(text_, &CommentTextItem::editComment, this, &CommentItem::editComment);

    auto x = text_->boundingRect().width() * 0.5;
    auto h = text_->boundingRect().height();
    text_->setPos(-x, 0);

    prepareGeometryChange();
    rect_ = childrenBoundingRect().adjusted(-SZ, -SZ, SZ, SZ);
    rect_.moveLeft(-x - SZ);

    connect(text_->document(), &QTextDocument::contentsChanged, this, [this]() {
        prepareGeometryChange();
        rect_ = childrenBoundingRect().adjusted(-SZ, -SZ, SZ, SZ);
    });

    connect(text_->document(), &QTextDocument::contentsChanged, this, [this]() {
        data_->setTitle(text_->document()->toPlainText());
    });
}

QRectF CommentItem::boundingRect() const
{
    return rect_;
}

void CommentItem::setEditable(bool value)
{
    QSignalBlocker sb(text_);
    text_->setEditable(value);
}

bool CommentItem::isEdited() const
{
    return text_->isEdited();
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

void CommentItem::showEditDialog()
{
    CommentEditor dlg(itemData());
    connect(&dlg, &CommentEditor::acceptData, this, [this](const SharedItemData& data) {
        emit CommentItem::updateDevice(data);
        data_ = data;
        text_->setPlainText(data_->title());
    });
    dlg.exec();
}

void CommentItem::addEditAct(QMenu& menu)
{
    auto act = new QAction(tr("Edit"), &menu);

    connect(act, &QAction::triggered, this,
        [this]() {
            std::unique_ptr<CommentEditor> dialog(new CommentEditor(itemData()));
            connect(dialog.get(), &CommentEditor::acceptData, this, &CommentItem::updateDevice);
            dialog->exec();
        });

    menu.addAction(act);

    auto props = new QAction(tr("Properties"), &menu);
    connect(props, &QAction::triggered, this, [this]() { showEditDialog(); });
    menu.addAction(props);
}

void CommentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto wd = itemData()->borderWidth();
    auto bd = itemData()->borderColor();
    if (!bd.isValid())
        bd = QColor::fromRgbF(0.25, 0.25, 0.25);

    auto bg = itemData()->backgroundColor();
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
}

void CommentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    text_->setEditable(true);
}

void CommentItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    auto w = rect_.width();
    auto h = rect_.height();
    auto y = rect_.top();
    auto x = rect_.left();

    if (state_ == NORMAL && event->modifiers().testFlag(Qt::AltModifier)) {
        auto pos = event->pos();

        if (QRectF { x, y, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_LEFT_TOP;
        } else if (QRectF { rect_.right() - SZ, y, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_RIGHT_TOP;
        } else if (QRectF { rect_.right() - SZ, rect_.bottom() - SZ, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_RIGHT_BOTTOM;
        } else if (QRectF { x, rect_.bottom() - SZ, SZ, SZ }.contains(pos)) {
            state_ = RESIZE_LEFT_BOTTOM;
        } else {
            return QGraphicsItem::mousePressEvent(event);
        }

        click_pos_ = pos;
        event->accept();
        update();
    } else {
        QGraphicsItem::mousePressEvent(event);
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
        rect_ = rect_.normalized();
        event->accept();
        update();
    }
}
