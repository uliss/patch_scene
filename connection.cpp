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
#include "connection.h"
#include "device.h"

#include <QGraphicsScene>
#include <QJsonObject>
#include <QPainter>

namespace {
constexpr const char* KEY_SRC = "src";
constexpr const char* KEY_DEST = "dest";
constexpr const char* KEY_SRC_OUT = "out";
constexpr const char* KEY_DEST_IN = "in";
}

using namespace ceam;

QJsonObject ConnectionData::toJson() const
{
    QJsonObject j;

    j[KEY_SRC] = static_cast<int>(src_);
    j[KEY_DEST] = static_cast<int>(dest_);
    j[KEY_DEST_IN] = static_cast<int>(in_);
    j[KEY_SRC_OUT] = static_cast<int>(out_);

    return j;
}

std::optional<ConnectionData> ConnectionData::fromJson(const QJsonValue& j)
{
    if (!j.isObject())
        return {};

    ConnectionData data(0, 0, 0, 0);
    auto obj = j.toObject();
    auto src = obj.value(KEY_SRC).toInt(-1);
    if (src >= 0)
        data.src_ = src;

    auto dest = obj.value(KEY_DEST).toInt(-1);
    if (dest >= 0)
        data.dest_ = dest;

    auto in = obj.value(KEY_DEST_IN).toInt(-1);
    if (in >= 0)
        data.in_ = in;

    auto out = obj.value(KEY_SRC_OUT).toInt(-1);
    if (out >= 0)
        data.out_ = out;

    return data;
}

Connection::Connection(const ConnectionData& data)
    : data_(data)
{
    setZValue(ZVALUE_CONN);
}

bool Connection::operator==(const ConnectionData& data) const
{
    return data_ == data;
}

void Connection::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QPen p(QColor(50, 50, 50));
    p.setWidthF(1.5);
    painter->setPen(p);
    painter->drawPath(line_);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

QPainterPath Connection::shape() const
{
    return line_;
}

QRectF Connection::boundingRect() const
{
    return line_.controlPointRect();
}

XletInfo Connection::destinationInfo() const
{
    return { data_.destination(), XletType::In, data_.destinationInput() };
}

std::optional<std::pair<Device*, Device*>> Connection::findConnectedElements() const
{
    auto sc = scene();
    if (!sc)
        return {};

    Device* src = nullptr;
    Device* dest = nullptr;

    for (auto it : sc->items()) {
        auto dev = qgraphicsitem_cast<Device*>(it);
        if (dev) {
            if (dev->id() == data_.source()) {
                src = dev;
            } else if (dev->id() == data_.destination()) {
                dest = dev;
            }

            if (src && dest)
                return std::pair { src, dest };
        }
    }

    return {};
}

XletInfo Connection::sourceInfo() const
{
    return { data_.source(), XletType::Out, data_.sourceOutput() };
}

bool Connection::checkConnectedElements() const
{
    return findConnectedElements().has_value();
}

bool Connection::updateCachedPos()
{
    auto conn = findConnectedElements();
    if (!conn.has_value())
        return false;

    auto p0 = conn->first->outletPos(data_.sourceOutput(), true);
    auto p1 = conn->second->inletPos(data_.destinationInput(), true);

    prepareGeometryChange();
    line_.clear();
    line_.moveTo(p0);

    auto bezy = (std::abs(p0.x() - p1.x()) < 20) ? 20 : 40;

    line_.cubicTo(p0 + QPointF(0, bezy), p1 + QPointF(0, -bezy), p1);
    update();

    return true;
}

uint ceam::qHash(const ConnectionData& key)
{
    return ::qHash(key.destination())
        ^ ::qHash(key.destinationInput())
        ^ ::qHash(key.source())
        ^ ::qHash(key.sourceOutput());
}
