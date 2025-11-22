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

constexpr qreal WIDTH = 200.0;

ScaleWidget::ScaleWidget(QWidget* parent)
    : QWidget { parent }
    , length_(WIDTH)
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
    QFontMetrics fm(font());

    QSizeF size = geometry().size();
    size.rwidth() -= 2;

    auto ft = font();
    ft.setPointSize(ft.pointSize() * 0.75);
    painter.setFont(ft);

    painter.setPen(QPen(Qt::black, 1));
    painter.setBrush(Qt::NoBrush);

    auto scale_unit = tr("m");
    qreal scale_factor = 1;
    qreal step = 50 * scale_;

    if (scale_ >= 2.0) {
        step = 25 * scale_;
        scale_factor = 50;
        scale_unit = tr("cm");
    } else if (scale_ <= 0.5) {
        step = 100 * scale_;
        scale_factor = 2;
    }

    bool flip_color = true;
    const auto W = width();

    QTextOption txt_opts;
    QRect txt_box;
    QString text;

    const auto N = static_cast<int>(std::ceil(W / step));

    for (int i = 0; i <= N; i++) {
        auto x = i * step;
        painter.setBrush(QBrush(flip_color ? Qt::black : Qt::white));
        painter.drawRect(QRectF(x, RULER_Y, step, RULER_H));
        flip_color = !flip_color;

        if (i == 0) {
            text = "0";
        } else if (i < N) {
            text = QString("%1%2").arg(i * scale_factor).arg(scale_unit);
        } else {
            text = QString("<%1%2").arg(i * scale_factor).arg(scale_unit);
        }

        auto tw = fm.boundingRect(text).width() + 2;

        if (i == 0) {
            txt_opts.setAlignment(Qt::AlignLeft);
            txt_box.setRect(0, TXT_Y, tw, TXT_H);
        } else if (i < N) {
            if ((x + tw * 0.5) < W) {
                txt_box.setRect(x - tw * 0.5, TXT_Y, tw, TXT_H);
                txt_opts.setAlignment(Qt::AlignHCenter);
            } else {
                txt_box.setRect(x - tw, TXT_Y, tw, TXT_H);
                txt_opts.setAlignment(Qt::AlignRight);
            }
        } else {
            if (txt_box.right() + tw < W) {
                txt_box.setRect(W - tw, TXT_Y, tw, TXT_H);
                txt_opts.setAlignment(Qt::AlignRight);
            } else
                break;
        }

        painter.drawText(txt_box, text, txt_opts);
    }

    // draw closing line
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(QRectF(W - 0.5, RULER_Y, 5, RULER_H));
}
