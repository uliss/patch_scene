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
#include "background_properties_dialog.h"
#include "connection.h"
#include "logging.hpp"

#include <QAction>
#include <QBuffer>
#include <QFile>
#include <QGraphicsScene>
#include <QJsonObject>
#include <QMenu>
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

    {
        QSignalBlocker sb(this);
        clear();
    }

    pixmap_ = new QGraphicsPixmapItem(pixmap);
    auto bbox = pixmap_->boundingRect();
    pixmap_->setPos(-bbox.width() / 2, -bbox.height() / 2);
    pixmap_->setZValue(ZVALUE_BACKGROUND);
    scene_->addItem(pixmap_);
    emit backgroundChanged();
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

    {
        QSignalBlocker sb(this);
        clear();
    }
    svg_ = new QGraphicsSvgItem();
    svg_->setSharedRenderer(&svg_renderer_);
    auto bbox = svg_->boundingRect();
    svg_->setPos(-bbox.width() / 2, -bbox.height() / 2);
    svg_->setZValue(ZVALUE_BACKGROUND);
    scene_->addItem(svg_);
    svg_bin_content_ = qCompress(content);

    emit backgroundChanged();
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

bool SceneBackground::isVisible() const
{
    if (!scene_)
        return false;

    return visible_;
}

void SceneBackground::setVisible(bool value)
{
    if (pixmap_)
        pixmap_->setVisible(value);

    if (svg_)
        svg_->setVisible(value);
}

QRectF SceneBackground::boundingRect() const
{
    if (!scene_)
        return {};

    if (pixmap_)
        return pixmap_->boundingRect();

    if (svg_)
        return svg_->boundingRect();

    return {};
}

void SceneBackground::setPos(const QPointF& pos)
{
    if (!scene_)
        return;

    if (pixmap_)
        return pixmap_->setPos(pos);

    if (svg_)
        return svg_->setPos(pos);
}

bool SceneBackground::isEmpty() const
{
    return !svg_ && !pixmap_;
}

QJsonValue SceneBackground::toJson() const
{
    if (!scene_)
        return {};

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
    if (!scene_)
        return false;

    if (!v.isObject()) {
        if (!v.isNull()) {
            WARN() << "object expected";
        }

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

void SceneBackground::addToContextMenu(QMenu& menu)
{
    if (!isEmpty()) {
        auto clear_bg = new QAction(tr("Clear background"), this);
        connect(clear_bg, &QAction::triggered, this, [this]() { clear(); });

        menu.addAction(clear_bg);

        auto replace_bg = new QAction(tr("Replace background"), this);
        connect(replace_bg, &QAction::triggered, this,
            [this]() { emit requestBackgroundChange(); });

        menu.addAction(replace_bg);

        auto image_props = new QAction(tr("Background properties"), this);
        connect(image_props, &QAction::triggered, this,
            [this]() {
                BackgroundPropertiesDialog dialog(this);
                dialog.exec();
            });

        menu.addAction(image_props);
    } else {
        auto set_bg = new QAction(tr("Set background"), this);
        connect(set_bg, &QAction::triggered, this,
            [this]() { emit requestBackgroundChange(); });

        menu.addAction(set_bg);
    }
}

QGraphicsItem* SceneBackground::sceneItem()
{
    if (!scene_)
        return nullptr;

    if (pixmap_)
        return pixmap_;
    else if (svg_)
        return svg_;
    else
        return nullptr;
}

void SceneBackground::clear()
{
    if (!scene_)
        return;

    if (pixmap_) {
        scene_->removeItem(pixmap_);
        delete pixmap_;
        pixmap_ = nullptr;
        emit backgroundChanged();
    }

    if (svg_) {
        scene_->removeItem(svg_);
        delete svg_;
        svg_ = nullptr;
        emit backgroundChanged();
    }

    svg_bin_content_.clear();
}
