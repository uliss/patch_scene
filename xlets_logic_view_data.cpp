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
#include "xlets_logic_view_data.h"

#include <QJsonObject>

namespace {
constexpr const char* JSON_KEY_INPUT_COLUMNS = "input-columns";
constexpr const char* JSON_KEY_OUTPUT_COLUMNS = "output-columns";
constexpr const char* JSON_KEY_NAME = "name";
constexpr const char* JSON_KEY_TYPE = "type";
constexpr const char* JSON_STR_LOGIC = "logic";
}

namespace ceam {

bool XletsLogicViewData::setMaxInputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_input_column_count_ = n;
    return true;
}

bool XletsLogicViewData::setMaxOutputColumnCount(int n)
{
    if (n < MIN_COL_COUNT || n > MAX_COL_COUNT)
        return false;

    max_output_column_count_ = n;
    return true;
}

QJsonValue XletsLogicViewData::toJson() const
{
    QJsonObject json;
    json[JSON_KEY_INPUT_COLUMNS] = max_input_column_count_;
    json[JSON_KEY_OUTPUT_COLUMNS] = max_output_column_count_;
    json[JSON_KEY_NAME] = name_;
    json[JSON_KEY_TYPE] = JSON_STR_LOGIC;
    return json;
}

std::optional<XletsLogicViewData> XletsLogicViewData::fromJson(const QJsonValue& j)
{
    XletsLogicViewData res;
    if (!j.isObject())
        return res;

    auto obj = j.toObject();

    res.max_input_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_INPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);
    res.max_output_column_count_ = qBound<int>(MIN_COL_COUNT, obj[JSON_KEY_OUTPUT_COLUMNS].toInt(DEF_COL_COUNT), MAX_COL_COUNT);

    return res;
}

} // namespace ceam
