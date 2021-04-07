#ifndef WIDGET_H
#define WIDGET_H

#include <QMainWindow>
#include <QTimer>
#include <QStackedLayout>

class Navegation;
class ModbusClient;
class ModbusServer;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

protected:
    QTimer modbusTimer;
    QThread *modbusThread;

private:
    Navegation* navegation = nullptr;
    ModbusClient* modbusClient = nullptr;
    ModbusServer* modbusServer = nullptr;

    QStackedLayout* stackLayout;
    QVBoxLayout* mainLayout;
    void mySetWindowTitle(int page);

};

#endif // WIDGET_H
