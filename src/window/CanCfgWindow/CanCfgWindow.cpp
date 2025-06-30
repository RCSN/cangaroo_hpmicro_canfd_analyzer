#include "CanCfgWindow.h"
#include "ui_CanCfgWindow.h"
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include <QPaintEvent>
#include <QPainter>
#include <core/Backend.h>
#include <core/MeasurementSetup.h>
#include <core/MeasurementNetwork.h>
#include <core/MeasurementInterface.h>
CanCfgWindow::CanCfgWindow(QWidget *parent, Backend &backend) :
    ConfigurableWidget(parent),
    ui(new Ui::CanCfgWindow),
    _backend(backend),
    _timer(new QTimer(this))

{
    _first_open = true;
    ui->setupUi(this);
    refreshInterfaces();
    connect(&backend, SIGNAL(beginMeasurement()),  this, SLOT(refreshInterfaces()));
    connect(ui->cbEnableRes, SIGNAL(clicked(bool)), this, SLOT(cbEnableRes_clicked(bool)));
    connect(ui->comboBoxInterface, SIGNAL(currentIndexChanged(const QString)), this, SLOT(comboBoxInterface_currentIndexChanged(const QString)));
}

CanCfgWindow::~CanCfgWindow()
{
    delete ui;
    CanDevList.clear();
}

void CanCfgWindow::disableWindow(int disable)
{
    if(disable)
    {
        this->setDisabled(1);
    }
    else
    {
        // Only enable if an interface is present
        if(_backend.getInterfaceList().count() > 0)
            this->setDisabled(0);
        else
            this->setDisabled(1);
    }
}

void CanCfgWindow::refreshInterfaces()
{
    CanInterface *intf;
    qDebug() << "CanCfgWindow::refreshInterfaces";
    ui->comboBoxInterface->clear();
    for (int i = 0; i < CanDevList.size(); i++) {
        intf = (CanInterface *)CanDevList.at(i);
        disconnect(intf, &CanInterface::_hpm_request_msg, this, &CanCfgWindow::canDevRequestMsg);
    }
    CanDevList.clear();
    int cb_idx = 0;

    // TODO: Only show interfaces that are included in active MeasurementInterfaces!
    foreach (CanInterfaceId ifid, _backend.getInterfaceList()) {
        intf = _backend.getInterfaceById(ifid);

        if((intf != nullptr) && (intf->isOpen()))
        {
            CanDevList.append(intf);
            ui->comboBoxInterface->addItem(intf->getName() + " " + intf->getDescription());//Colin
            ui->comboBoxInterface->setItemData(cb_idx, QVariant(ifid));
            cb_idx++;         
        }
    }

    for (int i = 0; i < CanDevList.size(); i++) {
        intf = (CanInterface *)CanDevList.at(i);
        connect(intf, &CanInterface::_hpm_request_msg, this, &CanCfgWindow::canDevRequestMsg);
    }
    intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    if (intf == nullptr) {
        qDebug() << "refreshInterfaces: " << "can't find interface" << ui->comboBoxInterface->currentData().toUInt();
        return;
    }
    ui->cbEnableRes->setEnabled(false);
    ui->comboBoxInterface->setEnabled(false);
    ui->cbEnableRes->setChecked(intf->get_enable_terminal_res());
    ui->cbEnableRes->setEnabled(true);
    ui->comboBoxInterface->setEnabled(true);
    if(cb_idx == 0)
        disableWindow(1);
    else
        disableWindow(0);
}

void CanCfgWindow::comboBoxInterface_currentIndexChanged(const QString &arg1)
{
    CanInterface *intf = nullptr;
    if (_first_open == true) {
        _first_open = false;

    }
    (void)arg1;
    intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    if (intf == nullptr) {
        qDebug() << "CanCfgWindow11: " << "can't find interface" << ui->comboBoxInterface->currentData().toUInt();
        return;
    }
    intf->get_enable_terminal_res();
}

void CanCfgWindow::paintEvent(QPaintEvent *event)
{
   (void)event;
   QPainter p(this);
   p.setPen(QColor("black"));
   p.drawRect(0, 0, width() -1, height() -1);
}

Backend &CanCfgWindow::backend()
{
    return _backend;

}

void CanCfgWindow::cbEnableRes_clicked(bool checked)
{
    QString current_inteface_str = ui->comboBoxInterface->currentText();
    QStringList strs= current_inteface_str.split(" ");
    CanInterface *intf = nullptr;
    if (_first_open == true) {
        _first_open = false;
        refreshInterfaces();
    }
    intf = _backend.getInterfaceById((CanInterfaceId)ui->comboBoxInterface->currentData().toUInt());
    if (intf == nullptr) {
        qDebug() << "CanCfgWindow22: " << "can't find interface";
        return;
    }
    ui->cbEnableRes->setEnabled(false);
    ui->comboBoxInterface->setEnabled(false);
    intf->set_enable_terminal_res(checked);
    ui->cbEnableRes->setEnabled(true);
    ui->comboBoxInterface->setEnabled(true);
}

void CanCfgWindow::canDevRequestMsg(QByteArray msg)
{
    qDebug() << "canDevRequestMsg" << msg;
    int channel;
    bool enable_res;
    bool containsSubstring;
    QString str_msg = QString::fromLocal8Bit(msg);
    QStringList parts;
    containsSubstring = str_msg.contains("hpm_cfg_g_120r_");
    if (containsSubstring == true) {
        str_msg.remove("hpm_cfg_g_120r_");
        parts = str_msg.split("_");
        channel = parts.at(0).toInt();
        enable_res = parts.at(1).toInt();
        if (channel == ui->comboBoxInterface->currentIndex()) {
            qDebug() << parts;
            ui->cbEnableRes->setChecked(enable_res);
        }
    }
}
