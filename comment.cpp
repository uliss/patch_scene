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

#include <QMenu>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

using namespace ceam;

namespace {

SharedDeviceData commentData(const QString& title)
{
    QSharedDataPointer data(new DeviceData(DEV_NULL_ID));
    data->setCategory(ItemCategory::Comment);
    data->setTitle(title);
    return data;
}
} // namespace

Comment::Comment()
    : Device(commentData(tr("Comment")))
{
}

void Comment::createContextMenu(QMenu& menu)
{
    addLockAction(menu);

    menu.addSeparator();
    addDuplicateAct(menu);
    addRemoveAct(menu);

    menu.addSeparator();
    addEditAct(menu);
}

void Comment::addEditAct(QMenu& menu)
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

void Comment::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    auto box = boundingRect();
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
    // painter->drawRect(box);
    painter->drawRoundedRect(box, 5, 5);
}
