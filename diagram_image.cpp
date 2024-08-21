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
#include "diagram_image.h"

#include <QBuffer>
#include <QFile>
#include <QJsonObject>
#include <QTemporaryFile>

namespace {
constexpr const char* JSON_KEY_DATA = "data";
constexpr const char* JSON_KEY_TYPE = "type";
constexpr const char* JSON_PIXMAP = "pixmap";
constexpr const char* JSON_SVG = "svg";

QPixmap pixmapFromJson(const QJsonValue& v)
{
    const auto encoded = v.toString().toLatin1();
    QPixmap p;
    p.loadFromData(QByteArray::fromBase64(encoded), "PNG");
    return p;
}

QJsonValue jsonFromPixmap(const QPixmap& p)
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    p.save(&buffer, "PNG");
    const auto encoded = buffer.data().toBase64();
    return { QLatin1String(encoded) };
}
}

using namespace ceam;

DiagramImage::DiagramImage(const QString& path, QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    setAcceptDrops(false);
    setAcceptHoverEvents(false);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setFlag(QGraphicsItem::ItemIsFocusable, false);

    setImagePath(path);
}

QRectF DiagramImage::boundingRect() const
{
    if (svg_)
        return svg_->boundingRect();
    else if (pixmap_)
        return pixmap_->boundingRect();
    else
        return {};
}

void DiagramImage::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    if (svg_)
        svg_->paint(painter, option, widget);

    if (pixmap_)
        pixmap_->paint(painter, option, widget);
}

bool DiagramImage::setPixmap(const QPixmap& pixmap)
{
    if (!pixmap)
        return false;

    clearImage();
    pixmap_ = new QGraphicsPixmapItem(pixmap, this);
    return true;
}

bool DiagramImage::setSvg(const QString& path)
{
    if (path.isEmpty()) {
        qWarning() << "empty SVG filename";
        return false;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "can't open SVG file:" << path;
        return false;
    }

    auto svg = std::make_unique<QGraphicsSvgItem>(new QGraphicsSvgItem(path, this));
    auto svg_bbox = svg->boundingRect();
    qWarning() << svg_bbox;
    // if (!svg_bbox.isValid()) {
    //     qWarning() << "invalid SVG file:" << file.readAll();
    //     return false;
    // }

    auto content = qCompress(file.readAll());
    if (content.isEmpty()) {
        qWarning() << "SVG compression error:" << path;
        return false;
    }

    clearImage();
    svg_ = svg.release();
    svg_->setParentItem(this);
    svg_content_ = content;
    return true;
}

bool DiagramImage::setImagePath(const QString& path)
{
    if (path.isEmpty())
        return false;

    if (path.endsWith(".svg", Qt::CaseInsensitive)) {
        return setSvg(path);
    } else if (path.endsWith(".png", Qt::CaseInsensitive)
        || path.endsWith(".jpg", Qt::CaseInsensitive)
        || path.endsWith(".jpeg", Qt::CaseInsensitive)) {
        return setPixmap(QPixmap(path));
    } else {
        qWarning() << "unknown image format:" << path;
        return false;
    }
}

bool DiagramImage::isEmpty() const
{
    return !svg_ && !pixmap_;
}

QJsonValue DiagramImage::toJson() const
{
    if (pixmap_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = jsonFromPixmap(pixmap_->pixmap());
        js[JSON_KEY_TYPE] = JSON_PIXMAP;
        return js;
    } else if (svg_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = QLatin1String { svg_content_.toBase64() };
        js[JSON_KEY_TYPE] = JSON_SVG;
        return js;
    } else
        return QJsonValue();
}

std::unique_ptr<DiagramImage> DiagramImage::fromJson(const QJsonValue& v)
{
    if (!v.isObject()) {
        qWarning() << "object expected";
        return {};
    }

    auto obj = v.toObject();
    auto type = obj.value(JSON_KEY_TYPE).toString();
    if (type == JSON_PIXMAP) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            qWarning() << __FILE__ << __FUNCTION__ << "data key expected";
            return {};
        }

        std::unique_ptr<DiagramImage> res(new DiagramImage());
        if (res->setPixmap(pixmapFromJson(data)))
            return res;
        else
            return {};
    } else if (type == JSON_SVG) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            qWarning() << __FILE__ << __FUNCTION__ << "data key expected";
            return {};
        }

        auto compressed_data = QByteArray::fromBase64(data.toString().toLatin1());
        if (compressed_data.isEmpty()) {
            qWarning() << __FILE__ << __FUNCTION__ << "invalid SVG compressed data";
            return {};
        }

        QTemporaryFile tmp;
        if (!tmp.open()) {
            qWarning() << __FILE__ << __FUNCTION__ << "can't open tmp SVG file";
            return {};
        }
        auto nbytes = tmp.write(qUncompress(compressed_data));
        if (nbytes <= 0) {
            qWarning() << __FILE__ << __FUNCTION__ << "can't write to tmp SVG file";
            return {};
        }
        tmp.flush();
        tmp.close();
        std::unique_ptr<DiagramImage> res(new DiagramImage());
        if (res->setSvg(tmp.fileName()))
            return res;
        else
            return {};
    } else {
        qWarning() << "unknown background image type:" << type;
        return {};
    }
}

void DiagramImage::clearImage()
{
    delete pixmap_;
    delete svg_;

    pixmap_ = nullptr;
    svg_ = nullptr;

    svg_content_.clear();
}
