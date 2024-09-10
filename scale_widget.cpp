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
#include "scale_widget.h"

#include <QPainter>

using namespace ceam;

ScaleWidget::ScaleWidget(QWidget* parent)
    : QWidget { parent }
    , length_(200)
    , scale_(1)
{
    setGeometry(0, 0, 220, 20);
}

void ScaleWidget::setPos(const QPoint& pos)
{
    auto size = geometry().size();
    setGeometry(pos.x(), pos.y(), size.width(), size.height());
}

void ScaleWidget::setScale(qreal scale)
{
    scale_ = scale;
    // repaint();
    update();
}

void ScaleWidget::paintEvent(QPaintEvent* event)
{
    constexpr auto RULER_H = 5;
    constexpr auto RULER_Y = 1;
    constexpr auto TXT_H = 20;
    constexpr auto TXT_W = 40;
    constexpr auto TXT_Y = RULER_H + RULER_Y + 2;

    QPainter painter(this);
    painter.setFont(font());

    QSizeF size = geometry().size();
    size.rwidth() -= 2;

    auto ft = font();
    ft.setPointSize(ft.pointSize() * 0.75);
    painter.setFont(ft);
    QTextOption txt_opts(Qt::AlignHCenter);

    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);

    painter.setPen(QPen(Qt::black, 1));

    auto scale_unit = tr("m");
    qreal scale_factor = 1;
    qreal step = 50 * scale_;
    int N = std::ceil(length_ / step);
    if (N < 3) {
        step = 25 * scale_;
        N *= 2;
        scale_factor = 50;
        scale_unit = tr("cm");
    } else if (N > 7) {
        step = 100 * scale_;
        N /= 2;
        scale_factor = 2;
    }

    bool flip = true;
    for (int i = 0; i <= N; i++) {
        painter.setBrush(QBrush(flip ? Qt::black : Qt::white));
        painter.drawRect(QRect(step * i, RULER_Y, step, RULER_H));
        flip = !flip;

        QRect txt_box;
        if (i == 0) {
            txt_opts.setAlignment(Qt::AlignLeft);
            txt_box.setRect(0, TXT_Y, TXT_W, TXT_H);
        } else {
            txt_opts.setAlignment(Qt::AlignHCenter);
            txt_box.setRect(step * i - TXT_W / 2, TXT_Y, TXT_W, TXT_H);
        }

        painter.drawText(txt_box, QString("%1%2").arg(i * scale_factor).arg(i == 0 ? "" : scale_unit), txt_opts);
    }
}
