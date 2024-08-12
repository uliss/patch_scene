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
#include "about_window.h"
#include "device_library.h"
#include "export_document.h"
#include "preferences_dialog.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextDocumentWriter>
#include <QTextTable>

constexpr const char* SETTINGS_ORG = "space.ceam";
constexpr const char* SETTINGS_APP = "PatchScene";

constexpr const char* SKEY_MAINWINDOW = "mainwindow";
constexpr const char* SKEY_GEOMETRY = "geometry";
constexpr const char* SKEY_SAVESTATE = "savestate";
constexpr const char* SKEY_MAXIMIZED = "maximized";
constexpr const char* SKEY_POS = "pos";
constexpr const char* SKEY_SIZE = "size";

constexpr int DATA_DEVICE_DATA = Qt::UserRole + 1;
constexpr int DATA_DEVICE_ID = Qt::UserRole + 2;
constexpr int DATA_CONNECTION = Qt::UserRole + 3;

constexpr int COL_CONN_SRC_NAME = 0;
constexpr int COL_CONN_DEST_NAME = 3;
constexpr int COL_SEND_DEV_NAME = 2;
constexpr int DATA_CONN_NCOLS = 6;

enum DeviceColumnOrder {
    COL_DEV_TITLE = 0,
    COL_DEV_VENDOR,
    COL_DEV_MODEL,
};
constexpr int DATA_DEV_NCOLS = 3;

class DeviceItemModel : public QStandardItemModel {
public:
    DeviceItemModel(QObject* parent = nullptr)
        : QStandardItemModel(parent)
    {
    }

    QMimeData* mimeData(const QModelIndexList& indexes) const override
    {
        QMimeData* data = new QMimeData();

        for (auto& idx : indexes) {
            if (idx.column() == 0)
                data->setText(idx.data(DATA_DEVICE_DATA).toString());
        }

        return data;
    }
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStatusBar(new QStatusBar);
    // createToolbarScaleView();

#ifdef Q_OS_DARWIN
    // mac_app_hide_ = new QAction(tr("Hide"), this);
    // mac_app_show_ = new QAction(tr("Show"), this);
#endif

    setupDockTitle(ui->libraryDock);
    setupDockTitle(ui->tableDock);

    device_model_ = new QStandardItemModel(0, DATA_DEV_NCOLS, this);
    device_model_->setHorizontalHeaderLabels({ tr("Name"), tr("Vendor"), tr("Model") });
    setupEquipmentTableView(ui->deviceList, device_model_);
    connect(ui->deviceList, &QTableView::clicked, this, [this](const QModelIndex& index) {
        if (index.column() == COL_DEV_TITLE) {
            bool ok = false;
            auto id = index.data(DATA_DEVICE_ID).toInt(&ok);
            if (ok)
                diagram_->cmdSelectUnique(id);
        }
    });
    ui->deviceList->resizeColumnsToContents();
    connect(device_model_, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item) {
        if (!item)
            return;

        bool ok = false;
        auto id = item->data(DATA_DEVICE_ID).toInt(&ok);
        if (ok) {
            auto dev = diagram_->findDeviceById(id);
            if (dev) {

                auto data = dev->deviceData();

                switch (item->column()) {
                case COL_DEV_TITLE:
                    qWarning() << "update title";
                    data->setTitle(item->text());
                    break;
                case COL_DEV_VENDOR:
                    qWarning() << "update vendor";
                    data->setVendor(item->text());
                    break;
                case COL_DEV_MODEL:
                    qWarning() << "update model";
                    data->setModel(item->text());
                    break;
                default:
                    qWarning() << "unknown column:" << item->column();
                    return;
                }

                diagram_->cmdUpdateDevice(data);
            } else {
                qWarning() << "device not found:" << (int)id;
            }
        } else {
            qWarning() << "id property not found";
        }
    });

    conn_model_ = new QStandardItemModel(0, DATA_CONN_NCOLS, this);
    conn_model_->setHorizontalHeaderLabels({ tr("Source"), tr("Model"), tr("Plug"), tr("Destination"), tr("Model"), tr("Plug") });
    setupEquipmentTableView(ui->connectionList, conn_model_);
    connect(ui->connectionList, &QTableView::clicked, this, [this](const QModelIndex& index) {
        if (index.column() == COL_CONN_SRC_NAME || index.column() == COL_CONN_DEST_NAME) {
            bool ok = false;
            auto id = index.data(DATA_DEVICE_ID).toInt(&ok);
            if (ok)
                diagram_->cmdSelectUnique(id);
        }
    });
    ui->connectionList->resizeColumnsToContents();

    send_model_ = new QStandardItemModel(0, 2, this);
    send_model_->setHorizontalHeaderLabels({ tr("Send"), tr("Input"), tr("Device"), tr("Output") });
    setupEquipmentTableView(ui->returnList, send_model_);
    ui->returnList->resizeColumnsToContents();

    return_model_ = new QStandardItemModel(0, 2, this);
    return_model_->setHorizontalHeaderLabels({ tr("Return"), tr("Output"), tr("Device"), tr("Input") });
    setupEquipmentTableView(ui->sendList, return_model_);
    ui->sendList->resizeColumnsToContents();

    setupExpandButton(ui->deviceListBtn, ui->deviceList, ui->deviceListLine);
    setupExpandButton(ui->connectionListBtn, ui->connectionList, ui->connectionListLine);
    setupExpandButton(ui->sendListBtn, ui->sendList, ui->sendListLine);
    setupExpandButton(ui->returnListBtn, ui->returnList, ui->returnListLine);

    diagram_ = new Diagram();
    ui->gridLayout->addWidget(diagram_, 1, 1);
    connect(diagram_, SIGNAL(sceneChanged()), this, SLOT(onSceneChange()));
    connect(diagram_, SIGNAL(deviceAdded(SharedDeviceData)), this, SLOT(onDeviceAdd(SharedDeviceData)));
    connect(diagram_, SIGNAL(deviceRemoved(SharedDeviceData)), this, SLOT(onDeviceRemove(SharedDeviceData)));
    connect(diagram_, SIGNAL(deviceUpdated(SharedDeviceData)), this, SLOT(onDeviceUpdate(SharedDeviceData)));
    connect(diagram_, SIGNAL(connectionAdded(ConnectionData)), this, SLOT(onConnectionAdd(ConnectionData)));
    connect(diagram_, SIGNAL(connectionRemoved(ConnectionData)), this, SLOT(onConnectionRemove(ConnectionData)));

    connect(diagram_, &Diagram::canRedoChanged, this, [this](bool value) { ui->actionRedo->setEnabled(value); });
    connect(diagram_, &Diagram::canUndoChanged, this, [this](bool value) { ui->actionUndo->setEnabled(value); });

    connect(ui->actionAboutApp, SIGNAL(triggered(bool)), this, SLOT(showAbout()));
    connect(ui->actionCopy, SIGNAL(triggered()), diagram_, SLOT(copySelected()));
    connect(ui->actionCut, SIGNAL(triggered()), diagram_, SLOT(cutSelected()));
    connect(ui->actionPaste, SIGNAL(triggered()), diagram_, SLOT(paste()));
    connect(ui->actionDuplicate, SIGNAL(triggered()), this, SLOT(duplicateSelection()));
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openDocument()));
    connect(ui->actionPreferences, SIGNAL(triggered(bool)), this, SLOT(showPreferences()));
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(printScheme()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveDocument()));
    connect(ui->actionSelect_All, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSetBackground, SIGNAL(triggered(bool)), this, SLOT(setBackground()));

    connect(ui->actionShowCables, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowCables(value);
    });
    connect(ui->actionShowBackground, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowBackground(value);
    });
    connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(exportDocument()));

    // zoom
    connect(ui->actionZoomIn, SIGNAL(triggered()), diagram_, SLOT(zoomIn()));
    connect(ui->actionZoomNormal, SIGNAL(triggered()), diagram_, SLOT(zoomNormal()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), diagram_, SLOT(zoomOut()));
    connect(diagram_, &Diagram::zoomChanged, this, [this](qreal z) {
        statusBar()->showMessage(tr("Zoom %1%").arg(qRound(z * 100)), 1000);
    });

    connect(ui->librarySearch, &QLineEdit::textChanged, this, [this](const QString& txt) {
        library_proxy_->setFilterRegularExpression(txt);

        if (!txt.isEmpty())
            ui->libraryTree->expandAll();
    });

    connect(ui->actionRedo, SIGNAL(triggered()), diagram_, SLOT(redo()));
    connect(ui->actionUndo, SIGNAL(triggered()), diagram_, SLOT(undo()));

    ui->librarySearch->setStatusTip(tr("search in library"));
    ui->librarySearch->setClearButtonEnabled(true);
    ui->librarySearch->addAction(QIcon(":/ceam/cables/resources/search_02.svg"), QLineEdit::LeadingPosition);
    ui->librarySearch->setStyleSheet("QLineEdit {border-width: 1px;}");

#ifndef Q_OS_DARWIN
    ui->actionZoomIn->setIconVisibleInMenu(true);
    ui->actionZoomOut->setIconVisibleInMenu(true);
#else
    setUnifiedTitleAndToolBarOnMac(true);

    {
        auto font = ui->librarySearch->font();
        font.setPointSize(font.pointSize() - 2);
        ui->librarySearch->setFont(font);
        ui->librarySearch->setMaximumHeight(16);
    }
#endif

    resizePanels();

    updateTitle();

    loadLibraryDevices();

    readPositionSettings();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTitle()
{
    if (project_name_.isEmpty()) {
        setWindowTitle(tr("New Project - PatchScene"));
        setWindowModified(true);
    } else {
        setWindowTitle(tr("%1 - PatchScene").arg(project_name_));
    }
}

void MainWindow::setBackground()
{
    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    auto filename = QFileDialog::getOpenFileName(this,
        tr("Open image"),
        path,
        tr("Image files (*.JPG *.jpg *.jpeg *.PNG *.png *.SVG *.svg)"));
    diagram_->setBackground(filename);
}

void MainWindow::showAbout()
{
    auto about = new AboutWindow(this);
    about->show();
}

void MainWindow::showPreferences()
{
    auto prefs = new PreferencesDialog(this);
    prefs->show();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (isWindowModified()) {
        // auto x = new MacWarningDialog(tr("Warning"),
        //     tr("Document is not saved!\n"
        //        "Do you wan't to save it before closing?"), this);
        // x->execNativeDialogLater();

        auto btn = QMessageBox::question(this,
            tr("Warning"),
            tr("Document is not saved!\n"
               "Do you wan't to save it before closing?"),
            QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel));

        switch (btn) {
        case QMessageBox::Yes:
            if (!saveDocument())
                return event->ignore();

            break;
        case QMessageBox::No:
            break;
        default:
            event->ignore();
            return;
        }
        event->ignore();
    }

    writePositionSettings();

    QMainWindow::closeEvent(event);
}

void MainWindow::onDeviceAdd(SharedDeviceData data)
{
    if (data->category() != ItemCategory::Device)
        return;

    auto title = new QStandardItem(data->title());
    title->setData(data->id(), DATA_DEVICE_ID);
    title->setEditable(true);

    auto vendor = new QStandardItem(data->vendor());
    vendor->setData(data->id(), DATA_DEVICE_ID);
    vendor->setEditable(true);

    auto model = new QStandardItem(data->model());
    model->setData(data->id(), DATA_DEVICE_ID);
    model->setEditable(true);

    device_model_->appendRow({ title, vendor, model });
    ui->deviceList->resizeColumnsToContents();
}

void MainWindow::onDeviceRemove(SharedDeviceData data)
{
    if (data->category() != ItemCategory::Device)
        return;

    for (int i = 0; i < device_model_->rowCount(); i++) {
        auto item = device_model_->item(i, COL_DEV_TITLE);
        if (item && item->data(DATA_DEVICE_ID) == data->id()) {
            device_model_->removeRow(i);
            ui->deviceList->resizeColumnsToContents();
            break;
        }
    }
}

static bool updateItemDeviceName(QStandardItem* item, const SharedDeviceData& data)
{
    if (!item)
        return false;

    bool ok = false;
    auto dev_id = item->data(DATA_DEVICE_ID).toInt(&ok);
    if (dev_id && dev_id == data->id()) {
        if (data->title() != item->text())
            item->setText(data->title());

        return true;
    } else
        return false;
}

void MainWindow::onDeviceUpdate(SharedDeviceData data)
{
    if (!data) {
        qWarning() << "invalid data";
        return;
    }

    bool device_found = false;
    bool title_update = false;
    for (int i = 0; i < device_model_->rowCount(); i++) {
        auto title = device_model_->item(i, COL_DEV_TITLE);
        if (title && title->data(DATA_DEVICE_ID) == data->id()) {
            QSignalBlocker block(device_model_);

            if (data->category() != ItemCategory::Device) {
                device_model_->removeRow(i);
            } else {
                title_update = (title->text() != data->title());

                if (title_update) {
                    title->setText(data->title());
                    ui->deviceList->update(title->index());
                }

                auto vendor = device_model_->item(i, COL_DEV_VENDOR);
                if (vendor && vendor->text() != data->vendor()) {
                    vendor->setText(data->vendor());
                    ui->deviceList->update(vendor->index());
                }

                auto model = device_model_->item(i, COL_DEV_MODEL);
                if (model && model->text() != data->model()) {
                    model->setText(data->model());
                    ui->deviceList->update(model->index());
                }

                device_found = true;
            }

            ui->deviceList->resizeColumnsToContents();
            break;
        }
    }

    // not found in model
    if (!device_found && data->category() == ItemCategory::Device) {
        onDeviceAdd(data);
    } else if (title_update) {
        // update connection device name change
        for (int i = 0; i < conn_model_->rowCount(); i++) {
            if (updateItemDeviceName(conn_model_->item(i, COL_CONN_SRC_NAME), data))
                continue;

            if (updateItemDeviceName(conn_model_->item(i, COL_CONN_DEST_NAME), data))
                continue;
        }

        // update connection device name change
        for (int i = 0; i < send_model_->rowCount(); i++)
            updateItemDeviceName(send_model_->item(i, COL_SEND_DEV_NAME), data);
    }
}

void MainWindow::onConnectionAdd(ConnectionData data)
{
    auto conn2str = [](ConnectorType t) {
        switch (t) {
        case ConnectorType::Socket_Male:
            return tr("female");
        case ConnectorType::Socket_Female:
            return tr("male");
        default:
            return QString {};
        }
    };

    XletData src, dest;
    Device *src_dev = nullptr, *dest_dev = nullptr;
    if (diagram_->findConnectionXletData(data, src, dest, &src_dev, &dest_dev)) {
        auto src_name = new QStandardItem(src_dev->deviceData()->title());
        src_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
        src_name->setData(data.src, DATA_DEVICE_ID);
        src_name->setEditable(false);
        auto src_model = new QStandardItem(src.modelString());
        src_model->setEditable(false);
        auto src_plug = new QStandardItem(conn2str(src.type));
        src_plug->setEditable(false);
        auto dest_name = new QStandardItem(dest_dev->deviceData()->title());
        dest_name->setData(data.dest, DATA_DEVICE_ID);
        dest_name->setEditable(false);
        auto dest_model = new QStandardItem(dest.modelString());
        dest_model->setEditable(false);
        auto dest_plug = new QStandardItem(conn2str(dest.type));
        dest_plug->setEditable(false);

        conn_model_->appendRow({ src_name, src_model, src_plug, dest_name, dest_model, dest_plug });
        ui->connectionList->resizeColumnsToContents();

        if (dest_dev->deviceData()->category() == ItemCategory::Send) {
            auto src_idx = new QStandardItem(QString("%1").arg((int)data.out + 1));
            auto dest_idx = new QStandardItem(QString("%1").arg((int)data.in + 1));
            send_model_->appendRow({ dest_name->clone(), dest_idx, src_name->clone(), src_idx });
            ui->returnList->resizeColumnsToContents();
        }

        if (src_dev->deviceData()->category() == ItemCategory::Return) {
            auto src_idx = new QStandardItem(QString("%1").arg((int)data.out + 1));
            auto dest_idx = new QStandardItem(QString("%1").arg((int)data.in + 1));
            return_model_->appendRow({ src_name->clone(), src_idx, dest_name->clone(), dest_idx });
            ui->sendList->resizeColumnsToContents();
        }
    }
}

void MainWindow::onConnectionRemove(ConnectionData data)
{
    for (int i = 0; i < conn_model_->rowCount(); i++) {
        auto item = conn_model_->item(i, COL_CONN_SRC_NAME);
        if (item && item->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            conn_model_->removeRow(i);
        }
    }

    for (int i = 0; i < send_model_->rowCount(); i++) {
        auto item = send_model_->item(i, COL_SEND_DEV_NAME);
        if (item && item->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            send_model_->removeRow(i);
        }
    }

    for (int i = 0; i < return_model_->rowCount(); i++) {
        auto item = return_model_->item(i, 0);
        if (item && item->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            return_model_->removeRow(i);
        }
    }
}

void MainWindow::onSceneChange()
{
    setWindowModified(true);
}

void MainWindow::setProjectName(const QString& fileName)
{
    project_name_ = QFileInfo(fileName).baseName();
    file_name_ = fileName;
}

bool MainWindow::doSave()
{
    QFile file(file_name_);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "can't open" << file_name_;
        return false;
    }

    QJsonDocument doc(diagram_->toJson());
    file.write(doc.toJson());

    setProjectName(file.fileName());
    updateTitle();
    setWindowModified(false);

    auto log_msg = QString("Save to '%1'").arg(file_name_);
    qDebug() << log_msg;
    statusBar()->showMessage(log_msg, 1000);
    return true;
}

void MainWindow::loadLibraryDevices()
{
    auto model = new DeviceItemModel(this);
    auto parentItem = model->invisibleRootItem();

    ui->libraryTree->setDragEnabled(true);
    ui->libraryTree->setDragDropMode(QAbstractItemView::DragOnly);
    ui->libraryTree->setSortingEnabled(true);
    ui->libraryTree->sortByColumn(0, Qt::AscendingOrder);

    auto devices = new QStandardItem(tr("devices"));
    devices->setEditable(false);
    parentItem->appendRow(devices);

    DeviceLibrary dev_lib;
    if (dev_lib.readFile("://ceam/cables/resources/library.json")) {
        for (auto& dev : dev_lib.devices()) {
            auto item = new QStandardItem(dev->title());
            item->setEditable(false);

            QJsonDocument doc(dev->toJson());
            item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
            item->setDropEnabled(false);
            devices->appendRow(item);
        }
    }

    library_proxy_ = new QSortFilterProxyModel(this);
    library_proxy_->setSourceModel(model);
    library_proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    library_proxy_->setAutoAcceptChildRows(true);
    library_proxy_->setRecursiveFilteringEnabled(true);
    library_proxy_->setSortLocaleAware(true);
    ui->libraryTree->setModel(library_proxy_);

    auto instr = new QStandardItem({ tr("instruments") });
    instr->setEditable(false);
    parentItem->appendRow(instr);

    for (auto& ins : dev_lib.instruments()) {
        auto item = new QStandardItem(ins->title());
        item->setEditable(false);

        QJsonDocument doc(ins->toJson());
        item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
        item->setDropEnabled(false);
        instr->appendRow(item);
    }

    auto sends = new QStandardItem({ tr("sends") });
    sends->setEditable(false);
    parentItem->appendRow(sends);

    for (auto& send : dev_lib.sends()) {
        auto item = new QStandardItem(send->title());
        item->setEditable(false);

        QJsonDocument doc(send->toJson());
        item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
        item->setDropEnabled(false);
        sends->appendRow(item);
    }

    auto returns = new QStandardItem({ tr("returns") });
    returns->setEditable(false);
    parentItem->appendRow(returns);

    for (auto& rtn : dev_lib.returns()) {
        auto item = new QStandardItem(rtn->title());
        item->setEditable(false);

        QJsonDocument doc(rtn->toJson());
        item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
        item->setDropEnabled(false);
        returns->appendRow(item);
    }

    // auto misc = new QStandardItem({ tr("misc") });
    // misc->setEditable(false);
    // parentItem->appendRow(misc);
}

void MainWindow::createToolbarScaleView()
{
    auto w = new QComboBox(this);
    w->addItem("25%", 0.25);
    w->addItem("50%", 0.5);
    w->addItem("75%", 0.75);
    w->addItem("100%", 1.0);
    w->addItem("150%", 1.5);
    w->addItem("200%", 2.0);
    w->addItem("400%", 4.0);

    w->setCurrentText("100%");
    ui->toolBar->addWidget(w);
}

void MainWindow::resizePanels()
{
    float windowWidth = width();
    int dockWidthA = 0.25 * windowWidth;
    int dockWidthB = 0.25 * windowWidth;

    QList<int> dockSizes = { dockWidthA, dockWidthB };
    resizeDocks({ ui->libraryDock, ui->tableDock }, dockSizes, Qt::Horizontal);
}

void MainWindow::setupExpandButton(QToolButton* btn, QTableView* tab, QFrame* line)
{
    line->setHidden(true);
    tab->setHidden(false);
    btn->setArrowType(Qt::DownArrow);
    btn->setIconSize(QSize(6, 6));

    btn->setStyleSheet("QToolButton { "
                       "border-width: 0;"
                       "margin: 0; "
                       "padding: 0 0 0 2px; "
                       "background-color: transparent;"
                       " }");
    connect(btn, &QToolButton::clicked, this, [btn, tab, line]() {
        bool vis = tab->isVisible();
        btn->setArrowType(vis ? Qt::RightArrow : Qt::DownArrow);
        tab->setHidden(vis);
        line->setHidden(!vis);
    });
}

void MainWindow::setupEquipmentTableView(QTableView* tab, QStandardItemModel* model)
{
    QSortFilterProxyModel* m = new QSortFilterProxyModel(this);
    m->setDynamicSortFilter(true);
    m->setSourceModel(model);
    tab->setModel(m);
    tab->setSortingEnabled(true);

    // tab->horizontalHeader()->setVisible(false);
    tab->setStyleSheet("QTableView::item {padding: 0px}");
    tab->setSelectionBehavior(QAbstractItemView::SelectItems);
    tab->setSelectionMode(QAbstractItemView::SingleSelection);
    tab->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void MainWindow::setupDockTitle(QDockWidget* dock)
{
    // dock->setTitleBarWidget(new DockWidget(dock->windowTitle(), "book"));
    dock->setStyleSheet("QDockWidget::title { text-align: left; margin: 0 0 0 5px; }");

    // auto btn = new QToolButton(this);
    // btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    // btn->setText(dock->windowTitle());
    // btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // btn->setIcon(QIcon(":/ceam/cables/resources/zoom_normal_02.svg"));
    // btn->setStyleSheet("QToolButton { border-width: 0; background-color: transparent; margin: 2px 0 0 4px; }");
    // dock->setTitleBarWidget(btn);
}

void MainWindow::writePositionSettings()
{
    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_MAINWINDOW);

    qs.setValue(SKEY_GEOMETRY, saveGeometry());
    qs.setValue(SKEY_SAVESTATE, saveState());
    qs.setValue(SKEY_MAXIMIZED, isMaximized());
    if (!isMaximized()) {
        qs.setValue(SKEY_POS, pos());
        qs.setValue(SKEY_MAXIMIZED, size());
    }

    qs.endGroup();
}

void MainWindow::readPositionSettings()
{
    QSettings qs(SETTINGS_ORG, SETTINGS_APP);

    qs.beginGroup(SKEY_MAINWINDOW);

    restoreGeometry(qs.value(SKEY_GEOMETRY, saveGeometry()).toByteArray());
    restoreState(qs.value(SKEY_SAVESTATE, saveState()).toByteArray());
    move(qs.value(SKEY_POS, pos()).toPoint());
    resize(qs.value(SKEY_SIZE, size()).toSize());
    if (qs.value(SKEY_MAXIMIZED, isMaximized()).toBool())
        showMaximized();

    qs.endGroup();
}

void MainWindow::printScheme()
{
    diagram_->printScheme();
}

void MainWindow::selectAll()
{
    diagram_->cmdSelectAll();
}

bool MainWindow::saveDocument()
{
    if (file_name_.isEmpty())
        return saveDocumentAs();
    else
        return doSave();
}

bool MainWindow::saveDocumentAs()
{
    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    file_name_ = QFileDialog::getSaveFileName(this, tr("Save project"), path, tr("PatchScene projects (*.psc *.json)"));
    return doSave();
}

void MainWindow::duplicateSelection()
{
    diagram_->cmdDuplicateSelection();
}

void MainWindow::exportDocument()
{
    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    auto odt_file = QFileDialog::getSaveFileName(this, tr("Save project"), path, tr("OpenDocument format (*.odt)"));
    if (odt_file.isEmpty())
        return;

    QTextDocument doc;
    QTextCursor cursor(&doc);

    ceam::doc::insert_section(cursor, tr("Devices"));
    ceam::doc::insert_table(cursor, device_model_, { 40, 20, 20 });

    ceam::doc::insert_section(cursor, tr("Connections"));
    ceam::doc::insert_table(cursor, conn_model_);

    ceam::doc::insert_section(cursor, tr("Sends"));
    ceam::doc::insert_table(cursor, send_model_);

    ceam::doc::insert_section(cursor, tr("Returns"));
    ceam::doc::insert_table(cursor, return_model_);

    QTextDocumentWriter writer(odt_file, "ODF");

    if (writer.write(&doc)) {
        qDebug() << "exported to" << odt_file;
    } else {
        qWarning() << "can't save to" << odt_file;
    }
}

void MainWindow::openDocument()
{
    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    auto file_name = QFileDialog::getOpenFileName(this, tr("Open project"), path, tr("PatchScene projects (*.psc *.json)"));
    if (diagram_->loadJson(file_name)) {
        setProjectName(file_name);
        updateTitle();
        setWindowModified(false);
        statusBar()->showMessage(tr("Load '%1'").arg(file_name_), 2000);
    }
}
