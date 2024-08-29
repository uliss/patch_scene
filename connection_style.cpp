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
#include "connection_style.h"

using namespace ceam;

ConnectionStyleDatabase::ConnectionStyleDatabase()
{
    pen_width_[ConnectionStyle::Power] = 4;
    color_[ConnectionStyle::Power] = Qt::darkGray;
    color_[ConnectionStyle::Invalid] = Qt::red;
    color_[ConnectionStyle::Network] = Qt::darkGreen;
    color_[ConnectionStyle::Light] = Qt::darkBlue;
}

ConnectionStyleDatabase& ConnectionStyleDatabase::instance()
{
    static ConnectionStyleDatabase instance_;
    return instance_;
}

float ConnectionStyleDatabase::penWidth(ConnectionStyle cat, float def) const
{
    auto it = pen_width_.find(cat);
    return it == pen_width_.end() ? def : it.value();
}

QColor ConnectionStyleDatabase::color(ConnectionStyle cat, const QColor& def) const
{
    auto it = color_.find(cat);
    return it == color_.end() ? def : it.value();
}
