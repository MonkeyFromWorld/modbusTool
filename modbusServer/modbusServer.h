/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the QtSerialBus module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef MODBUS_SERVER_H
#define MODBUS_SERVER_H

#define NAVIGATION_PAGE 0
#define MODBUS_CLIENT_PAGE 1
#define MODBUS_SERVER_PAGE 2
#define XLSX_DEAL_PAGE 3

#define REGISTER_QUANTITY 0X2000
#define MAX_REGISTER_ADDRESS (REGISTER_QUANTITY-1)

#include <QButtonGroup>
#include <QMainWindow>
#include <QModbusServer>

QT_BEGIN_NAMESPACE

class QLineEdit;
class QCheckBox;
class QLabel;

namespace Ui {
class ModbusServer;
class ServerSettingsDialog;
}

QT_END_NAMESPACE

class ServerSettingsDialog;

class ModbusServer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ModbusServer(QWidget *parent = nullptr);
    ~ModbusServer();

signals:
    void display(int number);

private slots:
    void on_actionNavigation_triggered();

    void onCoilStartAddressChange(int startAddress);
    void onDIStartAddressChange(int startAddress);
    void onIRStartAddressChange(int startAddress);
    void onHRStartAddressChange(int startAddress);

    void onConnectButtonClicked();
    void onStateChanged(int state);

    void coilChanged(int id);
    void discreteInputChanged(int id);
    void bitChanged(int id, QModbusDataUnit::RegisterType table, bool value);

    void setRegister(const QString &value);
    void updateWidgets(QModbusDataUnit::RegisterType table, int address, int size);

    void onCurrentConnectTypeChanged(int);
    void handleDeviceError(QModbusDevice::Error newError);

private:
    void initActions();
    void setupDeviceData();
    void setupWidgetContainers();

    Ui::ModbusServer *ui = nullptr;
    QModbusServer *modbusDevice = nullptr;

    QButtonGroup coilButtons;
    QButtonGroup discreteButtons;
    QList<QLabel* > IR_addressShow;
    QList<QLabel* > HR_addressShow;
    QHash<QString, QLineEdit *> registers;

    ServerSettingsDialog *m_settingsDialog = nullptr;
};

#endif // MODBUS_SERVER_H
