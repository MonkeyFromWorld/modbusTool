#include "common.h"
#include "modbusClient.h"
#include "./modbusClient/clientSettingsdialog.h"
#include "ui_modbusclient.h"
#include <QFile>
#include <QTextStream>

enum ModbusConnection {
    Serial,
    Tcp
};

enum { NumColumn = 0, CoilsColumn = 1, HoldingColumn = 2, ColumnCount = 3, RowCount = 8192 };

ModbusClient::ModbusClient(QWidget* parent) :
    QMainWindow(parent),
    ui(new Ui::ModbusClient)
{
    setWindowTitle("Modbus Client");

    ui->setupUi(this);
    m_settingsDialog = new ClientSettingsDialog();
    initAction();

    this->model = new RegisterModel(this);
    ui->writeValueTable->setModel(this->model);

    // 添加 寄存器类型选择
    ui->registerType->addItem(tr("Coils"), QModbusDataUnit::Coils);
    ui->registerType->addItem(tr("Discrete Inputs"), QModbusDataUnit::DiscreteInputs);
    ui->registerType->addItem(tr("Input Registers"), QModbusDataUnit::InputRegisters);
    ui->registerType->addItem(tr("Holding Registers"), QModbusDataUnit::HoldingRegisters);

    setStartAddress(ui->writeAddress->value());
    setNumberOfValues(ui->writeSize->currentText());

#if QT_CONFIG(modbus_serialport)
    ui->connectType->setCurrentIndex(0);
    onConnectTypeChanged(0);
#else
    // lock out the serial port option
    ui->connectType->setCurrentIndex(1);
    onConnectTypeChanged(1);
    ui->connectType->setEnabled(false);
#endif

    //    auto model = new QStandardItemModel(10, 1, this);
    //    for (int i = 0; i < 10; ++i)
    //        model->setItem(i, new QStandardItem(QStringLiteral("%1").arg(i + 1)));
    //ui->writeSize->setModel(model);
    ui->writeSize->setCurrentText("1");

    for (int ii = 0; ii < REGISTER_QUANTITY; ++ii) {
        if (ii < ui->writeAddress->value() || ii > ui->writeAddress->value() + ui->writeSize->currentIndex()) {
            ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), true);
        }
        else
        {
            ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), false);
        }
    }

    // 写寄存器数量限制与自动变化
    connect(ui->writeSize, &QComboBox::currentTextChanged, this, [=](){
        if (ui->writeAddress->value() + ui->writeSize->currentIndex() > MAX_REGISTER_ADDRESS)
            ui->writeSize->setCurrentIndex(MAX_REGISTER_ADDRESS - ui->writeAddress->value());

        for (int ii = 0; ii < REGISTER_QUANTITY; ++ii) {
            if (ii < ui->writeAddress->value() || ii > ui->writeAddress->value() + ui->writeSize->currentIndex()) {
                ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), true);
            }
            else
            {
                ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), false);
            }
        }
    });
    connect(ui->writeSize, &QComboBox::currentTextChanged,
            this, &ModbusClient::setNumberOfValues);

    auto valueChanged = QOverload<int>::of(&QSpinBox::valueChanged);
    connect(ui->writeAddress, valueChanged, this, &ModbusClient::setStartAddress);
    connect(ui->writeAddress, valueChanged, this, [this]() {
        if (ui->writeAddress->value() + ui->writeSize->currentIndex() > MAX_REGISTER_ADDRESS)
            ui->writeSize->setCurrentIndex(MAX_REGISTER_ADDRESS - ui->writeAddress->value());

        for (int ii = 0; ii < REGISTER_QUANTITY; ++ii) {
            if (ii < ui->writeAddress->value() || ii > ui->writeAddress->value() + ui->writeSize->currentIndex()) {
                ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), true);
            }
            else
            {
                ui->writeValueTable->setRowHidden(ii, ui->writeValueTable->model()->index(ui->writeAddress->value(), 0), false);
            }
        }
        //        QModelIndex addressIndex = ui->writeValueTable->model()->index(ui->writeAddress->value(), 0);
        //        ui->writeValueTable->setCurrentIndex(addressIndex);   // 跳转到该行

    });

    // 读寄存器数量限制与自动变化
    ui->readSize->setCurrentText("1");
    connect(ui->readSize, &QComboBox::currentTextChanged, this, [=](){
        if (ui->readAddress->value() + ui->readSize->currentIndex() > MAX_REGISTER_ADDRESS)
            ui->readSize->setCurrentIndex(MAX_REGISTER_ADDRESS - ui->readAddress->value());
    });
    connect(ui->readAddress, valueChanged, this, [=]() {
        if (ui->readAddress->value() + ui->readSize->currentIndex() > MAX_REGISTER_ADDRESS)
            ui->readSize->setCurrentIndex(MAX_REGISTER_ADDRESS - ui->readAddress->value());
    });

}

ModbusClient::~ModbusClient()
{
    if (modbusDevice)
        modbusDevice->disconnectDevice();
    delete modbusDevice;
    delete model;
}

void ModbusClient::initAction()
{
    connect(ui->connectButton, &QPushButton::clicked, this, &ModbusClient::onConnectButtonClicked);
    connect(ui->actionConnect, &QAction::triggered, this, &ModbusClient::onConnectButtonClicked);
    connect(ui->actionDisconnect, &QAction::triggered, this, &ModbusClient::onConnectButtonClicked);
    connect(ui->readButton, &QPushButton::clicked, this, &ModbusClient::onReadButtonClicked);
    connect(ui->writeButton, &QPushButton::clicked, this, &ModbusClient::onWriteButtonClicked);
    connect(ui->readWriteButton, &QPushButton::clicked, this, &ModbusClient::onReadWriteButtonClicked);
    connect(ui->connectType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::onConnectTypeChanged);
    connect(ui->registerType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ModbusClient::onRegisterTypeChanged);
    connect(ui->actionOptions, &QAction::triggered, m_settingsDialog, &QDialog::show);
    connect(this, &ModbusClient::updateViewport, ui->writeValueTable->viewport(), QOverload<>::of(&QWidget::update));

    connect(ui->actionNavigation, &QAction::triggered, this, &ModbusClient::on_actionNavigation_triggered);
}

void ModbusClient::on_actionNavigation_triggered()
{
    m_settingsDialog->hide();
    emit display(NAVIGATION_PAGE);
}

void ModbusClient::setStartAddress(int address)
{
    model->m_address = address;
    emit ModbusClient::updateViewport();
}

void ModbusClient::setNumberOfValues(const QString &number)
{
    model->m_number = number.toInt();
    emit ModbusClient::updateViewport();
}

// 连接类型改变时
void ModbusClient::onConnectTypeChanged(int index)
{
    if (modbusDevice) {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
        modbusDevice = nullptr;
    }

    auto type = static_cast<ModbusConnection>(index);
    if (type == Serial) {
#if QT_CONFIG(modbus_serialport)
        modbusDevice = new QModbusRtuSerialMaster();
#endif
    } else if (type == Tcp) {
        modbusDevice = new QModbusTcpClient();
        if (ui->portEdit->text().isEmpty())
            ui->portEdit->setText(QLatin1String("127.0.0.1:502"));
    }

    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        ui->statusBar->showMessage(modbusDevice->errorString(), 5000);
    });

    if (!modbusDevice) {
        ui->connectButton->setDisabled(true);
        if (type == Serial)
            ui->statusBar->showMessage(tr("Could not create Modbus master."), 5000);
        else
            ui->statusBar->showMessage(tr("Could not create Modbus client."), 5000);
    } else {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &ModbusClient::onModbusStateChanged);
    }
}

// 连接按钮按下，进行连接或断开连接
void ModbusClient::onConnectButtonClicked()
{
    if (!modbusDevice)
        return;

    ui->statusBar->clearMessage();
    if (modbusDevice->state() != QModbusDevice::ConnectedState) {
        if (static_cast<ModbusConnection>(ui->connectType->currentIndex()) == Serial) {
            modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                ui->portEdit->text());
        } else {
            const QUrl url = QUrl::fromUserInput(ui->portEdit->text());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }

        if (!modbusDevice->connectDevice()) {
            ui->statusBar->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        } else {

        }
    } else {
        modbusDevice->disconnectDevice();
    }
}

// 连接或不连接按钮点击时，显示状态切换
void ModbusClient::onModbusStateChanged(int state)
{
    if (state == QModbusDevice::UnconnectedState)
        ui->connectButton->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->connectButton->setText(tr("Disconnect"));
}

// 读寄存器按钮被点击
void ModbusClient::onReadButtonClicked()
{
    if (!modbusDevice)
        return;

    // 先清除显示区数据
    ui->readValue->clear();
    ui->statusBar->clearMessage();

    // 调用函数进行读寄存器，完成后去onReadReady函数，删除指针，或显示错误
    if (auto *reply = modbusDevice->sendReadRequest(readRequest(), ui->serverEdit->value())) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusClient::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        ui->statusBar->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

// 读取完成后，显示出来
void ModbusClient::onReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        // 循环读写的寄存器个数，根据起始地址赋值
        for (int i = 0, total = int(unit.valueCount()); i < total; ++i) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress() + i)
                                     .arg(QString::number(unit.value(i), 10)); // 都以十进制显示
            ui->readValue->addItem(entry);
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        ui->statusBar->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 10), 5000);
    } else {
        ui->statusBar->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 10), 5000);
    }

    reply->deleteLater();
}

// 写寄存器的按钮被点击
void ModbusClient::onWriteButtonClicked()
{
    if (!modbusDevice)
        return;
    ui->statusBar->clearMessage();

    // 设置参数，寄存器赋值
    QModbusDataUnit writeUnit = writeRequest();
    QModbusDataUnit::RegisterType table = writeUnit.registerType();
    for (int i = 0, total = int(writeUnit.valueCount()); i < total; ++i) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, model->m_coils[i + writeUnit.startAddress()]);
        else
            writeUnit.setValue(i, model->m_holdingRegisters[i + writeUnit.startAddress()]);
    }

    // 调函数写寄存器，完成后，有错误显示错误，无错误结束
    if (auto *reply = modbusDevice->sendWriteRequest(writeUnit, ui->serverEdit->value())) {
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    ui->statusBar->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 10),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    ui->statusBar->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 10), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        ui->statusBar->showMessage(tr("Write error: ") + modbusDevice->errorString(), 5000);
    }
}

// 同时读写的按钮被点击，先写后读
void ModbusClient::onReadWriteButtonClicked()
{
    if (!modbusDevice)
        return;
    ui->readValue->clear();
    ui->statusBar->clearMessage();

    QModbusDataUnit writeUnit = writeRequest(); // 通讯设置参数
    QModbusDataUnit::RegisterType table = writeUnit.registerType(); // 寄存器类型
    // 寄存器赋值，从0开始？
    for (int i = 0, total = int(writeUnit.valueCount()); i < total; ++i) {
        if (table == QModbusDataUnit::Coils)
            writeUnit.setValue(i, model->m_coils[i + writeUnit.startAddress()]);
        else
            writeUnit.setValue(i, model->m_holdingRegisters[i + writeUnit.startAddress()]);
    }

    // 调用函数写寄存器
    if (auto *reply = modbusDevice->sendReadWriteRequest(readRequest(), writeUnit,
        ui->serverEdit->value())) {
        // 未完成时设置连接槽函数onReadReady，完成后删除判断的指针
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &ModbusClient::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        // 否则错误
        ui->statusBar->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

// 寄存器选择类型变化时界面变化
void ModbusClient::onRegisterTypeChanged(int index)
{
    const bool coilsOrHolding = index == 0 || index == 3;
    // 寄存器选择类型变化时，判断如果是非可写寄存器，则屏蔽寄存器数值输入
    if (coilsOrHolding) {
        ui->writeValueTable->setColumnHidden(1, index != 0);
        ui->writeValueTable->setColumnHidden(2, index != 3);
        ui->writeValueTable->resizeColumnToContents(0);
    }
    // 根据寄存器类型屏蔽读或写按钮
    ui->readWriteButton->setEnabled(index == 3);
    ui->writeButton->setEnabled(coilsOrHolding);
    ui->writeGroupBox->setEnabled(coilsOrHolding);
}

// 通讯设置参数赋值
QModbusDataUnit ModbusClient::readRequest() const
{
    // 寄存器类型
    const auto table =
        static_cast<QModbusDataUnit::RegisterType>(ui->registerType->currentData().toInt());
    // 起始地址
    int startAddress = ui->readAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress <= MAX_REGISTER_ADDRESS);

    // do not go beyond 10 entries
    // 寄存器数量
    quint16 numberOfEntries = qMin(ui->readSize->currentText().toUShort(), quint16(1+MAX_REGISTER_ADDRESS - startAddress));
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}

// 通讯设置参数赋值
QModbusDataUnit ModbusClient::writeRequest() const
{
    // 寄存器类型
    const auto table =
        static_cast<QModbusDataUnit::RegisterType>(ui->registerType->currentData().toInt());

    // 起始地址
    int startAddress = ui->writeAddress->value();
    Q_ASSERT(startAddress >= 0 && startAddress < REGISTER_QUANTITY);

    // do not go beyond 10 entries
    // 寄存器数量
    quint16 numberOfEntries = qMin(ui->writeSize->currentText().toUShort(), quint16(REGISTER_QUANTITY - startAddress));
    return QModbusDataUnit(table, startAddress, numberOfEntries);
}


RegisterModel::RegisterModel(QObject* parent) :
    QAbstractTableModel(parent),
    m_coils(RowCount, false), m_holdingRegisters(RowCount, 0u)
{

}

RegisterModel::~RegisterModel()
{
}


int RegisterModel::rowCount(const QModelIndex &/*parent*/) const
{
    return RowCount;
}

int RegisterModel::columnCount(const QModelIndex &/*parent*/) const
{
    return ColumnCount;
}

QVariant RegisterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QVariant();

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (index.column() == NumColumn && role == Qt::DisplayRole)
        return QString::number(index.row());

    if (index.column() == CoilsColumn && role == Qt::CheckStateRole) // coils
        return m_coils.at(index.row()) ? Qt::Checked : Qt::Unchecked;

    if (index.column() == HoldingColumn && role == Qt::DisplayRole) // holding registers
        return QString("%1").arg(QString::number(m_holdingRegisters.at(index.row()), 10));

    return QVariant();

}

QVariant RegisterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case NumColumn:
            return QStringLiteral("#");
        case CoilsColumn:
            return QStringLiteral("Coils  ");
        case HoldingColumn:
            return QStringLiteral("Holding Registers");
        default:
            break;
        }
    }
    return QVariant();
}

bool RegisterModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() ||  index.row() >= RowCount || index.column() >= ColumnCount)
        return false;

    Q_ASSERT(m_coils.count() == RowCount);
    Q_ASSERT(m_holdingRegisters.count() == RowCount);

    if (index.column() == CoilsColumn && role == Qt::CheckStateRole) { // coils
        auto s = static_cast<Qt::CheckState>(value.toUInt());
        s == Qt::Checked ? m_coils.setBit(index.row()) : m_coils.clearBit(index.row());
        emit dataChanged(index, index);
        return true;
    }

    if (index.column() == HoldingColumn && role == Qt::EditRole) { // holding registers
        bool result = false;
        quint16 newValue = value.toString().toUShort(&result, 10);
        if (result)
            m_holdingRegisters[index.row()] = newValue;

        emit dataChanged(index, index);
        return result;
    }

    return false;
}

Qt::ItemFlags RegisterModel::flags(const QModelIndex &index) const
{
    if (!index.isValid() || index.row() >= RowCount || index.column() >= ColumnCount)
        return QAbstractTableModel::flags(index);

    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if ((index.row() < m_address) || (index.row() >= (m_address + m_number)))
        flags &= ~Qt::ItemIsEnabled;

    if (index.column() == CoilsColumn) // coils
        return flags | Qt::ItemIsUserCheckable;
    if (index.column() == HoldingColumn) // holding registers
        return flags | Qt::ItemIsEditable;

    return flags;
}

