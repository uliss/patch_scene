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
#ifndef BATTERY_H
#define BATTERY_H

#include <cstdint>
#include <functional>

#include <QString>

namespace ceam {

enum class BatteryType : std::uint8_t {
    None,
    AA,
    AAA,
    AAAA,
    PP3_Krona,
    B,
    C,
    A23,
    A27,
    MaxBattery_,
};

const char* toString(BatteryType type);
const char* toJsonString(BatteryType type);
BatteryType fromJsonString(const QString& str);
void foreachBatteryType(std::function<void(const char*, int)> fn);

class BatteryChange {
    BatteryType typeA_ { BatteryType::None }, typeB_ { BatteryType::None };
    int countA_ { 0 }, countB_ { 0 };

public:
    BatteryChange() { }
    BatteryChange(BatteryType typeA, int countA, BatteryType typeB, int countB);

    int typeADelta() const { return countA_; }
    int typeBDelta() const { return countB_; }
    BatteryType typeA() const { return typeA_; }
    BatteryType typeB() const { return typeB_; }

    explicit operator bool() const;
};
} // namespace ceam

#endif // BATTERY_H
