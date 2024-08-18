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
#include "device_item_model.h"
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

constexpr const char* SCHEME_DATA_URL = "mydata://scheme.png";
constexpr qreal SCHEME_IMAGE_WIDTH = 625;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setStatusBar(new QStatusBar);
    // createToolbarScaleView();

    setupDockTitle(ui->libraryDock);
    setupDockTitle(ui->tableDock);
    setupDockTitle(ui->favoritesDock);
    setUnifiedTitleAndToolBarOnMac(true);

    favorites_ = new FavoritesWidget(ui->favoritesDock);
    ui->favoritesHBox->layout()->addWidget(favorites_);

    initDeviceList();
    initConnectionList();
    initSendList();
    initReturnList();

    setupExpandButton(ui->deviceListBtn, ui->deviceList, ui->deviceListLine);
    setupExpandButton(ui->connectionListBtn, ui->connectionList, ui->connectionListLine);
    setupExpandButton(ui->sendListBtn, ui->sendList, ui->sendListLine);
    setupExpandButton(ui->returnListBtn, ui->returnList, ui->returnListLine);

    initDiagram();
    initActions();
    initLibrarySearch();

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

void MainWindow::initDiagram()
{
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
        device_model_->clearItems();
        conn_model_->clearItems();
        send_model_->clearItems();
        return_model_->clearItems();
    });
    connect(diagram_, SIGNAL(addToFavorites(SharedDeviceData)), this, SLOT(onAddToFavorites(SharedDeviceData)));
    connect(diagram_, &Diagram::zoomChanged, this, [this](qreal z) {
        statusBar()->showMessage(tr("Zoom %1%").arg(qRound(z * 100)), 1000);
    });
}

void MainWindow::initDeviceList()
{
    device_model_ = new DeviceItemModel(this);
    connect(device_model_, &QStandardItemModel::itemChanged, this, [this](QStandardItem* item) {
        auto id = device_model_->deviceId(item);
        if (id) {
            auto dev = diagram_->findDeviceById(id.value());
            if (dev) {
                auto data = device_model_->updateDeviceData(item, dev->deviceData());
                diagram_->cmdUpdateDevice(data);
            } else {
                qWarning() << "device not found:" << (int)id.value();
            }
        }
    });

    QSignalBlocker sb(ui->deviceList);
    ui->deviceList->setSortingEnabled(true);
    ui->deviceList->sortByColumn(0, Qt::AscendingOrder);

    ui->deviceList->horizontalHeader()->setVisible(true);
    ui->deviceList->horizontalHeader()->setStretchLastSection(true);
    ui->deviceList->verticalHeader()->setVisible(false);

    ui->deviceList->setStyleSheet("QTableView::item {padding: 0px}");
    ui->deviceList->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->deviceList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->deviceList->setModel(device_model_->sortProxy());

    connect(ui->deviceList, &QTableView::clicked, this, [this](const QModelIndex& index) {
        auto id = device_model_->deviceId(index.row());
        if (id)
            diagram_->cmdSelectUnique(id.value());
    });
    ui->deviceList->resizeColumnsToContents();
}

void MainWindow::initActions()
{
#ifdef Q_OS_DARWIN
    ui->actionZoomIn->setIconVisibleInMenu(false);
    ui->actionZoomOut->setIconVisibleInMenu(false);
#else
    ui->actionZoomIn->setIconVisibleInMenu(true);
    ui->actionZoomOut->setIconVisibleInMenu(true);
#endif

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
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveDocumentAs()));
    connect(ui->actionSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
    connect(ui->actionSetBackground, SIGNAL(triggered(bool)), this, SLOT(setBackground()));
    connect(ui->actionProjectInfo, SIGNAL(triggered(bool)), this, SLOT(documentProperties()));

    connect(ui->actionShowCables, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowCables(value);
    });
    connect(ui->actionShowBackground, &QAction::triggered, diagram_, [this](bool value) {
        diagram_->setShowBackground(value);
    });
    connect(ui->actionExport, SIGNAL(triggered()), this, SLOT(exportToOdf()));

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

    connect(ui->actionRedo, SIGNAL(triggered()), diagram_, SLOT(redo()));
    connect(ui->actionUndo, SIGNAL(triggered()), diagram_, SLOT(undo()));
}

void MainWindow::initLibrarySearch()
{
    connect(ui->librarySearch, &QLineEdit::textChanged, this, [this](const QString& txt) {
        library_proxy_->setFilterRegularExpression(txt);

        if (!txt.isEmpty())
            ui->libraryTree->expandAll();
    });
    ui->librarySearch->setStatusTip(tr("search in library"));
    ui->librarySearch->setClearButtonEnabled(true);
    ui->librarySearch->addAction(QIcon(":/icons/search_02.svg"), QLineEdit::LeadingPosition);
    ui->librarySearch->setStyleSheet("QLineEdit {border-width: 1px;}");

#ifdef Q_OS_DARWIN
    {
        auto font = ui->librarySearch->font();
        font.setPointSize(font.pointSize() - 2);
        ui->librarySearch->setFont(font);
        ui->librarySearch->setMaximumHeight(16);
    }
#endif
}

void MainWindow::initConnectionList()
{
    conn_model_ = new ConnectionItemModel(this);

    QSignalBlocker sb(ui->connectionList);
    ui->connectionList->setModel(conn_model_->sortProxy());
    ui->connectionList->setSortingEnabled(true);
    ui->connectionList->sortByColumn(0, Qt::AscendingOrder);

    ui->connectionList->horizontalHeader()->setVisible(true);
    ui->connectionList->horizontalHeader()->setStretchLastSection(true);
    ui->connectionList->verticalHeader()->setVisible(false);

    ui->connectionList->setStyleSheet("QTableView::item {padding: 0px}");
    ui->connectionList->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->connectionList->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->connectionList, &QTableView::clicked, this, [this](const QModelIndex& index) {
        auto id = conn_model_->deviceId(index);
        if (id)
            diagram_->cmdSelectUnique(id.value());
    });
    ui->connectionList->resizeColumnsToContents();
}

void MainWindow::initSendList()
{
    send_model_ = new SendItemModel(this);

    QSignalBlocker sb(ui->sendList);
    ui->sendList->setModel(send_model_->sortProxy());
    ui->sendList->setSortingEnabled(true);
    ui->sendList->sortByColumn(0, Qt::AscendingOrder);

    ui->sendList->horizontalHeader()->setVisible(true);
    ui->sendList->horizontalHeader()->setStretchLastSection(true);
    ui->sendList->verticalHeader()->setVisible(true);

    ui->sendList->setStyleSheet("QTableView::item {padding: 0px}");
    ui->sendList->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->sendList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->sendList->resizeColumnsToContents();
}

void MainWindow::initReturnList()
{
    return_model_ = new ReturnItemModel(this);

    QSignalBlocker sb(ui->returnList);
    ui->returnList->setModel(return_model_->sortProxy());
    ui->returnList->setSortingEnabled(true);
    ui->returnList->sortByColumn(0, Qt::AscendingOrder);

    ui->returnList->horizontalHeader()->setVisible(true);
    ui->returnList->horizontalHeader()->setStretchLastSection(true);
    ui->returnList->verticalHeader()->setVisible(true);

    ui->returnList->setStyleSheet("QTableView::item {padding: 0px}");
    ui->returnList->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui->returnList->setSelectionMode(QAbstractItemView::SingleSelection);

    ui->returnList->resizeColumnsToContents();
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
    if (device_model_->addDevice(data))
        ui->deviceList->resizeColumnsToContents();
}

void MainWindow::onDeviceRemove(SharedDeviceData data)
{
    if (device_model_->removeDevice(data))
        ui->deviceList->resizeColumnsToContents();
}

void MainWindow::onDeviceTitleUpdate(DeviceId id, const QString& title)
{
    if (conn_model_->updateDeviceTitle(id, title))
        ui->connectionList->resizeColumnsToContents();

    if (send_model_->updateDeviceTitle(id, title))
        ui->sendList->resizeColumnsToContents();

    if (return_model_->updateDeviceTitle(id, title))
        ui->returnList->resizeColumnsToContents();
}

void MainWindow::updateDeviceView(const SharedDeviceData& data, int idx)
{
    if (data->category() != ItemCategory::Device) {
        device_model_->removeRow(idx);
    } else {
        QSignalBlocker sb(device_model_);

        auto title = device_model_->deviceTitle(idx);
        if (title) {
            title->setText(data->title());
            ui->deviceList->update(title->index());
        }

        auto vendor = device_model_->deviceVendor(idx);
        if (vendor) {
            vendor->setText(data->vendor());
            ui->deviceList->update(vendor->index());
        }

        auto model = device_model_->deviceModel(idx);
        if (model) {
            model->setText(data->model());
            ui->deviceList->update(model->index());
        }
    }

    ui->deviceList->resizeColumnsToContents();
}

void MainWindow::onDeviceUpdate(SharedDeviceData data)
{
    if (!data) {
        qWarning() << "invalid data";
        return;
    }

    bool device_found = false;
    for (int i = 0; i < device_model_->deviceCount(); i++) {
        auto id = device_model_->deviceId(i);
        if (id && id.value() == data->id()) {
            return updateDeviceView(data, i);
        }
    }

    // not found in model
    if (!device_found && data->category() == ItemCategory::Device) {
        onDeviceAdd(data);
    }
}

void MainWindow::onConnectionAdd(ConnectionData data)
{
    XletData src, dest;
    Device *src_dev = nullptr, *dest_dev = nullptr;
    if (diagram_->findConnectionXletData(data, src, dest, &src_dev, &dest_dev)) {
        if (!src_dev || !dest_dev)
            return;

        if (conn_model_->addConnection(data, src_dev->deviceData(), src, dest_dev->deviceData(), dest))
            ui->connectionList->resizeColumnsToContents();

        if (send_model_->addConnection(data, src_dev->deviceData(), dest_dev->deviceData()))
            ui->sendList->resizeColumnsToContents();

        if (return_model_->addConnection(data, src_dev->deviceData(), dest_dev->deviceData()))
            ui->returnList->resizeColumnsToContents();
    }
}

void MainWindow::onConnectionRemove(ConnectionData data)
{
    if (conn_model_->removeConnection(data))
        ui->connectionList->resizeColumnsToContents();

    if (send_model_->removeConnection(data))
        ui->sendList->resizeColumnsToContents();

    if (return_model_->removeConnection(data))
        ui->returnList->resizeColumnsToContents();
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

QTextDocument* MainWindow::exportToDocument()
{
    auto doc = new QTextDocument(this);
    doc->setMetaInformation(QTextDocument::DocumentTitle, diagram_->meta().title());

    QTextCursor cursor(doc);

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

    ceam::doc::insert_section(cursor, tr("Scheme"));
    ceam::doc::insert_paragraph(cursor, "");
    auto img = diagram_->toImage();
    doc->addResource(QTextDocument::ImageResource, QUrl(SCHEME_DATA_URL), QVariant(img));

    QTextImageFormat imageFormat;
    imageFormat.setName(SCHEME_DATA_URL);
    imageFormat.setWidth(SCHEME_IMAGE_WIDTH);
    imageFormat.setHeight(SCHEME_IMAGE_WIDTH * img.height() / img.width());
    cursor.insertImage(imageFormat);

    ceam::doc::insert_paragraph(cursor, "");
    ceam::doc::insert_section(cursor, tr("Devices"));
    ceam::doc::insert_table(cursor, device_model_);

    ceam::doc::insert_section(cursor, tr("Connections"));
    ceam::doc::insert_table(cursor, conn_model_);

    ceam::doc::insert_section(cursor, tr("Sends"));
    ceam::doc::insert_table(cursor, send_model_);

    ceam::doc::insert_section(cursor, tr("Returns"));
    ceam::doc::insert_table(cursor, return_model_);

    ceam::doc::insert_paragraph(cursor, tr("Created with PatchScene v%1").arg(PATCH_SCENE_VERSION), Qt::AlignRight);

    return doc;
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
    file_name_ = QFileDialog::getSaveFileName(this, tr("Save project"), path, tr("PatchScene projects (*.psc)"));
    if (file_name_.isEmpty())
        return false;

    if (QFileInfo(file_name_).suffix().isEmpty())
        file_name_.append(".psc");

    auto res = doSave();
    if (res)
        addRecentFile(file_name_);

    return res;
}

void MainWindow::duplicateSelection()
{
    diagram_->cmdDuplicateSelection();
}

void MainWindow::exportToOdf()
{
    QFileInfo finfo(file_name_);

    auto path = QStandardPaths::locate(QStandardPaths::DocumentsLocation, "", QStandardPaths::LocateDirectory);
    if (file_name_.isEmpty()) {
        path += "/NewProject.odt";
    } else {
        path += "/" + finfo.baseName() + ".odt";
    }

    auto odt_file = QFileDialog::getSaveFileName(this, tr("Save to OpenDocument format"), path, tr("OpenDocument format (*.odt)"));
    if (odt_file.isEmpty())
        return;

    if (QFileInfo(odt_file).suffix().isEmpty())
        odt_file.append(".odt");

    auto doc = exportToDocument();
    if (!doc)
        return;

    QTextDocumentWriter writer(odt_file, "ODF");

    if (writer.write(doc)) {
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
