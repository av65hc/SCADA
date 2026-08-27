#ifndef MODBUSMASTER_H
#define MODBUSMASTER_H

#include <QObject>
#include <QVector>
#include <QModbusDataUnit>
#include <cstring>

#include "../entity/registeritem.h"

class QModbusClient;

class ModbusMaster : public QObject{
    Q_OBJECT
public:
    explicit ModbusMaster(QObject* parent = nullptr);
    ~ModbusMaster();
    bool openSerial(const QString & portParam);
    bool openTcp(const QString &hostParam);
    void close();
    bool isConnected() const;
    static double decodeValue(const QVector<uint16_t>& raw,DataType type);
    int readHoldReg(int slavedId,int startAddr, int count, QVector<uint16_t> &out);
    int readInputReg(int slavedId,int startAddr,int count,QVector<uint16_t> &out);
private:
    int readReg(QModbusDataUnit::RegisterType type,int slaveId,int startAddr,int count,QVector<uint16_t>& out);
    int writeSingleReg(int slaveId,int addr, uint16_t value);
    QModbusClient *m_client = nullptr;
};

#endif