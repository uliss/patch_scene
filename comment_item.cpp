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

    setDefaultTextColor(QColor(80, 80, 80));
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
    connect(text_, &CommentTextItem::editComment, this, [this](SceneItemId id) {
        state_ = text_->isEdited() ? EDIT : NORMAL;
        update();
        emit CommentItem::editComment(id);
    });

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

    setAcceptHoverEvents(true);
}

QRectF CommentItem::boundingRect() const
{
    return rect_;
}

void CommentItem::setEditable(bool value)
{
    QSignalBlocker sb(text_);
    text_->setEditable(value);

    if (!value && state_ != NORMAL) {
        state_ = NORMAL;
        update();
    }
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
    CommentEditor dlg(data_);
    connect(&dlg, &CommentEditor::acceptData, this, [this](const SharedItemData& data) {
        emit CommentItem::updateDevice(data);

        data_ = data;
        text_->setPlainText(data_->title());
        text_->setDefaultTextColor(data_->textColor());
        update();
    });
    dlg.exec();
}

void CommentItem::addEditAct(QMenu& menu)
{
    auto act = new QAction(tr("Edit"), &menu);

    connect(act, &QAction::triggered, this,
        [this]() {
            std::unique_ptr<CommentEditor> dialog(new CommentEditor(data_));
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
    auto wd = data_->borderWidth();
    auto bd = data_->borderColor();
    if (!bd.isValid())
        bd = QColor::fromRgbF(0.25, 0.25, 0.25);

    auto bg = data_->backgroundColor();
    if (!bg.isValid())
        bg = Qt::white;

    if (option->state & QStyle::State_Selected) {
        bd = Qt::blue;
    }

    painter->setPen(QPen(bd, wd));
    painter->setBrush(bg);
    painter->drawRect(rect_);

    if (state_ != NORMAL) {
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(Qt::cyan);

        auto x = rect_.x() + 1;
        auto y = rect_.y() + 1;
        // left top
        painter->drawLine(QLineF { x, y, x + SZ, y });
        painter->drawLine(QLineF { x, y, x, y + SZ });

        // right top
        auto r = rect_.right() - 1;
        painter->drawLine(QLineF { r - SZ, y, r, y });
        painter->drawLine(QLineF { r, y, r, y + SZ });

        // left bottom
        auto b = rect_.bottom() - 1;
        painter->drawLine(QLineF { x, b - SZ, x, b });
        painter->drawLine(QLineF { x, b, x + SZ, b });

        // right bottom
        painter->drawLine(QLineF { r, b, r, b - SZ });
        painter->drawLine(QLineF { r - SZ, b, r, b });
    }

    paintStateIcons(painter, rect_);
}

void CommentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    text_->setEditable(true);
    state_ = EDIT;
    update();
}

void CommentItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    auto x0 = rect_.left();
    auto y0 = rect_.top();
    auto x1 = rect_.right() - SZ;
    auto y1 = rect_.bottom() - SZ;

    auto pos = event->pos();

    if (QRectF { x0, y0, SZ, SZ }.contains(pos)) {
        state_ = RESIZE_LEFT_TOP;
    } else if (QRectF { x1, y0, SZ, SZ }.contains(pos)) {
        state_ = RESIZE_RIGHT_TOP;
    } else if (QRectF { x1, y1, SZ, SZ }.contains(pos)) {
        state_ = RESIZE_RIGHT_BOTTOM;
    } else if (QRectF { x0, y1, SZ, SZ }.contains(pos)) {
        state_ = RESIZE_LEFT_BOTTOM;
    } else {
        setCursor({});
        return QGraphicsObject::mousePressEvent(event);
    }

    click_pos_ = pos;
    event->accept();
}

void CommentItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    auto delta = event->pos() - click_pos_;
    auto dx = delta.x();
    auto dy = delta.y();

    switch (state_) {
    case RESIZE_LEFT_TOP: {
        setPos(pos() + delta);
        syncSize(-dx, -dy);
    } break;
    case RESIZE_RIGHT_BOTTOM: {
        syncSize(dx, dy);
        click_pos_ = event->pos();
    } break;
    case RESIZE_RIGHT_TOP: {
        setY(y() + dy);
        syncSize(dx, -dy);
        click_pos_.rx() = event->pos().x();
    } break;
    case RESIZE_LEFT_BOTTOM: {
        setX(x() + dx);
        syncSize(-dx, dy);
        click_pos_.ry() = event->pos().y();
    } break;
    case NORMAL:
    default:
        QGraphicsObject::mouseMoveEvent(event);
        break;
    }
}

void CommentItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (state_ != NORMAL) {
        unsetCursor();
        rect_ = rect_.normalized();
        event->accept();
        update();
    }
}

void CommentItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    Q_UNUSED(event);
}

void CommentItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    if (state_ == NORMAL)
        return;

    auto l = rect_.x();
    auto t = rect_.y();
    auto r = rect_.right() - SZ;
    auto b = rect_.bottom() - SZ;

    const auto pos = event->pos();

    if (QRectF { l, t, SZ, SZ }.contains(pos)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (QRectF { r, t, SZ, SZ }.contains(pos)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (QRectF { r, b, SZ, SZ }.contains(pos)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (QRectF { l, b, SZ, SZ }.contains(pos)) {
        setCursor(Qt::SizeBDiagCursor);
    } else {
        unsetCursor();
    }
}

void CommentItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    unsetCursor();
    Q_UNUSED(event);
}

void CommentItem::syncSize(qreal dw, qreal dh)
{
    constexpr auto INDENT = SZ * 2;
    constexpr auto MIN_W = SZ * 6;
    constexpr auto MIN_H = INDENT * 2;

    prepareGeometryChange();
    auto new_wd = qMax(MIN_W, rect_.width() + dw);

    {
        rect_.setWidth(new_wd);
        QSignalBlocker sb(text_);
        text_->setTextWidth(new_wd - INDENT);
    }

    auto new_ht = qMax(MIN_H, rect_.height() + dh);
    new_ht = qMax(new_ht, text_->boundingRect().height() + INDENT);
    rect_.setHeight(new_ht);
    update();
}
