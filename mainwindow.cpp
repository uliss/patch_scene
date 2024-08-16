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
#include "diagram_item_model.h"
#include "diagram_meta_dialog.h"
#include "export_document.h"
#include "patch_scene_version.h"
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

enum ConnColumnOrder {
    COL_CONN_SRC_NAME = 0,
    COL_CONN_SRC_MODEL,
    COL_CONN_SRC_PLUG,
    COL_CONN_DEST_NAME,
    COL_CONN_DEST_MODEL,
    COL_CONN_DEST_PLUG
};
constexpr int DATA_CONN_NCOLS = 6;

enum SendColumnOrder {
    COL_SEND_NAME = 0,
    COL_SEND_INPUT,
    COL_SEND_SRC_NAME,
    COL_SEND_SRC_OUTPUT
};
constexpr int DATA_SEND_NCOLS = 4;

enum ReturnColumnOrder {
    COL_RETURN_NAME = 0,
    COL_RETURN_OUTPUT,
    COL_RETURN_DEST_NAME,
    COL_RETURN_DEST_INPUT,
};
constexpr int DATA_RETURN_NCOLS = 4;

enum DeviceColumnOrder {
    COL_DEV_TITLE = 0,
    COL_DEV_VENDOR,
    COL_DEV_MODEL,
};
constexpr int DATA_DEV_NCOLS = 3;

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
    setupDockTitle(ui->favoritesDock);

    favorites_ = new FavoritesWidget(ui->favoritesDock);
    ui->favoritesHBox->layout()->addWidget(favorites_);

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

    diagram_ = new Diagram(1600, 1600);
    ui->gridLayout->addWidget(diagram_, 1, 1);
    connect(diagram_, SIGNAL(sceneChanged()), this, SLOT(onSceneChange()));
    connect(diagram_, SIGNAL(deviceAdded(SharedDeviceData)), this, SLOT(onDeviceAdd(SharedDeviceData)));
    connect(diagram_, SIGNAL(deviceRemoved(SharedDeviceData)), this, SLOT(onDeviceRemove(SharedDeviceData)));
    connect(diagram_, SIGNAL(deviceUpdated(SharedDeviceData)), this, SLOT(onDeviceUpdate(SharedDeviceData)));
    connect(diagram_, SIGNAL(deviceTitleUpdated(DeviceId, QString)), this, SLOT(onDeviceTitleUpdate(DeviceId, QString)));

    connect(diagram_, SIGNAL(connectionAdded(ConnectionData)), this, SLOT(onConnectionAdd(ConnectionData)));
    connect(diagram_, SIGNAL(connectionRemoved(ConnectionData)), this, SLOT(onConnectionRemove(ConnectionData)));

    connect(diagram_, &Diagram::canRedoChanged, this, [this](bool value) { ui->actionRedo->setEnabled(value); });
    connect(diagram_, &Diagram::canUndoChanged, this, [this](bool value) { ui->actionUndo->setEnabled(value); });
    connect(diagram_, &Diagram::sceneClearAll, this, [this]() {
        device_model_->clear();
        conn_model_->clear();
        send_model_->clear();
        return_model_->clear();
    });
    connect(diagram_, SIGNAL(addToFavorites(SharedDeviceData)), this, SLOT(onAddToFavorites(SharedDeviceData)));

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
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSetBackground, SIGNAL(triggered(bool)), this, SLOT(setBackground()));
    connect(ui->actionProjectInfo, SIGNAL(triggered(bool)), this, SLOT(documentProperties()));

    connect(ui->actionShowCables, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowCables(value);
    });
    connect(ui->actionShowBackground, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowBackground(value);
    });
    connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(exportDocument()));

    connect(ui->actionAddDevice, &QAction::triggered, this, [this]() {
        auto pos = diagram_->mapFromGlobal(QCursor::pos());
        if (pos.x() < 0 || pos.y() < 0)
            return;

        diagram_->cmdCreateDevice(diagram_->mapToScene(pos));
    });

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
    ui->librarySearch->addAction(QIcon(":/icons/search_02.svg"), QLineEdit::LeadingPosition);
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

    loadLibrary();
    loadFavorites();

    readPositionSettings();
    readRecentFiles();
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

void MainWindow::documentProperties()
{
    auto dialog = new DiagramMetaDialog(diagram_->meta(), this);
    connect(dialog, &DiagramMetaDialog::accepted, this, [dialog, this]() {
        diagram_->setMeta(dialog->metaInfo());
    });
    dialog->show();
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
    // /auto prefs = new PreferencesDialog(this);
    // prefs->show();
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
    writeRecentFiles();
    writeFavorites();

    QMainWindow::closeEvent(event);
}

void MainWindow::onAddToFavorites(SharedDeviceData data)
{
    favorites_->addItem(data);
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

static bool updateItemDeviceName(QStandardItem* item, DeviceId id, const QString& title)
{
    if (!item)
        return false;

    bool ok = false;
    auto dev_id = item->data(DATA_DEVICE_ID).toInt(&ok);
    if (dev_id && dev_id == id) {
        if (item->text() != title)
            item->setText(title);

        return true;
    } else
        return false;
}

void MainWindow::onDeviceTitleUpdate(DeviceId id, const QString& title)
{
    // update connection device name change
    for (int i = 0; i < conn_model_->rowCount(); i++) {
        if (updateItemDeviceName(conn_model_->item(i, COL_CONN_SRC_NAME), id, title))
            continue;

        if (updateItemDeviceName(conn_model_->item(i, COL_CONN_DEST_NAME), id, title))
            continue;
    }

    // update connection device name change
    for (int i = 0; i < send_model_->rowCount(); i++)
        updateItemDeviceName(send_model_->item(i, COL_SEND_SRC_NAME), id, title);

    // update connection device name change
    for (int i = 0; i < return_model_->rowCount(); i++)
        updateItemDeviceName(return_model_->item(i, COL_RETURN_DEST_NAME), id, title);
}

void MainWindow::onDeviceUpdate(SharedDeviceData data)
{
    if (!data) {
        qWarning() << "invalid data";
        return;
    }

    bool device_found = false;
    for (int i = 0; i < device_model_->rowCount(); i++) {
        auto title = device_model_->item(i, COL_DEV_TITLE);
        if (title && title->data(DATA_DEVICE_ID) == data->id()) {
            QSignalBlocker block(device_model_);

            if (data->category() != ItemCategory::Device) {
                device_model_->removeRow(i);
            } else {
                bool title_update = (title->text() != data->title());

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
        dest_name->setData(QVariant::fromValue(data), DATA_CONNECTION);
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
        auto item = send_model_->item(i, COL_SEND_SRC_NAME);
        if (item && item->data(DATA_CONNECTION) == QVariant::fromValue(data)) {
            send_model_->removeRow(i);
        }
    }

    for (int i = 0; i < return_model_->rowCount(); i++) {
        auto item = return_model_->item(i, COL_RETURN_DEST_NAME);
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
        qWarning() << "can't open for writing" << file_name_;
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

void MainWindow::loadSection(QStandardItem* parent, const QList<SharedDeviceData>& data)
{
    for (auto& x : data) {
        auto item = new QStandardItem(x->title());
        item->setEditable(false);

        QJsonDocument doc(x->toJson());
        item->setData(doc.toJson(QJsonDocument::Compact), DATA_DEVICE_DATA);
        item->setDragEnabled(true);
        item->setDropEnabled(false);
        parent->appendRow(item);
    }
}

void MainWindow::loadLibrary()
{
    auto model = new DiagramItemModel(this);
    auto parentItem = model->invisibleRootItem();

    ui->libraryTree->setDragEnabled(true);
    ui->libraryTree->setDragDropMode(QAbstractItemView::DragOnly);
    ui->libraryTree->setSortingEnabled(true);
    ui->libraryTree->sortByColumn(0, Qt::AscendingOrder);

    library_proxy_ = new QSortFilterProxyModel(this);
    library_proxy_->setSourceModel(model);
    library_proxy_->setFilterCaseSensitivity(Qt::CaseInsensitive);
    library_proxy_->setAutoAcceptChildRows(true);
    library_proxy_->setRecursiveFilteringEnabled(true);
    library_proxy_->setSortLocaleAware(true);
    ui->libraryTree->setModel(library_proxy_);

    DeviceLibrary dev_lib;
    if (!dev_lib.readFile(":/library.json"))
        return;

    auto devices = new QStandardItem(tr("devices"));
    devices->setEditable(false);
    parentItem->appendRow(devices);
    loadSection(devices, dev_lib.devices());

    auto instr = new QStandardItem({ tr("instruments") });
    instr->setEditable(false);
    parentItem->appendRow(instr);
    loadSection(instr, dev_lib.instruments());

    auto sends = new QStandardItem({ tr("sends") });
    sends->setEditable(false);
    parentItem->appendRow(sends);
    loadSection(sends, dev_lib.sends());

    auto returns = new QStandardItem({ tr("returns") });
    returns->setEditable(false);
    parentItem->appendRow(returns);
    loadSection(returns, dev_lib.returns());

    auto furniture = new QStandardItem({ tr("furniture") });
    furniture->setEditable(false);
    parentItem->appendRow(furniture);
    loadSection(furniture, dev_lib.furniture());

    auto humans = new QStandardItem({ tr("humans") });
    humans->setEditable(false);
    parentItem->appendRow(humans);
    loadSection(humans, dev_lib.humans());
}

void MainWindow::loadFavorites()
{
    favorites_->setFromVariant(settings_.readFavorites());
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
    tab->sortByColumn(0, Qt::AscendingOrder);

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
    // btn->setIcon(QIcon(":/icons/zoom_normal_02.svg"));
    // btn->setStyleSheet("QToolButton { border-width: 0; background-color: transparent; margin: 2px 0 0 4px; }");
    // dock->setTitleBarWidget(btn);
}

void MainWindow::writePositionSettings() const
{
    settings_.writeWindowPos(this);
}

void MainWindow::writeFavorites() const
{
    settings_.writeFavorites(favorites_->toVariant());
}

void MainWindow::syncRecentFilesMenu()
{
    ui->menuRecentFiles->clear();

    for (auto& url : recent_files_) {
        auto open_file = new QAction(url.fileName(), this);
        open_file->setData(url);
        open_file->setToolTip(url.path());
        open_file->setStatusTip(url.path());
        connect(open_file, &QAction::triggered, this, [this, url]() {
            openDocument(url.path());
        });
        ui->menuRecentFiles->addAction(open_file);
    }

    if (!recent_files_.empty()) {
        ui->menuRecentFiles->addSeparator();
        auto act_clear = new QAction(tr("Clear"));
        connect(act_clear, &QAction::triggered, this, [this]() {
            recent_files_.clear();
            syncRecentFilesMenu();
        });
        ui->menuRecentFiles->addAction(act_clear);
    }
}

void MainWindow::readRecentFiles()
{
    recent_files_ = settings_.readRecentFiles();
    syncRecentFilesMenu();
}

void MainWindow::writeRecentFiles() const
{
    settings_.writeRecentFiles(recent_files_);
}

void MainWindow::addRecentFile(const QUrl& file)
{
    constexpr int MAX_RECENT_FILES = 10;

    auto idx = recent_files_.indexOf(file);
    if (idx >= 0)
        recent_files_.removeAll(file);

    recent_files_.push_front(file);
    if (recent_files_.size() > MAX_RECENT_FILES)
        recent_files_.resize(MAX_RECENT_FILES);

    syncRecentFilesMenu();
}

void MainWindow::readPositionSettings()
{
    settings_.readWindowPos(this);
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
    auto odt_file = QFileDialog::getSaveFileName(this, tr("Save to OpenDocument format"), path, tr("OpenDocument format (*.odt)"));
    if (odt_file.isEmpty())
        return;

    QTextDocument doc;
    QTextCursor cursor(&doc);

    auto& meta = diagram_->meta();

    ceam::doc::insert_header(cursor, meta.title());

    ceam::doc::insert_paragraph(cursor, tr("Event date: %1").arg(meta.eventDate().toString()));

    if (!meta.info().isEmpty()) {
        ceam::doc::insert_section(cursor, tr("Information"));
        ceam::doc::insert_paragraph(cursor, meta.info());
    }

    ceam::doc::insert_section(cursor, tr("Contacts"));
    QList<QStringList> contacts_data;
    contacts_data.push_back({ tr("Name"), tr("Work"), tr("Phone"), tr("Email") });
    for (auto& c : meta.contacts())
        contacts_data.push_back({ c.name(), c.work(), c.phone(), c.email() });

    ceam::doc::insert_table(cursor, contacts_data);

    ceam::doc::insert_section(cursor, tr("Devices"));
    ceam::doc::insert_table(cursor, device_model_, { 40, 20, 20 });

    ceam::doc::insert_section(cursor, tr("Connections"));
    ceam::doc::insert_table(cursor, conn_model_);

    ceam::doc::insert_section(cursor, tr("Sends"));
    ceam::doc::insert_table(cursor, send_model_);

    ceam::doc::insert_section(cursor, tr("Returns"));
    ceam::doc::insert_table(cursor, return_model_);

    QTextDocumentWriter writer(odt_file, "ODF");

    ceam::doc::insert_paragraph(cursor, tr("Created with PatchScene v%1").arg(PATCH_SCENE_VERSION), Qt::AlignRight);

    if (writer.write(&doc)) {
        qDebug() << "exported to" << odt_file;
    } else {
        qWarning() << "can't save to" << odt_file;
    }
}

void MainWindow::openDocument()
{
    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    auto file_name = QFileDialog::getOpenFileName(this, tr("Open project"), path, tr("PatchScene projects (*.psc)"));
    openDocument(file_name);
}

void MainWindow::openDocument(const QString& path)
{
    if (diagram_->loadJson(path)) {
        setProjectName(path);
        updateTitle();
        setWindowModified(false);
        statusBar()->showMessage(tr("Load '%1'").arg(file_name_), 2000);
        addRecentFile(path);
    }
}
