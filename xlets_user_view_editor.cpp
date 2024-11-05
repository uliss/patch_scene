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
#include "xlets_user_view_editor.h"
#include "device_xlet_common.h"
#include "device_xlet_view.h"
#include "logging.hpp"
#include "ui_xlets_user_view_editor.h"
#include "xlets_user_view.h"
#include "xlets_view.h"

#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QStringListModel>

namespace {
constexpr int XW = 22;
constexpr int XH = 20;
}

namespace ceam {

XletsUserViewEditor::XletsUserViewEditor(QWidget* parent, const SharedDeviceData& data)
    : QDialog(parent)
    , ui(new Ui::XletsUserEditor)
    , data_(data)
    , view_scene_(data_->inputs(), data_->outputs())
{
    ui->setupUi(this);

    ui->inletsView->setScene(&in_scene_);
    ui->outletsView->setScene(&out_scene_);

    initInlets();
    initOutlets();
    initRowsAndCols();

    initButtons(data);
    initUserViewList(data);

    if (data->userViewData().isEmpty()) {
        enableUserView(false);
        adjustUserViewSize();
    } else
        initUserViewDataWith(data->currentUserView());

    ui->userView->setScene(&view_scene_);
    ui->userView->setFixedSize(view_scene_.itemsBoundingRect().size().toSize().grownBy({ 3, 3, 3, 3 }));
    ui->userView->centerOn(view_scene_.sceneRect().center());

    connect(&view_scene_, &XletsUserScene::updated, this,
        [this]() {
            setXletViewData(view_scene_.currentIndex(), view_scene_.data());
        });
}

XletsUserViewEditor::~XletsUserViewEditor()
{
    inlets_.clearXlets();
    outlets_.clearXlets();
    delete ui;
}

void XletsUserViewEditor::initRowsAndCols()
{
    ui->numCols->setMinimum(XletsUserViewData::MIN_COL_COUNT);
    ui->numCols->setMaximum(XletsUserViewData::MAX_COL_COUNT);
    ui->numRows->setMinimum(XletsUserViewData::MIN_ROW_COUNT);
    ui->numRows->setMaximum(XletsUserViewData::MAX_ROW_COUNT);

    connect(ui->numCols, &QSpinBox::valueChanged, this,
        [this](int v) {
            view_scene_.setCols(v);
            setXletViewData(view_scene_.currentIndex(), view_scene_.data());
            adjustUserViewSize();
        });

    connect(ui->numRows, &QSpinBox::valueChanged, this,
        [this](int v) {
            view_scene_.setRows(v);
            setXletViewData(view_scene_.currentIndex(), view_scene_.data());
            adjustUserViewSize();
        });
}

void XletsUserViewEditor::setXletViewData(int idx, const XletsUserViewData& data)
{
    if (idx < 0 || idx >= data_->userViewData().count()) {
        WARN() << "invalid user xlet view index:" << idx;
        return;
    } else {
        data_->userViewData()[idx] = data;
    }
}

void XletsUserViewEditor::accept()
{
    emit acceptData(data_);
    QDialog::accept();
}

void XletsUserViewEditor::initInlets()
{
    XletsLogicView view({}, inlets_);
    for (auto& x : data_->inputs()) {
        auto in = inlets_.append(x, XletType::In, nullptr);
        in->setDragMode(true);
        in_scene_.addItem(in);
    }

    view.placeXlets({});

    ui->inletsView->setFixedSize(in_scene_.itemsBoundingRect().size().toSize().grownBy({ 3, 3, 3, 3 }));
    ui->inletsView->centerOn(in_scene_.sceneRect().center());
}

void XletsUserViewEditor::initOutlets()
{
    XletsLogicView view({}, outlets_);
    for (auto& x : data_->outputs()) {
        auto out = outlets_.append(x, XletType::Out, nullptr);
        out->setDragMode(true);
        out_scene_.addItem(out);
    }
    view.placeXlets({});
    ui->outletsView->setFixedSize(out_scene_.itemsBoundingRect().size().toSize().grownBy({ 3, 3, 3, 3 }));
    ui->outletsView->centerOn(out_scene_.sceneRect().center());
}

void XletsUserViewEditor::initButtons(const SharedDeviceData& data)
{
    connect(ui->addView, &QToolButton::clicked, this,
        [this]() {
            auto tr_name = tr("User");
            auto new_item_count = ui->userViewList->findItems(tr_name, Qt::MatchStartsWith).size();
            auto row = ui->userViewList->currentRow() + 1;

            auto item = new QListWidgetItem(
                (new_item_count > 0)
                    ? tr("User %1").arg(new_item_count)
                    : tr_name);

            item->setFlags(item->flags() | Qt::ItemIsEditable);
            ui->userViewList->insertItem(row, item);

            XletsUserViewData data;
            data.setName(item->text());
            data_->userViewData().insert(row, data);

            ui->userViewList->setCurrentItem(item);
        });

    connect(ui->removeView, &QToolButton::clicked, this,
        [this]() {
            auto row = ui->userViewList->currentRow();
            auto item = ui->userViewList->takeItem(row);
            if (item) {
                delete item;

                data_->userViewData().remove(row);
                enableUserView(!data_->userViewData().isEmpty());
            }
        });
}

void XletsUserViewEditor::initUserViewList(const SharedDeviceData& data)
{
    QListWidgetItem* current_item = nullptr;
    for (auto& uv : data->userViewData()) {
        auto item = new QListWidgetItem(uv.name());
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        ui->userViewList->addItem(item);
        if (uv.name() == data->currentUserView()) {
            current_item = item;
        }
    }

    if (current_item) {
        ui->userViewList->setCurrentItem(current_item);
        current_item->setSelected(true);
    } else {
        auto first = ui->userViewList->item(0);
        if (first) {
            ui->userViewList->setCurrentItem(first);
            first->setSelected(true);
        }
    }

    connect(ui->userViewList, &QListWidget::itemDoubleClicked, this,
        [this](QListWidgetItem* item) {
            ui->userViewList->editItem(item);
        });

    connect(ui->userViewList, &QListWidget::itemChanged, this,
        [this](QListWidgetItem* item) {
            auto text = item->text().trimmed();
            if (text != item->text())
                item->setText(item->text().trimmed());

            auto idx = ui->userViewList->indexFromItem(item);
            auto row = idx.row();
            if (row >= 0 && row < data_->userViewData().count()) {
                data_->userViewData()[row].setName(text);
            }
        });

    connect(ui->userViewList, &QListWidget::currentRowChanged, this,
        [this](int idx) {
            initUserViewDataWith(idx);
        });
}

void XletsUserViewEditor::initUserViewDataWith(int idx)
{
    if (idx < 0 || idx >= data_->userViewData().size()) {
        WARN() << "invalid index:" << idx;
        ui->numCols->setValue(XletsUserViewData::DEF_COL_COUNT);
        ui->numRows->setValue(XletsUserViewData::DEF_ROW_COUNT);
        enableUserView(false);
        adjustUserViewSize();
        view_scene_.clearAll();
    } else {
        enableUserView(true);

        auto& data = data_->userViewData()[idx];

        view_scene_.setData(idx, data);
        ui->numCols->setValue(data.columnCount());
        ui->numRows->setValue(data.rowCount());

        adjustUserViewSize();
    }
}

void XletsUserViewEditor::initUserViewDataWith(const QString& viewName)
{
    if (viewName.isEmpty())
        return initUserViewDataWith(0);

    auto items = ui->userViewList->findItems(viewName, Qt::MatchExactly);
    if (items.isEmpty())
        return initUserViewDataWith(0);

    auto idx = ui->userViewList->indexFromItem(items.front());
    if (idx.isValid())
        initUserViewDataWith(idx.row());
    else
        initUserViewDataWith(0);
}

void XletsUserViewEditor::adjustUserViewSize()
{
    auto rect = view_scene_.itemsBoundingRect();
    auto sz = rect.size().toSize().grownBy({ 3, 3, 3, 3 });
    ui->userView->centerOn(rect.center() + QPoint(0, 3));
    ui->userView->setFixedSize(sz);
}

void XletsUserViewEditor::enableUserView(bool value)
{
    ui->inletsView->setEnabled(value);
    ui->outletsView->setEnabled(value);
    ui->userView->setEnabled(value);
    ui->numCols->setEnabled(value);
    ui->numRows->setEnabled(value);
}

XletsUserScene::XletsUserScene(const QList<XletData>& inlets, const QList<XletData>& outlets)
    : inlets_(inlets)
    , outlets_(outlets)
{
}

void XletsUserScene::setSize(int rows, int cols)
{
    clearCells();

    for (int ri = 0; ri < rows; ri++) {
        for (int ci = 0; ci < cols; ci++) {
            auto cell = new XletsUserViewCell({ 0, 0, XW, XH }, { ri, ci }, data_, this);
            cell->setPos(ci * XW, ri * XH);
            addItem(cell);
            cells_.push_back(cell);
        }
    }

    data_.setColumnCount(cols);
    data_.setRowCount(rows);
}

void XletsUserScene::setRows(int v)
{
    setSize(v, data_.columnCount());
}

void XletsUserScene::setCols(int v)
{
    setSize(data_.rowCount(), v);
}

void XletsUserScene::placeXlets()
{
    XletsUserView view({}, xlets_);
    view.setData(data_);
    view.placeXlets({});

    emit updated();
}

void XletsUserScene::reinitXlets()
{
    xlets_.clearXlets();
    for (auto& x : outlets_) {
        auto out = xlets_.append(x, XletType::Out, nullptr);
        out->setDragMode(true, true);
        out->setZValue(2);
        addItem(out);
    }
    for (auto& x : inlets_) {
        auto in = xlets_.append(x, XletType::In, nullptr);
        in->setDragMode(true, true);
        in->setZValue(2);
        addItem(in);
    }
}

void XletsUserScene::clearAll()
{
    clearCells();
    xlets_.clearXlets();
    clear();
}

void XletsUserScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->modifiers().testFlag(Qt::AltModifier)) {
        auto x = items(event->scenePos(), Qt::IntersectsItemShape, Qt::AscendingOrder);
        if (x.count() >= 2
            && qgraphicsitem_cast<XletsUserViewCell*>(x.front())) { // non empty cell
            auto cell = qgraphicsitem_cast<XletsUserViewCell*>(x.front());
            auto cell_idx = cell->cellIndex();
            auto idx = data_.xletAtCell(cell_idx);
            if (!idx.isNull() && data_.clearCell(cell_idx)) {
                if (xlets_.removeXlet(idx))
                    emit updated();
            }
        }
    } else
        QGraphicsScene::mousePressEvent(event);
}

void XletsUserScene::clearCells()
{
    for (auto c : cells_) {
        removeItem(c);
        delete c;
    }

    cells_.clear();
}

void XletsUserScene::setData(int currentIndex, const XletsUserViewData& data)
{
    data_ = data;
    idx_ = currentIndex;

    setSize(data.rowCount(), data.columnCount());

    reinitXlets();
    placeXlets();
}

XletsUserViewCell::XletsUserViewCell(const QRect& r, const CellIndex& idx, XletsUserViewData& data, XletsUserScene* parent)
    : QGraphicsRectItem(r)
    , cell_(idx)
    , data_(data)
    , scene_(parent)
{
    setPen(QPen(Qt::darkGray));
    setBrush(Qt::white);

    setAcceptDrops(true);
}

void XletsUserViewCell::dragEnterEvent(QGraphicsSceneDragDropEvent* event)
{
    auto mime = event->mimeData();
    if (mime && mime->hasFormat(DEVICE_XLET_MIME_INDEX) && mime->hasFormat(DEVICE_XLET_MIME_XLET_TYPE)) {
        event->setAccepted(true);
        hoverCell();
    } else {
        if (mime)
            WARN() << "unsupported mime formats:" << mime->formats();

        QGraphicsRectItem::dragEnterEvent(event);
    }
}

void XletsUserViewCell::dragLeaveEvent(QGraphicsSceneDragDropEvent* event)
{
    resetCell();
    QGraphicsRectItem::dragLeaveEvent(event);
}

void XletsUserViewCell::dropEvent(QGraphicsSceneDragDropEvent* event)
{
    auto mime = event->mimeData();
    if (mime && mime->hasFormat(DEVICE_XLET_MIME_INDEX) && mime->hasFormat(DEVICE_XLET_MIME_XLET_TYPE)) {
        bool ok = false;
        auto index = mime->data(DEVICE_XLET_MIME_INDEX).toInt(&ok);
        if (!ok) {
            WARN() << "can't get xlet" << DEVICE_XLET_MIME_INDEX << "from MIME data";
            return;
        }

        if (index < 0 || index > std::numeric_limits<XletIndex>::max()) {
            WARN() << "invalid xlet index MIME value:" << index;
            return;
        }

        auto type = mime->data(DEVICE_XLET_MIME_XLET_TYPE).toInt(&ok);
        if (!ok) {
            WARN() << "can't get xlet" << DEVICE_XLET_MIME_XLET_TYPE << "from MIME data";
            return;
        }

        XletViewIndex vidx { static_cast<XletIndex>(index), XletType::None };

        switch (static_cast<XletType>(type)) {
        case XletType::In:
            vidx.type = XletType::In;
            break;
        case XletType::Out:
            vidx.type = XletType::Out;
            break;
        default:
            WARN() << "invalid xlet type MIME value:" << type;
            return;
        }

        if (!data_.insertXlet(cell_, vidx)) {
            WARN() << QString("can't insert xlet %1 into given cell [%2 %3]").arg(index).arg(cell_.row).arg(cell_.column);
            return;
        }

        event->setAccepted(true);
        resetCell();

        if (!mime->hasFormat(DEVICE_XLET_MIME_SELF_DRAG))
            scene_->reinitXlets();

        scene_->placeXlets();
    } else {
        if (mime)
            WARN() << "unsupported mime formats:" << mime->formats();

        QGraphicsRectItem::dropEvent(event);
    }
}

void XletsUserViewCell::resetCell()
{
    setPen(QPen(Qt::darkGray));
    setBrush(Qt::white);
    setZValue(-1);
    update();
}

void XletsUserViewCell::hoverCell()
{
    setPen(QPen(Qt::darkMagenta));
    setBrush(Qt::lightGray);
    setZValue(1);
    update();
}

}
