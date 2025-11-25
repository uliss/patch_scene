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
#include "comment.h"
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

SharedDeviceData commentData(const QString& title)
{
    QSharedDataPointer data(new DeviceData(SCENE_ITEM_NULL_ID));
    data->setCategory(ItemCategory::Comment);
    data->setTitle(title);
    return data;
}
} // namespace

CommentItem::CommentItem()
    : SceneItem(commentData(tr("Comment")))
    , text_(new QGraphicsTextItem(this))
{
    auto opt = text_->document()->defaultTextOption();
    opt.setAlignment(Qt::AlignLeft);
    text_->document()->setDefaultTextOption(opt);

    text_->setPlainText(data_->title());
    // text_->setTextInteractionFlags(Qt::TextEditorInteraction);
    text_->setDefaultTextColor(Qt::red);
    text_->setPos(-text_->boundingRect().width() * 0.5, 0);

    // connect(title()->document(), &QTextDocument::documentLayoutChanged, this, [this]() {
    //     // syncRect();
    //     qWarning() << "CHNAGE";
    // });
}

QRectF CommentItem::boundingRect() const
{
    // qWarning() << "bb: " << childrenBoundingRect();
    return childrenBoundingRect();
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
            connect(dialog.get(), SIGNAL(acceptData(SharedDeviceData)), this, SIGNAL(updateDevice(SharedDeviceData)));
            dialog->exec();
        });

    menu.addAction(act);
}

void CommentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto box = childrenBoundingRect();
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
    painter->drawRect(box);
    // painter->drawRoundedRect(box, 5, 5);
}

void CommentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (!text_->textInteractionFlags().testFlags(Qt::TextEditorInteraction)) {
        text_->setTextInteractionFlags(Qt::TextEditorInteraction);
        event->accept();
    } else
        SceneItem::mouseDoubleClickEvent(event);
}

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
