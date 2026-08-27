#include "devconfighelper.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace DevConfigHelper
{

QJsonObject deviceToJson(const DeviceEntity& dev)
{
    QJsonObject obj;
    obj["devUuid"] = dev.devUuid;
    obj["devName"] = dev.devName;
    obj["slaveId"] = dev.slaveId;
    obj["commType"] = dev.commType;
    obj["portParam"] = dev.portParam;
    obj["enable"] = dev.enable;

    QJsonArray regArr;
    for(const auto& reg : dev.regList)
    {
        QJsonObject ro;
        ro["name"] = reg.name;
        ro["addr"] = reg.addr;
        ro["len"] = reg.len;
        ro["lowAlarm"] = reg.lowAlarm;
        ro["highAlarm"] = reg.highAlarm;
        ro["dataType"] = static_cast<int>(reg.dataType);
        regArr.append(ro);
    }
    obj["regList"] = regArr;
    return obj;
}

DeviceEntity jsonToDevice(const QJsonObject& obj)
{
    DeviceEntity dev;
    dev.devUuid = obj["devUuid"].toString();
    dev.devName = obj["devName"].toString();
    dev.slaveId = obj["slaveId"].toInt();
    dev.commType = obj["commType"].toString();
    dev.portParam = obj["portParam"].toString();
    dev.enable = obj["enable"].toBool(false);

    QJsonArray regArr = obj["regList"].toArray();
    for(auto val : regArr)
    {
        QJsonObject ro = val.toObject();
        RegisterItem reg;
        reg.name = ro["name"].toString();
        reg.addr = ro["addr"].toInt();
        reg.len = ro["len"].toInt(1);
        reg.lowAlarm = ro["lowAlarm"].toDouble();
        reg.highAlarm = ro["highAlarm"].toDouble();
        reg.dataType = static_cast<DataType>(ro["dataType"].toInt());
        dev.regList.push_back(reg);
    }
    return dev;
}

bool saveDevices(const QVector<DeviceEntity> &devList, const QString &filePath)
{
    QJsonArray arr;
    for(auto& d : devList)
    {
        arr.append(deviceToJson(d));
    }
    QJsonDocument doc(arr);
    QFile f(filePath);
    if(!f.open(QIODevice::WriteOnly))
    {
        qWarning()<<"保存设备配置失败:"<<f.errorString();
        return false;
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

QVector<DeviceEntity> loadDevices(const QString &filePath)
{
    QVector<DeviceEntity> res;
    QFile f(filePath);
    if(!f.exists())
    {
        qDebug()<<"设备配置文件不存在，返回空列表";
        return res;
    }
    if(!f.open(QIODevice::ReadOnly))
    {
        qWarning()<<"读取设备配置失败:"<<f.errorString();
        return res;
    }
    QByteArray data = f.readAll();
    f.close();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if(!doc.isArray()) return res;

    QJsonArray arr = doc.array();
    for(auto val : arr)
    {
        auto dev = jsonToDevice(val.toObject());
        res.push_back(dev);
    }
    return res;
}

}
