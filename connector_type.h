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
#ifndef CONNECTOR_TYPE_H
#define CONNECTOR_TYPE_H

#include <QCoreApplication>
#include <QJsonValue>
#include <QString>
#include <cstdint>

namespace ceam {

class ConnectorType {
    Q_DECLARE_TR_FUNCTIONS(ConnectorType)

    enum Type : std::uint8_t {
        SocketMale,
        SocketFemale,
        PlugMale,
        PlugFemale,
        MaxConnectorType,
    };

    ConnectorType(Type t)
        : type_(t)
    {
    }

public:
    ConnectorType()
        : ConnectorType(SocketFemale)
    {
    }

    bool operator==(const ConnectorType& t) const
    {
        return t.type_ == type_;
    }

    bool operator!=(const ConnectorType& t) const
    {
        return !operator==(t);
    }

    QString iconPath() const;
    QString localizedName() const;

    QString toJsonString() const;
    QJsonValue toJson() const;
    int toInt() const;

    bool isSocket() const;
    bool isPlug() const;

    ConnectorType complement() const;

public:
    static std::optional<ConnectorType> fromJson(const QJsonValue& val);
    static std::optional<ConnectorType> fromInt(int val);
    static void foreachType(const std::function<void(const ConnectorType&)> fn);

    static const ConnectorType socket_male, socket_female, plug_male, plug_female;

private:
    Type type_;
};

size_t qHash(const ConnectorType& ctype);

} // namespace ceam

QDebug operator<<(QDebug debug, const ceam::ConnectorType& type);

#endif // CONNECTOR_TYPE_H
