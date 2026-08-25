#include "modbusmaster.h"
#include <QModbusRtuSerialClient>
#include <QModbusTcpClient>
#include <QModbusReply>
#include <QModbusDevice>
#include <QSerialPort>
#include <QEventLoop>
#include <QDebug>
#include <QVariant>

ModbusMaster::ModbusMaster(QObject *parent) : QObject(parent){}
ModbusMaster::~ModbusMaster(){close();}

bool ModbusMaster::openSerial(const QString&portParam){
    QStringList parts = portParam.split(',',Qt::SkipEmptyParts);
    if(parts.isEmpty()) return false;

    auto* master = new QModbusRtuSerialClient(this);
    master->setConnectionParameter(QModbusDevice::SerialPortNameParameter,QVariant(parts[0].trimmed()));
    int baud = parts.size()>1 ?parts[1].toInt() : 9600;
    int databits = parts.size()>2 ?parts[2].toInt() : 8;
    QString parity = parts.size()>3 ?parts[3].trimmed().toUpper().at(0) : 'N';
    int stopbits = parts.size()>4 ?parts[4].toInt() : 1;

    master->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,baud);
    master->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,databits);
    master->setConnectionParameter(QModbusDevice::SerialParityParameter,parity == 'E' ? QSerialPort::EvenParity :
                                   parity == 'O' ? QSerialPort::OddParity : QSerialPort::NoParity);
    master->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,stopbits == 2 ? QSerialPort::TwoStop : QSerialPort::OneStop);

    master->setTimeout(200);
    master->setNumberOfRetries(2);

    m_client = master;
    if(!m_client->connectDevice()){
        qWarning()<<"串口链接失败："<<m_client->errorString();
        return false;
    }
    return true;
}

bool ModbusMaster::openTcp(const QString &hostParam){
    QStringList parts = hostParam.split(':');
    if(parts.size()<2) return false;
    auto* client = new QModbusTcpClient(this);
    client->setConnectionParameter(QModbusDevice::NetworkAddressParameter,parts[0].trimmed());
    client->setConnectionParameter(QModbusDevice::NetworkPortParameter,parts[1].toInt());
    client->setTimeout(1000);
    client->setNumberOfRetries(2);

    m_client = client;
    if (!m_client->connectDevice()) {
        qWarning() << "TCP 连接失败:" << m_client->errorString();
        return false;
    }
    return true;
}

void ModbusMaster::close(){
    if(m_client){
        m_client->disconnect();
        m_client->deleteLater();
        m_client = nullptr;
    }
}

bool ModbusMaster::isConnected() const{
    return m_client && m_client->state() == QModbusDevice::ConnectedState;
}

int ModbusMaster::readHoldReg(int slaveId, int startAddr, int count, QVector<uint16_t> &out){
    return readReg(QModbusDataUnit::HoldingRegisters,slaveId,startAddr,count,out);
}

int ModbusMaster::readInputReg(int slaveId, int startAddr, int count,
                               QVector<uint16_t>& out)
{
    return readReg(QModbusDataUnit::InputRegisters, slaveId, startAddr, count, out);
}

int ModbusMaster::readReg(QModbusDataUnit::RegisterType type, int slaveId, int startAddr, int count, QVector<uint16_t> &out){
    if (!m_client || m_client->state() != QModbusDevice::ConnectedState)
        return -1;   // 未连接

    QModbusDataUnit unit(type, startAddr, count);
    QModbusReply* reply = m_client->sendReadRequest(unit, slaveId);
    if (!reply)
        return -2;   // 请求发送失败

    // 异步转同步：用局部事件循环阻塞等待
    QEventLoop loop;
    connect(reply, &QModbusReply::finished, &loop, &QEventLoop::quit);
    if (!reply->isFinished())
        loop.exec();

    if (reply->error() != QModbusDevice::NoError) {
        int err = reply->error();
        reply->deleteLater();
        return err;   // 返回 Qt 的错误码
    }

    const QModbusDataUnit result = reply->result();
    out.clear();
    for (uint i = 0; i < result.valueCount(); ++i)
        out.append(result.value(i));

    reply->deleteLater();
    return 0;   // 成功
}

int ModbusMaster::writeSingleReg(int slaveId,int addr, uint16_t value){
    if(!m_client || m_client->state() != QModbusDevice::ConnectedState) return -1;
    QModbusDataUnit unit(QModbusDataUnit::HoldingRegisters,addr,1);
    unit.setValue(0,value);
    QModbusReply* reply = m_client->sendWriteRequest(unit,slaveId);
    if(!reply){
        return -2;
    }
    QEventLoop loop;
    connect(reply,&QModbusReply::finished, &loop, &QEventLoop::quit);
    if(!reply->isFinished())
        loop.exec();
    int err = reply->error();
    reply->deleteLater();
    return err;
}