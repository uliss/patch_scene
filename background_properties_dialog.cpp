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

        ui->imgOffsetX->setRange(-2000, 2000);
        ui->imgOffsetX->setValue(item->x());

        ui->imgOffsetY->setRange(-2000, 2000);
        ui->imgOffsetY->setValue(item->y());

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
    }

    adjustSize();
}

BackgroundPropertiesDialog::~BackgroundPropertiesDialog()
{
    delete ui;
}
