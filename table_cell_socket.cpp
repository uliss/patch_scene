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
    ConnectorType::foreachType([this, type](const ConnectorType& t) {
        addItem(QIcon(t.iconPath()), t.localizedName(), t.toInt());

        if (type == t)
            setCurrentIndex(t.toInt());
    });
}

ConnectorType TableCellConnectorType::connectorType() const
{
    auto res = ConnectorType::socket_female;

    bool ok = false;
    auto int_type = currentData().toInt(&ok);
    if (!ok)
        return res;

    auto conn = ConnectorType::fromInt(int_type);
    if (conn)
        res = conn.value();

    return res;
}

void TableCellConnectorType::setConnectorType(const ConnectorType& type)
{
    setCurrentIndex(findData(type.toInt()));
}
