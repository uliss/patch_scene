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

struct Jack {
    ConnectorModel model;
    ConnectorType type;

    bool operator==(const Jack& jack) const
    {
        return model == jack.model && type == jack.type;
    }
};

size_t qHash(const Jack& key);

struct JackPair {
    Jack p0, p1;

    bool operator==(const JackPair& type) const;
};

size_t qHash(const JackPair& key);

enum class JackCategory {
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
    QHash<JackPair, JackCategory> db_;

public:
    ConnectionDatabase();

    void initDefault();

    qsizetype count() const { return db_.count(); }

    bool add(JackPair t, JackCategory cat);
    JackCategory search(JackPair t) const;
    bool contains(JackPair t) const;
};

} // namespace ceam

#endif // CONNECTION_DATABASE_H
