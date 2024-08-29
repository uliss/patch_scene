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
#ifndef CONNECTION_DATABASE_H
#define CONNECTION_DATABASE_H

#include "connector_type.h"
#include "socket.h"

#include <QHash>

namespace ceam {

struct ConnectionJack {
    ConnectorModel model;
    ConnectorType type;

    bool operator==(const ConnectionJack& jack) const
    {
        return model == jack.model && type == jack.type;
    }
};

size_t qHash(const ConnectionJack& key);

struct ConnectionPair {
    ConnectionJack p0, p1;

    bool operator==(const ConnectionPair& type) const;
};

size_t qHash(const ConnectionPair& key);

enum class ConnectionStyle {
    Unknown,
    Invalid,
    Audio,
    Light,
    Computer,
    Network,
    Power,
    Auto,
    NotFound
};

class ConnectionDatabase {
    QHash<ConnectionPair, ConnectionStyle> db_;

public:
    ConnectionDatabase();

    void initDefault();

    qsizetype count() const { return db_.count(); }

    bool add(ConnectionPair t, ConnectionStyle cat);

    /**
     * Search connection pair style
     * @complexity amort O(1)
     * @return ConnectionStyle or NotFound
     */
    ConnectionStyle search(ConnectionPair t) const;
    bool contains(ConnectionPair t) const;
};

} // namespace ceam

#endif // CONNECTION_DATABASE_H
