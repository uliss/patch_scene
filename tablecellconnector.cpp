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
#include "tablecellconnector.h"
#include "socket.h"

#include <QFile>

constexpr auto CONNECTOR_MIN = static_cast<int>(ConnectorModel::UNKNOWN);
constexpr auto CONNECTOR_MAX = static_cast<int>(ConnectorModel::CONNECTOR_MAX);

class IconCache {
    std::map<ConnectorModel, QIcon> cache_;

public:
    static IconCache& instance()
    {
        static IconCache cache;
        return cache;
    }

    const QIcon& connectorIcon(ConnectorModel model)
    {
        static QIcon null;

        auto it = cache_.find(model);
        if (it == cache_.end()) {
            auto icon_name = connectorSvgName(model);
            auto icon_path = QString(":/connectors/%1_socket.svg").arg(icon_name);
            if (QFile::exists(icon_path)) {
                QIcon icon(icon_path);
                cache_[model] = icon;
                return cache_[model];
            } else {
                cache_[model] = null;
                return null;
            }
        } else {
            return it->second;
        }
    }
};

TableCellConnector::TableCellConnector(QWidget* parent)
    : QComboBox { parent }
{
    for (int i = CONNECTOR_MIN; i < CONNECTOR_MAX; i++) {
        auto model = static_cast<ConnectorModel>(i);
        auto& icon = IconCache::instance().connectorIcon(model);
        if (!icon.isNull())
            addItem(icon, connectorName(model), static_cast<int>(model));
    }

    QComboBox::model()->sort(0);
}

ConnectorModel TableCellConnector::model() const
{
    bool ok = false;
    auto m = currentData().toInt(&ok);
    if (!ok)
        return ConnectorModel::UNKNOWN;
    else
        return static_cast<ConnectorModel>(m);
}

void TableCellConnector::setModel(ConnectorModel model)
{
    auto idx = findData(static_cast<int>(model));
    setCurrentIndex(idx);
}
