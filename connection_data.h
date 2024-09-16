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
#ifndef CONNECTION_DATA_H
#define CONNECTION_DATA_H

#include "xlet_info.h"

namespace ceam {

enum class ConnectionCordType : std::uint8_t {
    Linear,
    Bezier,
    Segmented
};

class ConnectionData {
    DeviceId src_ { 0 }, dest_ { 0 };
    XletIndex out_ { 0 }, in_ { 0 };
    ConnectionCordType cord_type_ { ConnectionCordType::Bezier };

public:
    ConnectionData(DeviceId src, XletIndex out, DeviceId dest, XletIndex in)
        : src_(src)
        , out_(out)
        , dest_(dest)
        , in_(in)
    {
    }

    DeviceId source() const { return src_; }
    DeviceId destination() const { return dest_; }
    XletIndex sourceOutput() const { return out_; }
    XletIndex destinationInput() const { return in_; }

    XletInfo sourceInfo() const { return { src_, out_, XletType::Out }; }
    XletInfo destinationInfo() const { return { dest_, in_, XletType::In }; }

    ConnectionCordType cordType() const { return cord_type_; }
    void setCordType(ConnectionCordType type) { cord_type_ = type; }

    const bool operator==(const ConnectionData& data) const
    {
        return data.src_ == src_
            && data.dest_ == dest_
            && data.in_ == in_
            && data.out_ == out_;
    }

    bool operator!=(const ConnectionData& data) const { return !operator==(data); }

    bool relatesToId(DeviceId id) const
    {
        return src_ == id || dest_ == id;
    }

    bool isValid() const
    {
        return src_ != dest_;
    }

    bool isSameSource(const ConnectionData& conn) const
    {
        return src_ == conn.src_ && out_ == conn.out_;
    }

    bool isSameDestimation(const ConnectionData& conn) const
    {
        return dest_ == conn.dest_ && in_ == conn.in_;
    }

    /**
     * converts to Json object
     */
    QJsonObject toJson() const;

    bool setEndPoint(const XletInfo& ep);

public:
    static std::optional<ConnectionData> fromJson(const QJsonValue& j);
    static std::optional<ConnectionData> fromXletPair(const XletInfo& x0, const XletInfo& x1);
};

QDebug operator<<(QDebug debug, const ConnectionData& c);
size_t qHash(const ConnectionData& key);

} // namespace ceam

Q_DECLARE_METATYPE(ceam::ConnectionData)

#endif // CONNECTION_DATA_H
