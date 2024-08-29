/*****************************************************************************
 * Copyright 2024 Serge Poltavski. All rights reserved.
 *
 * This file may be distributed under the terms of GNU Public License version
 * 3 (GPL v3) as defined by the Free Software Foundation (FSF). A copy of the
 * license should have been included with this file, or the project in which
 * this file belongs to. You may also search the details of GPL v3 at:
 * http://www.gnu.org/licenses/gpl-3.0.txt
 *
 * If you have any questions regarding the use of this file, feel free to
 * contact the author of this file, or the owner of the project in which
 * this file belongs to.
 *****************************************************************************/
#include "connection_database.h"

namespace ceam {

size_t qHash(const ConnectionJack& key)
{
    auto m = static_cast<std::uint16_t>(key.model);
    auto t = key.type.toInt();
    return ::qHash(m) ^ ::qHash(t);
}

ConnectionDatabase::ConnectionDatabase() { }

void ConnectionDatabase::initDefault()
{
}

bool ConnectionDatabase::add(ConnectionPair t, ConnectionStyle cat)
{
    switch (cat) {
    case ConnectionStyle::Auto:
    case ConnectionStyle::NotFound:
        return false;
    // case ConnectionCategory::Unknown:
    // case ConnectionCategory::Invalid:
    // case ConnectionCategory::Audio:
    // case ConnectionCategory::Light:
    // case ConnectionCategory::Computer:
    // case ConnectionCategory::Network:
    // case ConnectionCategory::Power:
    default:
        db_.insert(t, cat);
        return true;
    }
}

ConnectionStyle ConnectionDatabase::search(ConnectionPair t) const
{
    auto it = db_.find(t);
    return it == db_.end() ? ConnectionStyle::NotFound : it.value();
}

bool ConnectionDatabase::contains(ConnectionPair t) const
{
    return search(t) != ConnectionStyle::NotFound;
}

size_t qHash(const ConnectionPair& key)
{
    return qHash(key.p0) ^ qHash(key.p1);
}

bool ConnectionPair::operator==(const ConnectionPair& type) const
{
    return (p0 == type.p0 && p1 == type.p1)
        || (p0 == type.p1 && p1 == type.p0);
}

} // namespace ceam
