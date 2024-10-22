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
#ifndef TABLE_CELL_SOCKET_H
#define TABLE_CELL_SOCKET_H

#include "connector_type.h"
#include <QComboBox>

namespace ceam {
class TableCellConnectorType : public QComboBox {
    Q_OBJECT
public:
    TableCellConnectorType(ConnectorType type, QWidget* parent = nullptr);

    ConnectorType connectorType() const;
};
}

#endif // TABLE_CELL_SOCKET_H
