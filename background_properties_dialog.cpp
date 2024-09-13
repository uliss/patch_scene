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
        ui->imageSize->setText(QString("%1px x %2px").arg(img_rect.width()).arg(img_rect.height()));

        auto scene = item->scene();
        if (scene) {
            auto rect = scene->sceneRect();
            ui->sceneSize->setText(QString("%1px x %2px").arg(rect.width()).arg(rect.height()));
        }

        auto xoff = item->x() - img_rect.width() * 0.5;
        auto yoff = item->y() - img_rect.height() * 0.5;

        ui->imgOffsetX->setRange(-2000, 2000);
        ui->imgOffsetX->setValue(xoff);

        ui->imgOffsetY->setRange(-2000, 2000);
        ui->imgOffsetY->setValue(yoff);

        ui->viewWidth->setRange(100, 5000);
        ui->viewWidth->setValue(img_rect.width());
        ui->viewHeight->setRange(100, 5000);
        ui->viewHeight->setValue(img_rect.height());

        connect(ui->fitWidthBtn, &QPushButton::clicked, this, [this]() {
            auto item = bg_->sceneItem();
            auto scene_width = item->scene()->sceneRect().width();
            auto image_width = item->boundingRect().width();
            auto image_height = item->boundingRect().height();
            auto scale = scene_width / image_width;
            item->setScale(scale);
            item->setPos(-scene_width / 2, (-image_height * scale) / 2);
        });

        connect(ui->linkSize, &QToolButton::clicked, this, [this](bool on) {
            ui->linkSize->setIcon(QIcon(on ? ":/icons/link.svg" : ":/icons/link_off.svg"));
        });

        connect(ui->viewHeight, &QSpinBox::valueChanged, this, [this](int new_height) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();

            if (ui->linkSize->isChecked()) {
                auto scale = new_height / bbox.height();
                auto new_width = qRound(bbox.width() * scale);
                QSignalBlocker sb(ui->viewWidth);
                ui->viewWidth->setValue(new_width);
                emit sizeChanged({ new_width, new_height });
            } else {
                emit sizeChanged(QSize(bbox.width(), new_height));
            }
        });

        connect(ui->viewWidth, &QSpinBox::valueChanged, this, [this](int new_width) {
            auto item = bg_->sceneItem();
            auto bbox = item->boundingRect();

            if (ui->linkSize->isChecked()) {
                auto scale = new_width / bbox.width();
                auto new_height = qRound(bbox.height() * scale);
                QSignalBlocker sb(ui->viewHeight);
                ui->viewHeight->setValue(new_height);
                emit sizeChanged({ new_width, new_height });
            } else {
                emit sizeChanged(QSize(new_width, bbox.height()));
            }
        });
    }

    adjustSize();
}

BackgroundPropertiesDialog::~BackgroundPropertiesDialog()
{
    delete ui;
}
