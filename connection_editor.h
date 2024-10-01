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
#ifndef CONNECTION_EDITOR_H
#define CONNECTION_EDITOR_H

#include "connection_data.h"

#include <QGraphicsObject>
#include <QMouseEvent>

namespace ceam {

class ConnectionEditor : public QGraphicsObject {
    Q_OBJECT

public:
    enum { Type = QGraphicsItem::UserType + 5 };
    int type() const override { return Type; }

public:
    ConnectionEditor();

    void setConnectionData(const ConnectionData& data);

    QRectF boundingRect() const final;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = nullptr) final;

signals:
    void connectionUpdated(ConnectionData);

private:
    QList<QGraphicsItem*> handles_;
    ConnectionData data_;
};

} // namespace ceam

#endif // CONNECTION_EDITOR_H
