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
#ifndef DIAGRAM_IMAGE_H
#define DIAGRAM_IMAGE_H

#include <QGraphicsPixmapItem>

class DiagramImage : public QGraphicsPixmapItem {
public:
    DiagramImage(QGraphicsItem* parent = nullptr);

    enum { Type = QGraphicsItem::UserType + 4 };
    int type() const override { return Type; }
};

#endif // DIAGRAM_IMAGE_H
