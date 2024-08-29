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
#ifndef CONNECTION_STYLE_H
#define CONNECTION_STYLE_H

#include "connection_database.h"

#include <QColor>
#include <QHash>

namespace ceam {

class ConnectionStyle {

    ConnectionStyle();

    QHash<JackCategory, float> pen_width_;
    QHash<JackCategory, QColor> color_;

public:
    static ConnectionStyle& instance();

    float penWidth(JackCategory cat, float def = 1.0) const;
    QColor color(JackCategory cat, const QColor& def = {}) const;
};

}

#endif // CONNECTION_STYLE_H
