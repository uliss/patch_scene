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
#include "battery.h"

#include <QMap>

using namespace ceam;

namespace {

using BatteryMapType = QMap<BatteryType, std::pair<const char*, const char*>>;
const BatteryMapType& batteryNameMap()
{
    // clang-format off
    static const BatteryMapType map_ = {
                                        { BatteryType::None,        { "None",         "none" } },
                                        { BatteryType::AA,          { "AA",           "aa" } },
                                        { BatteryType::AAA,         { "AAA",          "aaa" } },
                                        { BatteryType::AAAA,        { "AAAA",         "aaaa" } },
                                        { BatteryType::B,           { "B",            "b" } },
                                        { BatteryType::C,           { "C",            "c" } },
                                        { BatteryType::D,           { "D",            "d" } },
                                        { BatteryType::LR44,        { "LR44",         "lr44" } },
                                        { BatteryType::PP3_Krona,   { "PP3 (Krona)",  "krona" } },
                                        { BatteryType::R12x3,       { "3R12",         "3r12" } },
                                        { BatteryType::A23,         { "A23",          "a23" } },
                                        { BatteryType::A27,         { "A27",          "a27" } },
                                        { BatteryType::CR2016,      { "CR2016",       "cr2016" } },
                                        { BatteryType::CR2025,      { "CR2025",       "cr2025" } },
                                        { BatteryType::CR2032,      { "CR2032",       "cr2032" } },
                                        };
    // clang-format on
    return map_;
}

} // namespace

namespace ceam {

// clang-format on

const char* toString(BatteryType type)
{
    auto it = batteryNameMap().find(type);
    return (it == batteryNameMap().end()) ? "?" : it->first;
}

const char* toJsonString(BatteryType type)
{
    auto it = batteryNameMap().find(type);
    return (it == batteryNameMap().end()) ? "?" : it->second;
}

BatteryType fromJsonString(const QString& str)
{
    for (auto it = batteryNameMap().keyValueBegin(); it != batteryNameMap().keyValueEnd(); ++it) {
        if (it->second.second == str.toLower())
            return it->first;
    }

    return BatteryType::None;
}

void foreachBatteryType(const std::function<void(const char*, int)>& fn)
{
    for (int i = static_cast<int>(BatteryType::None);
        i < static_cast<int>(BatteryType::MaxBattery_);
        i++) //
    {
        fn(toString(static_cast<BatteryType>(i)), i);
    }
}

BatteryChange::BatteryChange(BatteryType typeA, int countA, BatteryType typeB, int countB)
{
    typeA_ = typeA;
    typeB_ = typeB;
    countA_ = countA;
    countB_ = countB;
}

BatteryChange::operator bool() const
{
    return typeA_ != typeB_ || countA_ != countB_;
}

} // namespace ceam
