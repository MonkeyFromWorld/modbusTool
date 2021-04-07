#include "common.h"
#include "navegation.h"
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include "ui_navegation.h"


Navegation::Navegation(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::Navegation)
{
    ui->setupUi(this);
    initForm();

    connect(ui->ToModbusClient, &QPushButton::clicked, this, &Navegation::on_ModbusClientButton_clicked);
    connect(ui->ToModbusServer, &QPushButton::clicked, this, &Navegation::on_ModbusServerButton_clicked);
}

Navegation::~Navegation()
{
    delete ui;
}

void Navegation::on_ModbusClientButton_clicked()
{
    emit display(MODBUS_CLIENT_PAGE);
}

void Navegation::on_ModbusServerButton_clicked()
{
    emit display(MODBUS_SERVER_PAGE);
}

void Navegation::initForm()
{
    this->initStyle();
}


void Navegation::initStyle()
{
    //加载样式表
    QString qss;
    QFile file(":/qss/navigation.qss");

    if (file.open(QFile::ReadOnly)) {
#if 1
        //用QTextStream读取样式文件不用区分文件编码 带bom也行
        QStringList list;
        QTextStream in(&file);
        //in.setCodec("utf-8");
        while (!in.atEnd()) {
            QString line;
            in >> line;
            list << line;
        }

        qss = list.join("\n");
#else
        //用readAll读取默认支持的是ANSI格式,如果不小心用creator打开编辑过了很可能打不开
        qss = QLatin1String(file.readAll());
#endif
        QString paletteColor = qss.mid(20, 7);
        this->setPalette(QPalette(QColor(paletteColor)));

        this->setStyleSheet(qss);
        file.close();
    }
}



