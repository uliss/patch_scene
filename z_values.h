/*****************************************************************************
 * Copyright 2025 Serge Poltavski. All rights reserved.
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
#ifndef Z_VALUES_H
#define Z_VALUES_H

namespace ceam {

constexpr double ZVALUE_CONN = 3000;
constexpr double ZVALUE_BACKGROUND = -3000;
constexpr double ZVALUE_LIVE_CONN = 16000;
constexpr double ZVALUE_SELECTION = 32000;

constexpr double ZVALUE_DEVICE = 1000;
constexpr double ZVALUE_SEND = ZVALUE_DEVICE;
constexpr double ZVALUE_RETURN = ZVALUE_DEVICE;
constexpr double ZVALUE_INSTRUMENT = ZVALUE_DEVICE;
constexpr double ZVALUE_HUMAN = 0;
constexpr double ZVALUE_FURNITURE = -1000;
constexpr double ZVALUE_COMMENT = -2000;

} // namespace ceam

#endif // Z_VALUES_H
