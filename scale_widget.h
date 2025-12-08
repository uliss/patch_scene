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
#ifndef SCALE_WIDGET_H
#define SCALE_WIDGET_H

#include <QWidget>

namespace ceam {

class ScaleWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScaleWidget(QWidget* parent = nullptr);

    void setPos(const QPoint& pos);

public slots:
    void setScale(qreal scale);

protected:
    void paintEvent(QPaintEvent* event) final;

private:
    qreal length_;
    qreal scale_;
};

} // namespace ceam

#endif // SCALE_WIDGET_H
