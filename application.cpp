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
#include <QCommandLineParser>
#include <QFileOpenEvent>
#include <QIcon>
#include <QLibraryInfo>
#include <QLoggingCategory>
#include <QTranslator>

using namespace ceam;

PatchSceneApp::PatchSceneApp(int& argc, char** argv, int flags)
    : QApplication(argc, argv, flags)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("PatchScene - helper application to draw connection schemes");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", tr("PatchScene file to open"));

    QCommandLineOption verbose_opt(QStringList() << "V"
                                                 << "verbose",
        tr("Vebose output"));
    parser.addOption(verbose_opt);

    QCommandLineOption debug_opt(QStringList()
            << "debug",
        tr("Debug output"));
    parser.addOption(debug_opt);

    QCommandLineOption lang_opt(QStringList() << "L"
                                              << "lang",
        tr("Select language"), "lang");
    parser.addOption(lang_opt);

    parser.process(*this);

    if (parser.isSet(debug_opt)) {
        QLoggingCategory::setFilterRules("*.debug=true");
        QLoggingCategory::setFilterRules("qt*=false");
    } else {
        QLoggingCategory::setFilterRules("*.debug=false");
    }

    if (qt_translator_.load(QLocale::system(), "qt", "_",
            QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {

        if (installTranslator(&qt_translator_))
            qDebug() << "qt tr added:" << qt_translator_.filePath();
    }

    if (parser.value(lang_opt).isEmpty()) {
        if (translator_.load(QLocale(), "patch_scene", QLatin1String("_"), QLatin1String(":/i18n"))) {
            if (installTranslator(&translator_))
                qDebug() << "app tr added:" << translator_.filePath() << translator_.language();
        }
    } else if (translator_.load("patch_scene", QLatin1String(":/i18n"), {}, parser.value(lang_opt))) {
        if (installTranslator(&translator_))
            qDebug() << "app tr added:" << translator_.filePath() << translator_.language();
    }

    qDebug() << "APP_DIR:       " << applicationDirPath();
    qDebug() << "current cpu:   " << QSysInfo::currentCpuArchitecture();
    qDebug() << "kernel type:   " << QSysInfo::kernelType();
    qDebug() << "kernel version:" << QSysInfo::kernelVersion();
    qDebug() << "Qt build:      " << QLibraryInfo::build();

#define PRINT_QT_PATH(p)                                 \
    {                                                    \
        qDebug() << "Qt " #p ":"                         \
                 << QLibraryInfo::path(QLibraryInfo::p); \
    }

    PRINT_QT_PATH(PrefixPath);
    PRINT_QT_PATH(DocumentationPath);
    PRINT_QT_PATH(HeadersPath);
    PRINT_QT_PATH(LibrariesPath);
    PRINT_QT_PATH(LibraryExecutablesPath);
    PRINT_QT_PATH(BinariesPath);
    PRINT_QT_PATH(PluginsPath);
    PRINT_QT_PATH(ArchDataPath);
    PRINT_QT_PATH(DataPath);
    PRINT_QT_PATH(TranslationsPath);
    PRINT_QT_PATH(SettingsPath);

    setWindowIcon(QIcon(":/app_icon.svg"));
    window_.reset(new MainWindow);
    window_->show();

    if (parser.positionalArguments().size() > 0)
        window_->openDocument(parser.positionalArguments().at(0));
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
