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
#ifndef BACKGROUND_PROPERTIES_DIALOG_H
#define BACKGROUND_PROPERTIES_DIALOG_H

#include <QDialog>

namespace Ui {
class BackgroundPropertiesDialog;
}

namespace ceam {

class SceneBackground;

class BackgroundPropertiesDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackgroundPropertiesDialog(SceneBackground* bg, QWidget* parent = nullptr);
    ~BackgroundPropertiesDialog();

signals:
    void sizeChanged(QSize);

public slots:
    void fitBest();
    void fitHeight();
    void fitWidth();
    void resetSize();

private:
    qreal centerXCorrection() const;
    qreal centerYCorrection() const;

private:
    Ui::BackgroundPropertiesDialog* ui;
    SceneBackground* bg_;
};

}

#endif // BACKGROUND_PROPERTIES_DIALOG_H
