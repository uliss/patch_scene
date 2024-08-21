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
#include "mainwindow.h"

#include <QApplication>
#include <QFileOpenEvent>
#include <QLibraryInfo>
#include <QTranslator>

class PatchSceneApp : public QApplication {
public:
    PatchSceneApp(int& argc, char** argv, int flags = ApplicationFlags)
        : QApplication(argc, argv, flags)
    {
        setWindowIcon(QIcon(":/app_icon.svg"));

        QTranslator qt_tr;

        qDebug() << QLocale::system();

        if (qt_tr.load(QLocale::system(), "qt", "_",
                QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {

            if (installTranslator(&qt_tr))
                qDebug() << "qt tr added:" << qt_tr.filePath();
        }

        QTranslator my_tr;
        if (my_tr.load(QLocale(), "patch_scene", QLatin1String("_"), QLatin1String(":/i18n"))) {
            if (installTranslator(&my_tr))
                qDebug() << "app tr added:" << my_tr.filePath() << my_tr.language();
        }

        window_.show();

        if (argc > 1 && argv[1])
            window_.openDocument(QString(argv[1]));
    }

signals:
    void openFile(const QString& fn);

protected:
    bool event(QEvent* event) final
    {
        if (event->type() == QEvent::FileOpen) {
            auto open_event = dynamic_cast<QFileOpenEvent*>(event);
            if (open_event) {
                window_.openDocument(open_event->file());
                return true;
            }
        }

        return QApplication::event(event);
    }

private:
    ceam::MainWindow window_;
};

int main(int argc, char* argv[])
{
    PatchSceneApp app(argc, argv);
    return app.exec();
}
