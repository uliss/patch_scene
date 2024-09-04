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
#include "about_window.h"
#include "app_version.h"
#include "ui_about_window.h"

#include <QLibraryInfo>
#include <QTextBrowser>
#include <QWheelEvent>

using namespace ceam;

namespace {
class PSCTextBrowser : public QTextBrowser {
public:
    explicit PSCTextBrowser(QWidget* parent)
        : QTextBrowser(parent)
    {
        setOpenExternalLinks(true);
        setOpenLinks(true);
    }

    void wheelEvent(QWheelEvent* event) final
    {
        if (event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier)) {
            // prevent zooming with mouse wheel
            event->accept();
        } else {
            QTextBrowser::wheelEvent(event);
        }
    }
};
}

AboutWindow::AboutWindow(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    auto general = new PSCTextBrowser(this);
    ui->tabWidget->addTab(general, tr("General"));

    general->setStyleSheet("background: palette(window)");
    general->setMarkdown(tr("![ceam_logo_color.png](:/ceam_logo_color.png)\n\n"
                            "**PatchScene** - software for drawing stage schemes and connections\n\n"
                            "Developed in **CEAM** (Center of electroacoustic music in Moscow Conservetory)\n\n"
                            "Version: %1 (build %2)\n\n"
                            "Copyright: Serge Poltavski, 2024\n\n")
                             .arg(app_version(), app_git_version()));

    auto license = new PSCTextBrowser(this);
    ui->tabWidget->addTab(license, tr("License"));

    license->setStyleSheet("background: palette(window)");
    license->setMarkdown(tr("Distrubuted under GPLv3 license\n\n"
                            "**SVG icons used from:**\n\n"
                            "- https://commons.wikimedia.org\n"
                            "- https://www.svgrepo.com/\n"));

    auto thanks = new PSCTextBrowser(this);
    ui->tabWidget->addTab(thanks, tr("Thanks"));

    thanks->setStyleSheet("background: palette(window)");
    thanks->setMarkdown(tr("**Thanks to:**\n\n"
                           "- Natasha Plotnikova\n\n"
                           "- Alex Nadzharov\n\n"
                           "- Ilya Kovalskiy\n\n"
                           "**Translations:**\n\n"
                           "_to Russian:_ Serge Poltavski"));

    auto contacts = new PSCTextBrowser(this);
    ui->tabWidget->addTab(contacts, tr("Contacts"));

    contacts->setStyleSheet("background: palette(window)");
    contacts->setMarkdown(tr("__Mail__\n\nserge.poltavski@gmail.com\n\n"
                             "__CEAM__\n\nhttp://ceam.space"));

    setWindowFlag(Qt::Tool, true);

    auto about_qt = new PSCTextBrowser(this);

#define QT_PATH_STR(p) QString("\n**" #p     \
                               "**: %1\n\n") \
                           .arg(QLibraryInfo::path(QLibraryInfo::p))

    about_qt->setStyleSheet("background: palette(window)");
    about_qt->setMarkdown(
        QString("**Qt version:** %1\n\n").arg(QT_VERSION_STR)
        + QT_PATH_STR(PrefixPath)
        + QT_PATH_STR(DocumentationPath)
        + QT_PATH_STR(HeadersPath)
        + QT_PATH_STR(LibrariesPath)
        + QT_PATH_STR(HeadersPath)
        + QT_PATH_STR(LibraryExecutablesPath)
        + QT_PATH_STR(BinariesPath)
        + QT_PATH_STR(PluginsPath)
        + QT_PATH_STR(ArchDataPath)
        + QT_PATH_STR(DataPath)
        + QT_PATH_STR(TranslationsPath)
        + QT_PATH_STR(SettingsPath));

    ui->tabWidget->addTab(about_qt, tr("About Qt"));

    setWindowFlag(Qt::Tool, true);
    // setModal(true);
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
