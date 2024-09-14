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
#include "background_properties_dialog.h"
#include "scene_background.h"
#include "ui_background_properties_dialog.h"

#include <QGraphicsScene>

using namespace ceam;

BackgroundPropertiesDialog::BackgroundPropertiesDialog(SceneBackground* bg, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::BackgroundPropertiesDialog)
    , bg_(bg)
{
    ui->setupUi(this);

    if (bg_ && bg_->sceneItem()) {
        auto item = bg_->sceneItem();

        auto img_rect = item->boundingRect();
        img_rect.setSize(item->transform().mapRect(img_rect).size());
        ui->imageSize->setText(QString("%1px x %2px").arg(img_rect.width()).arg(img_rect.height()));

        auto xoff = item->pos().x() + centerXCorrection();
        auto yoff = item->pos().y() + centerYCorrection();

        ui->imgOffsetX->setRange(-2000, 2000);
        ui->imgOffsetX->setValue(xoff);

        ui->imgOffsetY->setRange(-2000, 2000);
        ui->imgOffsetY->setValue(yoff);

        ui->viewWidth->setRange(100, 5000);
        ui->viewWidth->setValue(img_rect.width());
        ui->viewHeight->setRange(100, 5000);
        ui->viewHeight->setValue(img_rect.height());

        connect(ui->fitWidth, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitWidth);
        connect(ui->fitHeight, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitHeight);
        connect(ui->fitBest, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitBest);
        connect(ui->originalSize, &QPushButton::clicked, this, &BackgroundPropertiesDialog::resetSize);

        connect(ui->linkSize, &QToolButton::clicked, this, [this](bool on) {
            ui->linkSize->setIcon(QIcon(on ? ":/icons/link.svg" : ":/icons/link_off.svg"));
        });

        connect(ui->viewHeight, &QSpinBox::valueChanged, this, [this](int new_height) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();
            if (!bbox.isValid())
                return;

            if (ui->linkSize->isChecked()) {
                auto scale = new_height / bbox.height();
                auto new_width = qRound(bbox.width() * scale);
                emit sizeChanged({ new_width, new_height });
            } else {
                emit sizeChanged(QSize(bbox.width(), new_height));
            }
        });

        connect(ui->viewWidth, &QSpinBox::valueChanged, this, [this](int new_width) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();
            if (!bbox.isValid())
                return;

            if (ui->linkSize->isChecked()) {
                auto scale = new_width / bbox.width();
                auto new_height = qRound(bbox.height() * scale);
                emit sizeChanged({ new_width, new_height });
            } else {
                emit sizeChanged(QSize(new_width, bbox.height()));
            }
        });

        connect(ui->imgOffsetX, &QSpinBox::valueChanged, this, [this](int dx) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();
            if (!bbox.isValid())
                return;

            auto xoff = dx - centerXCorrection();
            item->setPos(xoff, item->y());
        });

        connect(ui->imgOffsetY, &QSpinBox::valueChanged, this, [this](int dy) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();
            if (!bbox.isValid())
                return;

            auto yoff = dy - centerYCorrection();
            item->setPos(item->x(), yoff);
        });

        connect(this, &BackgroundPropertiesDialog::sizeChanged, this,
            [this](const QSize& sz) {
                QSignalBlocker sbw(ui->viewWidth);
                QSignalBlocker sbh(ui->viewHeight);
                ui->viewWidth->setValue(sz.width());
                ui->viewHeight->setValue(sz.height());
                bg_->setSize(sz);
                bg_->setPos({ -sz.width() * 0.5, -sz.height() * 0.5 });
            });
    }

    adjustSize();
}

BackgroundPropertiesDialog::~BackgroundPropertiesDialog()
{
    delete ui;
}

void BackgroundPropertiesDialog::fitBest()
{
    auto item = bg_->sceneItem();
    auto scene_size = item->scene()->sceneRect().size();
    auto img_rect = item->boundingRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();

    const qreal scale = std::min(
        scene_size.height() / img_size.height(),
        scene_size.width() / img_size.width());

    emit sizeChanged((img_size * scale).toSize());
}

void BackgroundPropertiesDialog::fitHeight()
{
    auto item = bg_->sceneItem();
    auto scene_height = item->scene()->sceneRect().height();
    auto img_rect = item->boundingRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();

    auto scale = scene_height / img_size.height();
    emit sizeChanged((img_size * scale).toSize());
}

void BackgroundPropertiesDialog::fitWidth()
{
    auto item = bg_->sceneItem();
    auto scene_width = item->scene()->sceneRect().width();
    auto img_rect = item->boundingRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();

    auto scale = scene_width / img_size.width();
    emit sizeChanged((img_size * scale).toSize());
}

void BackgroundPropertiesDialog::resetSize()
{
    auto item = bg_->sceneItem();
    auto img_rect = item->boundingRect();
    if (!img_rect.isValid())
        return;

    item->setTransform({});
    item->setPos(-img_rect.width() * 0.5, -img_rect.height() * 0.5);
}

qreal BackgroundPropertiesDialog::centerXCorrection() const
{
    auto item = bg_->sceneItem();
    if (!item)
        return 0;

    return item->transform().mapRect(item->boundingRect()).width() * 0.5;
}

qreal BackgroundPropertiesDialog::centerYCorrection() const
{
    auto item = bg_->sceneItem();
    if (!item)
        return 0;

    return item->transform().mapRect(item->boundingRect()).height() * 0.5;
}
