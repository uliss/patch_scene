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

using namespace ceam;

QJsonObject ConnectionData::toJson() const
{
    QJsonObject j;

    j["src"] = static_cast<int>(src);
    j["dest"] = static_cast<int>(dest);
    j["in"] = static_cast<int>(in);
    j["out"] = static_cast<int>(out);

    return j;
}

bool ConnectionData::fromJson(const QJsonValue& j, ConnectionData& data)
{
    if (!j.isObject())
        return false;

    auto obj = j.toObject();
    auto src = obj.value("src").toInt(-1);
    if (src >= 0)
        data.src = src;

    auto dest = obj.value("dest").toInt(-1);
    if (dest >= 0)
        data.dest = dest;

    auto in = obj.value("in").toInt(-1);
    if (in >= 0)
        data.in = in;

    auto out = obj.value("out").toInt(-1);
    if (out >= 0)
        data.out = out;

    return true;
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
    return { data_.dest, XletType::In, data_.in };
}

XletInfo Connection::sourceInfo() const
{
    return { data_.src, XletType::Out, data_.out };
}

QJsonObject Connection::toJson() const
{
    return data_.toJson();
}

bool Connection::checkConnectedElements() const
{
    auto sc = scene();
    if (!sc)
        return false;

    Device* src = nullptr;
    Device* dest = nullptr;

    for (auto it : sc->items()) {
        auto dev = qgraphicsitem_cast<Device*>(it);
        if (dev) {
            if (dev->id() == data_.src) {
                src = dev;
            } else if (dev->id() == data_.dest) {
                dest = dev;
            }

            if (src && dest)
                break;
        }
    }

    return src && dest;
}

bool Connection::updateCachedPos()
{
    Device* src = nullptr;
    Device* dest = nullptr;

    for (auto it : scene()->items()) {
        auto dev = qgraphicsitem_cast<Device*>(it);
        if (dev) {
            if (dev->id() == data_.src) {
                src = dev;
            } else if (dev->id() == data_.dest) {
                dest = dev;
            }

            if (src && dest)
                break;
        }
    }

    if (!src) {
        qWarning() << "src id not found: " << data_.src;
        return false;
    }

    if (!dest) {
        qWarning() << "dest id not found: " << data_.dest;
        return false;
    }

    auto p0 = src->outletPos(data_.out, true);
    auto p1 = dest->inletPos(data_.in, true);

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
    return ::qHash(key.dest)
        ^ ::qHash(key.in)
        ^ ::qHash(key.src)
        ^ ::qHash(key.out);
}
