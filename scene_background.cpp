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
#include "logging.hpp"
#include "z_values.h"

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
constexpr const char* JSON_KEY_X = "x";
constexpr const char* JSON_KEY_Y = "y";
constexpr const char* JSON_KEY_WIDTH = "width";
constexpr const char* JSON_KEY_HEIGHT = "height";
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
} // namespace

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
        WARN() << "empty SVG filename";
        return false;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        WARN() << "can't open SVG file:" << path;
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

bool SceneBackground::adjustSizeAndPos(const QRectF& rect)
{
    auto bbox = imageRect();
    if (!bbox.isValid() || !rect.isValid())
        return false;

    auto item = sceneItem();
    if (!item)
        return false;

    WARN() << rect;

    auto tr = QTransform::fromScale(rect.width() / bbox.width(), rect.height() / bbox.height());
    item->setTransform(tr);
    item->moveBy(rect.x(), rect.y());
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
            WARN() << "invalid pixmap:" << p;

        return setPixmap(p);
    } else {
        WARN() << "unknown image format:" << path;
        return false;
    }
}

void SceneBackground::setVisible(bool value)
{
    auto item = sceneItem();
    if (item)
        item->setVisible(value);
}

QRectF SceneBackground::imageRect() const
{
    auto item = sceneItem();
    return item ? item->boundingRect() : QRectF {};
}

QSizeF SceneBackground::sceneSize() const
{
    auto item = sceneItem();
    if (item)
        return item->scene()->sceneRect().size();
    else
        return {};
}

QSizeF SceneBackground::viewSize() const
{
    auto item = sceneItem();
    if (item)
        return item->transform().mapRect(item->boundingRect()).size();
    else
        return {};
}

qreal SceneBackground::x() const
{
    auto item = sceneItem();
    if (item)
        return item->x() + viewSize().width() * 0.5;
    else
        return 0;
}

qreal SceneBackground::y() const
{
    auto item = sceneItem();
    if (item)
        return item->y() + viewSize().height() * 0.5;
    else
        return 0;
}

void SceneBackground::setPos(const QPointF& pos)
{
    auto item = sceneItem();
    if (item) {
        auto bbox = viewSize();
        item->setPos(pos.x() - bbox.width() * 0.5, pos.y() - bbox.height() * 0.5);
    }
}

void SceneBackground::setSize(const QSizeF& size)
{
    auto item = sceneItem();
    if (!item)
        return;

    auto bbox = imageRect();
    if (!bbox.isValid())
        return;

    auto dx = size.width() / bbox.width();
    auto dy = size.height() / bbox.height();

    item->setTransform(QTransform::fromScale(dx, dy));
}

bool SceneBackground::isEmpty() const
{
    return sceneItem() == nullptr;
}

QJsonValue SceneBackground::toJson() const
{
    if (!scene_)
        return {};

    if (pixmap_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = jsonFromPixmap(pixmap_->pixmap());
        js[JSON_KEY_TYPE] = JSON_PIXMAP;

        auto tr = pixmap_->transform();
        auto bbox = pixmap_->boundingRect();
        js[JSON_KEY_WIDTH] = tr.mapRect(bbox).width();
        js[JSON_KEY_HEIGHT] = tr.mapRect(bbox).height();
        js[JSON_KEY_X] = pixmap_->x() + bbox.width() * 0.5;
        js[JSON_KEY_Y] = pixmap_->y() + bbox.height() * 0.5;
        return js;
    } else if (svg_) {
        QJsonObject js;
        js[JSON_KEY_DATA] = QLatin1String { svg_bin_content_.toBase64() };
        js[JSON_KEY_TYPE] = JSON_SVG;

        auto tr = svg_->transform();
        auto bbox = svg_->boundingRect();
        js[JSON_KEY_WIDTH] = tr.mapRect(bbox).width();
        js[JSON_KEY_HEIGHT] = tr.mapRect(bbox).height();
        js[JSON_KEY_X] = svg_->x() + bbox.width() * 0.5;
        js[JSON_KEY_Y] = svg_->y() + bbox.height() * 0.5;
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
    auto x = obj.value(JSON_KEY_X).toDouble();
    auto y = obj.value(JSON_KEY_Y).toDouble();
    auto width = obj.value(JSON_KEY_WIDTH).toDouble();
    auto height = obj.value(JSON_KEY_HEIGHT).toDouble();

    if (type == JSON_PIXMAP) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            WARN() << "data key expected";
            return false;
        }

        if (!setPixmap(pixmapFromJson(data)))
            return false;

        return adjustSizeAndPos({ x, y, width, height });

    } else if (type == JSON_SVG) {
        auto data = obj.value(JSON_KEY_DATA);
        if (!data.isString()) {
            WARN() << "data key expected";
            return false;
        }

        auto bin_data = QByteArray::fromBase64(data.toString().toLatin1());
        if (bin_data.isEmpty()) {
            WARN() << "invalid SVG compressed data";
            return false;
        }

        if (!setSvg(qUncompress(bin_data)))
            return false;

        return adjustSizeAndPos({ x, y, width, height });
    } else {
        WARN() << "unknown background image type:" << type;
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

const QGraphicsItem* SceneBackground::sceneItem() const
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
