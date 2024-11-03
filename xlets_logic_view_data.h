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
#ifndef XLETS_LOGIC_VIEW_DATA_H
#define XLETS_LOGIC_VIEW_DATA_H

#include <QJsonValue>

namespace ceam {

class XletsLogicViewData {
public:
    constexpr static int MAX_COL_COUNT = 24;
    constexpr static int MIN_COL_COUNT = 2;
    constexpr static int DEF_COL_COUNT = 8;

public:
    int maxInputColumnCount() const { return max_input_column_count_; }
    bool setMaxInputColumnCount(int n);

    int maxOutputColumnCount() const { return max_output_column_count_; }
    bool setMaxOutputColumnCount(int n);

public:
    QJsonValue toJson() const;
    static std::optional<XletsLogicViewData> fromJson(const QJsonValue& j);

private:
    QString name_;
    int max_input_column_count_ { DEF_COL_COUNT };
    int max_output_column_count_ { DEF_COL_COUNT };
};
} // namespace ceam

#endif // XLETS_LOGIC_VIEW_DATA_H
