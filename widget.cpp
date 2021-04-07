#pragma execution_character_set("utf-8")
#include "widget.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qtranslator.h"
#include "qdesktopwidget.h"
#include "qdebug.h"

#include <QStandardItemModel>
#include <QModelIndex>
#include <QStatusBar>
#include <QPushButton>
#include <QUrl>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QMenu>
#include <QTimer>
#include <QThread>

#include "common.h"
//#include "./login/login.h"
#include "./navegation/navegation.h"
#include "./modbusClient/modbusClient.h"
#include "./modbusServer/modbusServer.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    navegation = new Navegation();
    modbusClient = new ModbusClient();
    modbusServer = new ModbusServer();

    stackLayout = new QStackedLayout;

    stackLayout->addWidget(navegation);
    stackLayout->addWidget(modbusClient);
    stackLayout->addWidget(modbusServer);
    connect(navegation, &Navegation::display, stackLayout, &QStackedLayout::setCurrentIndex);
    connect(modbusClient, &ModbusClient::display, stackLayout, &QStackedLayout::setCurrentIndex);
    connect(modbusServer, &ModbusServer::display, stackLayout, &QStackedLayout::setCurrentIndex);

    connect(navegation, static_cast<void (Navegation::*)(int)>(&Navegation::display), this, [=](int page){
        mySetWindowTitle(page);
    });
    connect(modbusClient, static_cast<void (ModbusClient::*)(int)>(&ModbusClient::display), this, [=](int page){
        mySetWindowTitle(page);
    });
    connect(modbusServer, static_cast<void (ModbusServer::*)(int)>(&ModbusServer::display), this, [=](int page){
        mySetWindowTitle(page);
    });

    mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0,0,0,0);// 设置布局边界与承载这个布局的窗体的边界之间的间隙为零, 消除白边
    mainLayout->addLayout(stackLayout);
    setLayout(mainLayout);

}

Widget::~Widget()
{
}

void Widget::mySetWindowTitle(int page)
{
    switch (page) {
        case NAVIGATION_PAGE: setWindowTitle("Navigation"); break;
        case MODBUS_CLIENT_PAGE: setWindowTitle("modbus client"); break;
        case MODBUS_SERVER_PAGE: setWindowTitle("modbus server"); break;
    default: break;
    }
}




