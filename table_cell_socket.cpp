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
#include "table_cell_socket.h"

using namespace ceam;

TableCellConnectorType::TableCellConnectorType(ConnectorType type, QWidget* parent)
{
    foreachConnectorType([this, type](ConnectorType t, int int_type) {
        addItem(connectorTypeName(t), int_type);

        if (type == t)
            setCurrentIndex(int_type);
    });
}

ConnectorType TableCellConnectorType::connectorType() const
{
    bool ok = false;
    auto conn = currentData().toInt(&ok);
    if (!ok)
        return ConnectorType::SocketFemale;
    else
        return static_cast<ConnectorType>(conn);
}

void TableCellConnectorType::setConnectorType(ConnectorType type)
{
    auto idx = findData(static_cast<int>(type));
    setCurrentIndex(idx);
}
