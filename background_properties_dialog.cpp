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

    if (bg_) {
        auto img_size = bg->imageRect().size();
        ui->imageSize->setText(QString("%1 x %2").arg(img_size.width()).arg(img_size.height()));

        ui->imgOffsetX->setRange(-2000, 2000);
        ui->imgOffsetX->setValue(bg_->x());

        ui->imgOffsetY->setRange(-2000, 2000);
        ui->imgOffsetY->setValue(bg_->y());

        auto view_size = bg_->viewSize();
        ui->viewWidth->setRange(100, 5000);
        ui->viewWidth->setValue(view_size.width());
        ui->viewHeight->setRange(100, 5000);
        ui->viewHeight->setValue(view_size.height());

        connect(ui->fitWidth, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitWidth);
        connect(ui->fitHeight, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitHeight);
        connect(ui->fitBest, &QPushButton::clicked, this, &BackgroundPropertiesDialog::fitBest);
        connect(ui->originalSize, &QPushButton::clicked, this, &BackgroundPropertiesDialog::resetSize);

        connect(ui->linkSize, &QToolButton::clicked, this, [this](bool on) {
            ui->linkSize->setIcon(QIcon(on ? ":/icons/link.svg" : ":/icons/link_off.svg"));
        });

        connect(ui->viewHeight, &QSpinBox::valueChanged, this, [this](int new_height) {
            auto bbox = bg_->imageRect();
            if (!bbox.isValid())
                return;

            if (ui->linkSize->isChecked()) {
                auto scale = new_height / bbox.height();
                auto new_width = qRound(bbox.width() * scale);
                emit sizeChanged({ ui->imgOffsetX->value(), ui->imgOffsetY->value(), new_width, new_height });
            } else {
                emit sizeChanged({ ui->imgOffsetX->value(), ui->imgOffsetY->value(), ui->viewWidth->value(), new_height });
            }
        });

        connect(ui->viewWidth, &QSpinBox::valueChanged, this, [this](int new_width) {
            auto bbox = bg_->imageRect();
            if (!bbox.isValid())
                return;

            if (ui->linkSize->isChecked()) {
                auto scale = new_width / bbox.width();
                auto new_height = qRound(bbox.height() * scale);
                emit sizeChanged({ ui->imgOffsetX->value(), ui->imgOffsetY->value(), new_width, new_height });
            } else {
                emit sizeChanged({ ui->imgOffsetX->value(), ui->imgOffsetY->value(), new_width, ui->viewHeight->value() });
            }
        });

        connect(ui->imgOffsetX, &QSpinBox::valueChanged, this, [this](int dx) {
            bg_->setPos(QPointF(dx, ui->imgOffsetY->value()));
        });

        connect(ui->imgOffsetY, &QSpinBox::valueChanged, this, [this](int dy) {
            bg_->setPos(QPointF(ui->imgOffsetX->value(), dy));
        });

        connect(this, &BackgroundPropertiesDialog::sizeChanged, this,
            [this](const QRect& rect) {
                QSignalBlocker sbw(ui->viewWidth);
                QSignalBlocker sbh(ui->viewHeight);
                QSignalBlocker sbx(ui->imgOffsetX);
                QSignalBlocker sby(ui->imgOffsetY);
                ui->viewWidth->setValue(rect.width());
                ui->viewHeight->setValue(rect.height());
                ui->imgOffsetX->setValue(rect.x());
                ui->imgOffsetY->setValue(rect.y());
                bg_->setSize(rect.size());
                bg_->setPos(rect.topLeft());
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
    auto scene_size = bg_->sceneSize();
    auto img_rect = bg_->imageRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();

    const qreal scale = std::min(
        scene_size.height() / img_size.height(),
        scene_size.width() / img_size.width());

    emit sizeChanged({ {}, (img_size * scale).toSize() });
}

void BackgroundPropertiesDialog::fitHeight()
{
    auto scene_height = bg_->sceneSize().height();
    auto img_rect = bg_->imageRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();
    auto scale = scene_height / img_size.height();
    emit sizeChanged({ {}, (img_size * scale).toSize() });
}

void BackgroundPropertiesDialog::fitWidth()
{
    auto scene_width = bg_->sceneSize().width();
    auto img_rect = bg_->imageRect();
    if (!img_rect.isValid())
        return;

    auto img_size = img_rect.size();
    auto scale = scene_width / img_size.width();
    emit sizeChanged({ {}, (img_size * scale).toSize() });
}

void BackgroundPropertiesDialog::resetSize()
{
    auto img_rect = bg_->imageRect();
    if (!img_rect.isValid())
        return;

    emit sizeChanged({ {}, img_rect.size().toSize() });
}
