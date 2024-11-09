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
#include "psc_utils.h"

#include <QRegularExpression>

QString ceam::utils::incrementString(const QString& str)
{
    static const QRegularExpression re(R"(.* (\d+)$)");

    if (str.isEmpty())
        return str;

    auto m = re.match(str);
    if (m.hasMatch()) {
        auto str_num = m.captured(1);
        bool ok = false;
        auto num = str_num.toInt(&ok, 10);
        if (ok) {
            auto pos = m.capturedStart(1);
            return str.first(pos) + QString::number(num + 1, 10);
        } else {
            return str;
        }

        // auto from = m.c
    } else {
        return str.trimmed() + " 1";
    }
}
