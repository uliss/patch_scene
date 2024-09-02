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

static const QString DEFS_URL = "https://raw.githubusercontent.com/"
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
    connect(updater_, SIGNAL(appcastDownloaded(QString, QByteArray)), this, SLOT(displayAppcast(QString, QByteArray)));

    connect(ui->checkButton, SIGNAL(clicked()), this, SLOT(checkForUpdates()));
    // emit ui->checkButton->clicked();

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
    /* Get settings from the UI */
    // QString version = m_ui->installedVersion->text();
    // bool customAppcast = m_ui->customAppcast->isChecked();
    // bool downloaderEnabled = m_ui->enableDownloader->isChecked();
    // bool notifyOnFinish = m_ui->showAllNotifcations->isChecked();
    // bool notifyOnUpdate = m_ui->showUpdateNotifications->isChecked();
    // bool mandatoryUpdate = m_ui->mandatoryUpdate->isChecked();

    /* Apply the settings */
    updater_->setModuleVersion(DEFS_URL, app_version());
    updater_->setNotifyOnFinish(DEFS_URL, true);
    updater_->setNotifyOnUpdate(DEFS_URL, true);
    // updater_->setUseCustomAppcast(DEFS_URL, true);
    updater_->setDownloaderEnabled(DEFS_URL, true);
    updater_->setMandatoryUpdate(DEFS_URL, true);

    /* Check for updates */
    updater_->checkForUpdates(DEFS_URL);
}

void AppUpdaterWindow::updateChangelog(const QString& url)
{
    if (url == DEFS_URL)
        ui->changelogText->setPlainText(updater_->getChangelog(url));
}

void AppUpdaterWindow::displayAppcast(const QString& url, const QByteArray& reply)
{
    if (url == DEFS_URL) {
        QString text = "This is the downloaded appcast: <p><pre>" + QString::fromUtf8(reply)
            + "</pre></p><p> If you need to store more information on the "
              "appcast (or use another format), just use the "
              "<b>QSimpleUpdater::setCustomAppcast()</b> function. "
              "It allows your application to interpret the appcast "
              "using your code and not QSU's code.</p>";

        ui->changelogText->setPlainText(text);
    }
}

} // namespace ceam
