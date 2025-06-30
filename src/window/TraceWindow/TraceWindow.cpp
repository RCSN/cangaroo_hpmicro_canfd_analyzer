/*

  Copyright (c) 2015, 2016 Hubert Denkmair <hubert@denkmair.de>

  This file is part of cangaroo.

  cangaroo is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  cangaroo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with cangaroo.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "TraceWindow.h"
#include "ui_TraceWindow.h"

#include <QDomDocument>
#include <QSortFilterProxyModel>
#include "LinearTraceViewModel.h"
#include "AggregatedTraceViewModel.h"
#include "TraceFilterModel.h"
#include <core/MeasurementSetup.h>
#include <core/CanTrace.h>
#include <core/Backend.h>

#include <window/TraceWindow/TraceWindow.h>
#include <window/SetupDialog/SetupDialog.h>
#include <window/LogWindow/LogWindow.h>
#include <window/TraceWindow/AutoHeightDelegate.h>

#include <QDebug>

TraceWindow::TraceWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::TraceWindow),
    _backend(&backend)
{
    ui->setupUi(this);
    // 添加自定义委托
    ui->tree->setItemDelegate(new AutoHeightDelegate(this));

    _linearTraceViewModel = new LinearTraceViewModel(backend);
    _linearProxyModel = new QSortFilterProxyModel(this);
    _linearProxyModel->setSourceModel(_linearTraceViewModel);
    _linearProxyModel->setDynamicSortFilter(true);

    _aggregatedTraceViewModel = new AggregatedTraceViewModel(backend);
    _aggregatedProxyModel = new QSortFilterProxyModel(this);
    _aggregatedProxyModel->setSourceModel(_aggregatedTraceViewModel);
    _aggregatedProxyModel->setDynamicSortFilter(true);

    _aggFilteredModel = new TraceFilterModel(this);
    _aggFilteredModel->setSourceModel(_aggregatedProxyModel);
    _linFilteredModel = new TraceFilterModel(this);
    _linFilteredModel->setSourceModel(_linearProxyModel);


    setMode(mode_aggregated);
    setAutoScroll(false);

    QFont font("consolas");
    //QFont font("Monospace");//Colin
    font.setStyleHint(QFont::TypeWriter);
    ui->tree->setWordWrap(true);
    ui->tree->setTextElideMode(Qt::ElideNone);
    ui->tree->setUniformRowHeights(false);  // 允许动态行高
    ui->tree->setFont(font);
    ui->tree->setAlternatingRowColors(true);

    ui->tree->setColumnWidth(0, 110);//Timestamp
    ui->tree->setColumnWidth(1, 120);//Port
    ui->tree->setColumnWidth(2, 70);//Rx/Tx
    ui->tree->setColumnWidth(3, 110);//CAN_ID
    ui->tree->setColumnWidth(4, 80);//DB_Sender
    ui->tree->setColumnWidth(5, 70);//DB_Name
    ui->tree->setColumnWidth(6, 80);//DB_comment

    ui->tree->setColumnWidth(7, 90);//Frame Type
    ui->tree->setColumnWidth(8, 90);//Frame Format
    ui->tree->setColumnWidth(9, 80);//CAN Type

    ui->tree->setColumnWidth(10, 50);//DLC
    ui->tree->setColumnWidth(11, 1000);//Data
    ui->tree->sortByColumn(BaseTraceViewModel::column_canid, Qt::AscendingOrder);

    ui->cbTimestampMode->addItem("Absolute", 0);
    ui->cbTimestampMode->addItem("Relative", 1);
    ui->cbTimestampMode->addItem("Delta", 2);

    ui->cbAggregateMode->addItem("ID", 0);//Colin
    ui->cbAggregateMode->addItem("*Port", 1);

    _backend->getTrace()->setDisplayChannel(ui->cbDispChannel->currentIndex());

    setTimestampMode(timestamp_mode_relative);

    connect(_linearTraceViewModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));

    connect(ui->filterLineEdit, SIGNAL(textChanged(QString)), this, SLOT(on_cbFilterChanged()));
    connect(ui->btTraceClear, SIGNAL(clicked()), this, SLOT(on_btTraceClear_triggered()));
}

TraceWindow::~TraceWindow()
{
    delete ui;
    delete _aggregatedTraceViewModel;
    delete _linearTraceViewModel;
}

void TraceWindow::setMode(TraceWindow::mode_t mode)
{
    bool isChanged = (_mode != mode);
    _mode = mode;

    if (_mode==mode_linear) {
        ui->tree->setSortingEnabled(false);
        ui->tree->setModel(_linFilteredModel); //_linearTraceViewModel);
        ui->cbAutoScroll->setEnabled(true);
    } else {
        ui->tree->setSortingEnabled(true);
        ui->tree->setModel(_aggFilteredModel); //_aggregatedProxyModel);
        ui->cbAutoScroll->setEnabled(false);
    }

    if (isChanged) {
        ui->cbAggregated->setChecked(_mode==mode_aggregated);
        emit(settingsChanged(this));
    }
}

void TraceWindow::setAutoScroll(bool doAutoScroll)
{
    if (doAutoScroll != _doAutoScroll) {
        _doAutoScroll = doAutoScroll;
        ui->cbAutoScroll->setChecked(_doAutoScroll);
        emit(settingsChanged(this));
    }
}

void TraceWindow::setTimestampMode(int mode)
{
    timestamp_mode_t new_mode;
    if ( (mode>=0) && (mode<timestamp_modes_count) ) {
        new_mode = (timestamp_mode_t) mode;
    } else {
        new_mode = timestamp_mode_absolute;
    }

    _aggregatedTraceViewModel->setTimestampMode(new_mode);
    _linearTraceViewModel->setTimestampMode(new_mode);

    if (new_mode != _timestampMode) {
        _timestampMode = new_mode;
        for (int i=0; i<ui->cbTimestampMode->count(); i++) {
            if (ui->cbTimestampMode->itemData(i).toInt() == new_mode) {
                ui->cbTimestampMode->setCurrentIndex(i);
            }
        }
        emit(settingsChanged(this));
    }
}

bool TraceWindow::saveXML(Backend &backend, QDomDocument &xml, QDomElement &root)
{
    if (!ConfigurableWidget::saveXML(backend, xml, root)) {
        return false;
    }

    root.setAttribute("type", "TraceWindow");
    root.setAttribute("mode", (_mode==mode_linear) ? "linear" : "aggregated");
    root.setAttribute("TimestampMode", _timestampMode);

    QDomElement elLinear = xml.createElement("LinearTraceView");
    elLinear.setAttribute("AutoScroll", (ui->cbAutoScroll->checkState() == Qt::Checked) ? 1 : 0);
    root.appendChild(elLinear);

    QDomElement elAggregated = xml.createElement("AggregatedTraceView");
    elAggregated.setAttribute("SortColumn", _aggregatedProxyModel->sortColumn());
    root.appendChild(elAggregated);

    return true;
}

bool TraceWindow::loadXML(Backend &backend, QDomElement &el)
{
    if (!ConfigurableWidget::loadXML(backend, el)) {
        return false;
    }

    setMode((el.attribute("mode", "linear") == "linear") ? mode_linear : mode_aggregated);
    setTimestampMode(el.attribute("TimestampMode", "0").toInt());

    QDomElement elLinear = el.firstChildElement("LinearTraceView");
    setAutoScroll(elLinear.attribute("AutoScroll", "0").toInt() != 0);

    QDomElement elAggregated = el.firstChildElement("AggregatedTraceView");
    int sortColumn = elAggregated.attribute("SortColumn", "-1").toInt();
    ui->tree->sortByColumn(sortColumn,Qt::AscendingOrder);
//Colin    AscendingOrder,    DescendingOrder
    return true;
}

void TraceWindow::rowsInserted(const QModelIndex &parent, int first, int last)
{
    (void) parent;
    (void) first;
    (void) last;

    if ((_mode==mode_linear) && (ui->cbAutoScroll->checkState() == Qt::Checked)) {
        ui->tree->scrollToBottom();
    }

}

void TraceWindow::on_cbAggregated_stateChanged(int i)
{
    setMode( (i==Qt::Checked) ? mode_aggregated : mode_linear );
}

void TraceWindow::on_cbAutoScroll_stateChanged(int i)
{
    setAutoScroll(i==Qt::Checked);
}

void TraceWindow::on_cbTimestampMode_currentIndexChanged(int index)
{
    setTimestampMode((timestamp_mode_t)ui->cbTimestampMode->itemData(index).toInt());
}

void TraceWindow::on_cbFilterChanged()
{
    _aggFilteredModel->setFilterText(ui->filterLineEdit->text());
    _linFilteredModel->setFilterText(ui->filterLineEdit->text());
    _aggFilteredModel->invalidate();
    _linFilteredModel->invalidate();
}


void TraceWindow::on_btTraceClear_triggered()
{
    _backend->clearTrace();//Colin
    log_info(">>Clear Trace<<");
}


void TraceWindow::on_cbShowSend_clicked(bool checked)
{
    _backend->set_show_send(checked);
}

void TraceWindow::on_cbDispChannel_currentIndexChanged(int index)
{
    _backend->getTrace()->setDisplayChannel(index);
}
