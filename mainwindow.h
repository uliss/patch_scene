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

#include "app_settings.h"
#include "battery_item_model.h"
#include "connection_item_model.h"
#include "device_item_model.h"
#include "diagram.h"
#include "favorites_widget.h"
#include "return_item_model.h"
#include "send_item_model.h"

#include <QMainWindow>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableWidget>

class QToolButton;
class DeviceLibrary;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE


namespace ceam {
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

public slots:
    bool saveDocument();
    bool saveDocumentAs();
    void duplicateSelection();
    void exportToOdf();
    void exportToPdf();
    void openDocument();
    void openDocument(const QString& path);
    void printScheme();
    void selectAll();
    void setBackground();
    void showAbout();
    void showPreferences();
    void updateTitle();
    void documentProperties();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onAddToFavorites(SharedDeviceData data);
    void onBatteryChange(const BatteryChange& data);
    void onDeviceAdd(SharedDeviceData data);
    void onDeviceRemove(SharedDeviceData data);
    void onDeviceTitleUpdate(DeviceId id, const QString& title);
    void onDeviceUpdate(SharedDeviceData data);
    void onConnectionAdd(ConnectionData data);
    void onConnectionRemove(ConnectionData data);
    void onSceneChange();

private:
    void initActions();
    void initConnectionList();
    void initBatteryList();
    void initDeviceList();
    void initDiagram();
    void initLibrarySearch();
    void initReturnList();
    void initSendList();

    void setProjectName(const QString& fileName);
    bool doSave();
    void createToolbarScaleView();
    void resizePanels();

    QTextDocument* exportToDocument(const QSizeF& pageSize);

    void setupExpandButton(QToolButton* btn, QTableView* tab, QFrame* line, bool expanded = true);
    void setupDockTitle(QDockWidget* dock);

    void readPositionSettings();
    void writePositionSettings() const;

    void loadLibrary();
    void loadFavorites();
    void writeFavorites() const;

    void addRecentFile(const QUrl& file);
    void readRecentFiles();
    void writeRecentFiles() const;
    void syncRecentFilesMenu();
    void updateDeviceView(const SharedDeviceData& data, int idx);

    void loadSection(QStandardItem* parent, const QList<SharedDeviceData>& data);

private:
    Ui::MainWindow* ui;
    Diagram* diagram_;
    QString project_name_;
    QString file_name_;
    QSortFilterProxyModel* library_proxy_;
    DeviceItemModel* device_model_;
    ConnectionItemModel* conn_model_;
    SendItemModel* send_model_;
    ReturnItemModel* return_model_;
    BatteryItemModel* battery_model_;
    FavoritesWidget* favorites_;
    AppSettings settings_;
    QList<QUrl> recent_files_;
};
}

#endif // MAINWINDOW_H
