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
#include "image_widget.h"

#include <QFileInfo>
#include <QMouseEvent>

namespace {

constexpr int IMG_PREVIEW_SIZE = 30;

} // namespace

using namespace ceam;

ImageWidget::ImageWidget(QWidget* parent)
    : QLabel(parent)
{
    setStyleSheet("background-color: white;");
    setFrameShape(QFrame::Box);
    setFixedSize(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE);
    setAlignment(Qt::AlignCenter);
}

void ImageWidget::setImagePath(const QString& path)
{
    QFileInfo finfo(path);
    QIcon icon(path);
    if (!icon.isNull()) {
        setPixmap(icon.pixmap(IMG_PREVIEW_SIZE, IMG_PREVIEW_SIZE));
        setToolTip(tr("Image: %1").arg(finfo.baseName()));
    } else {
        setText("?");
        setToolTip(tr("Image: none"));
    }
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (event->modifiers().testFlag(Qt::AltModifier)) {
            setImagePath({});
        } else
            emit clicked();
    } else
        QLabel::mousePressEvent(event);
}
