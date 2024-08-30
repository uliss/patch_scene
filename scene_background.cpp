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
#include "scene_background.h"

#include <QBuffer>
#include <QFile>
#include <QGraphicsScene>
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

SceneBackground::SceneBackground()
{
    // setAcceptDrops(false);
    // setAcceptHoverEvents(false);
    // setFlag(QGraphicsItem::ItemIsSelectable, false);
    // setFlag(QGraphicsItem::ItemIsMovable, false);
    // setFlag(QGraphicsItem::ItemIsFocusable, false);

    // loadImage(path);
}

void SceneBackground::setScene(QGraphicsScene* scene)
{
    if (scene_)
        clear();

    scene_ = scene;
}

bool SceneBackground::setPixmap(const QPixmap& pixmap)
{
    if (!scene_ || !pixmap)
        return false;

    clear();
    pixmap_ = new QGraphicsPixmapItem(pixmap);
    scene_->addItem(pixmap_);
    return true;
}

bool SceneBackground::setSvg(const QString& path)
{
    if (!scene_)
        return false;

    if (path.isEmpty()) {
        qWarning() << "empty SVG filename";
        return false;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "can't open SVG file:" << path;
        return false;
    }

    return setSvg(file.readAll());
}

bool SceneBackground::setSvg(const QByteArray& content)
{
    {
        QSignalBlocker sb(&svg_renderer_);
        if (!scene_ || !svg_renderer_.load(content))
            return false;
    }

    clear();
    svg_ = new QGraphicsSvgItem();
    svg_->setSharedRenderer(&svg_renderer_);
    scene_->addItem(svg_);
    svg_bin_content_ = qCompress(content);
    return true;
}

bool SceneBackground::loadImage(const QString& path)
{
    if (!scene_)
        return false;

    if (path.isEmpty())
        return false;

    if (path.endsWith(".svg", Qt::CaseInsensitive)) {
        return setSvg(path);
    } else if (path.endsWith(".png", Qt::CaseInsensitive)
        || path.endsWith(".jpg", Qt::CaseInsensitive)
        || path.endsWith(".jpeg", Qt::CaseInsensitive)) {

        QPixmap p(path);
        if (p.isNull())
            qWarning() << __FUNCTION__ << "invalid pixmap:" << p;

        return setPixmap(p);
    } else {
        qWarning() << "unknown image format:" << path;
        return false;
    }
}

void SceneBackground::setVisible(bool value)
{
    if (pixmap_)
        pixmap_->setVisible(value);

    if (svg_)
        svg_->setVisible(value);
}

bool SceneBackground::isEmpty() const
{
    return !svg_ && !pixmap_;
}

QJsonValue SceneBackground::toJson() const
{
    if (pixmap_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = jsonFromPixmap(pixmap_->pixmap());
        js[JSON_KEY_TYPE] = JSON_PIXMAP;
        return js;
    } else if (svg_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = QLatin1String { svg_bin_content_.toBase64() };
        js[JSON_KEY_TYPE] = JSON_SVG;
        return js;
    } else
        return QJsonValue();
}

bool SceneBackground::setFromJson(const QJsonValue& v)
{
    if (!v.isObject()) {
        qWarning() << "object expected";
        return false;
    }

    auto obj = v.toObject();
    auto type = obj.value(JSON_KEY_TYPE).toString();

    if (type == JSON_PIXMAP) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            qWarning() << __FILE__ << __FUNCTION__ << "data key expected";
            return false;
        }

        return setPixmap(pixmapFromJson(data));

    } else if (type == JSON_SVG) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            qWarning() << __FILE__ << __FUNCTION__ << "data key expected";
            return false;
        }

        auto bin_data = QByteArray::fromBase64(data.toString().toLatin1());
        if (bin_data.isEmpty()) {
            qWarning() << __FILE__ << __FUNCTION__ << "invalid SVG compressed data";
            return false;
        }

        return setSvg(qUncompress(bin_data));
    } else {
        qWarning() << "unknown background image type:" << type;
        return false;
    }
}

void SceneBackground::clear()
{
    if (!scene_)
        return;

    if (pixmap_) {
        scene_->removeItem(pixmap_);
        delete pixmap_;
        pixmap_ = nullptr;
    }

    if (svg_) {
        scene_->removeItem(svg_);
        delete svg_;
        svg_ = nullptr;
    }

    svg_bin_content_.clear();
}
