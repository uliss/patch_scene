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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "diagram.h"

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableWidget>

class QToolButton;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    void duplicateSelection();
    void exportDocument();
    void openDocument();
    bool saveDocument();
    bool saveDocumentAs();
    void printScheme();
    void selectAll();
    void showAbout();
    void showPreferences();
    void updateTitle();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onDeviceAdd(SharedDeviceData data);
    void onDeviceRemove(SharedDeviceData data);
    void onDeviceUpdate(SharedDeviceData data);
    void onConnectionAdd(ConnectionData data);
    void onConnectionRemove(ConnectionData data);
    void onSceneChange();

private:
    void setProjectName(const QString& fileName);
    bool doSave();
    void loadLibraryDevices();
    void createToolbarScaleView();
    void resizePanels();

    void setupExpandButton(QToolButton* btn, QTableView* tab, QFrame* line);
    void setupEquipmentTableView(QTableView* tab, QStandardItemModel* model);
    void setupDockTitle(QDockWidget* dock);

    void readPositionSettings();
    void writePositionSettings();

private:
    Ui::MainWindow* ui;
    Diagram* diagram;
    QString project_name_;
    QString file_name_;
    QSortFilterProxyModel* library_proxy_;
    QStandardItemModel *device_model_, *conn_model_, *send_model_, *return_model_;

    // QAction* mac_app_show_;
    // QAction* mac_app_hide_;
};

#endif // MAINWINDOW_H
