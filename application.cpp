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
#include "application.h"

#include <QApplication>
#include <QFileOpenEvent>
#include <QIcon>
#include <QLibraryInfo>
#include <QTranslator>

using namespace ceam;

PatchSceneApp::PatchSceneApp(int& argc, char** argv, int flags)
    : QApplication(argc, argv, flags)
{
    setWindowIcon(QIcon(":/app_icon.svg"));

    qDebug() << QLocale::system();

    if (qt_translator_.load(QLocale::system(), "qt", "_",
            QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {

        if (installTranslator(&qt_translator_))
            qDebug() << "qt tr added:" << qt_translator_.filePath();
    }

    if (translator_.load(QLocale(), "patch_scene", QLatin1String("_"), QLatin1String(":/i18n"))) {
        if (installTranslator(&translator_))
            qDebug() << "app tr added:" << translator_.filePath() << translator_.language();
    }

    window_.reset(new MainWindow);
    window_->show();

    if (argc > 1 && argv[1])
        window_->openDocument(QString(argv[1]));
}

bool PatchSceneApp::event(QEvent* event)
{
    if (event->type() == QEvent::FileOpen) {
        auto open_event = dynamic_cast<QFileOpenEvent*>(event);
        if (window_ && open_event) {
            window_->openDocument(open_event->file());
            return true;
        }
    }

    return QApplication::event(event);
}
