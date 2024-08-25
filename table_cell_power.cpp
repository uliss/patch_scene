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
#include "table_cell_power.h"

using namespace ceam;

TableCellPower::TableCellPower(PowerType type, QWidget* parent)
    : QComboBox(parent)
{
    foreachPowerType([this, type](PowerType ptype, int idx) {
        addItem(powerTypeToString(ptype), static_cast<int>(idx));

        if (type == ptype)
            setCurrentIndex(idx);
    });
}

PowerType TableCellPower::powerType() const
{
    bool ok = false;
    auto ptype = currentData().toInt(&ok);
    if (!ok)
        return PowerType::None;
    else
        return static_cast<PowerType>(ptype);
}

void TableCellPower::setPowerType(PowerType type)
{
    auto idx = findData(static_cast<int>(type));
    setCurrentIndex(idx);
}