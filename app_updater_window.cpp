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
#include "app_updater_window.h"
#include "app_version.h"
#include "ui_app_updater_window.h"

namespace ceam {

static const char* DEFS_URL = "https://raw.githubusercontent.com/"
                              "uliss/patch_scene/master/resources/updates.json";

AppUpdaterWindow::AppUpdaterWindow(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AppUpdaterWindow)
{
    ui->setupUi(this);

    /* QSimpleUpdater is single-instance */
    updater_ = QSimpleUpdater::getInstance();

    /* Check for updates when the "Check For Updates" button is clicked */
    connect(updater_, SIGNAL(checkingFinished(QString)), this, SLOT(updateChangelog(QString)));

    connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(checkForUpdates()));

    /* Resize the dialog to fit */
    setMinimumSize(minimumSizeHint());
    resize(minimumSizeHint());
}

AppUpdaterWindow::~AppUpdaterWindow()
{
    delete ui;
}

void AppUpdaterWindow::checkForUpdates()
{
    updater_->setModuleVersion(DEFS_URL, app_version());
    updater_->setNotifyOnFinish(DEFS_URL, true);
    updater_->setNotifyOnUpdate(DEFS_URL, true);
    updater_->setDownloaderEnabled(DEFS_URL, true);
    updater_->setMandatoryUpdate(DEFS_URL, false);

    /* Check for updates */
    updater_->checkForUpdates(DEFS_URL);
}

void AppUpdaterWindow::updateChangelog(const QString& url)
{
    if (url == DEFS_URL)
        ui->changelogText->setPlainText(updater_->getChangelog(url));
}

} // namespace ceam
