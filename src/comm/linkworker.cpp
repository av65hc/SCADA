#include "linkworker.h"
#include "modbusmaster.h"
#include <QDebug>

LinkWorker::LinkWorker(QObject *parent)
    : QObject{parent}
{
}

LinkWorker::~LinkWorker()
{
    stopWork();
}

void LinkWorker::setDeviceList(const QVector<DeviceEntity> &devList)
{
    m_devList = devList;
}

QString LinkWorker::getPortParam() const
{
    if(!m_devList.isEmpty())
        return m_devList.first().portParam;
    return "";
}

void LinkWorker::startWork()
{
    m_needExit = false;
    m_running = true;
}

void LinkWorker::stopWork()
{
    m_running = false;
    m_needExit = true;
    m_waitCond.wakeAll(); //立刻唤醒等待，消除msleep延迟
}

void LinkWorker::slotTaskLoop()
{
    qDebug()<<"LinkWorker采集任务槽启动";
    ModbusMaster master;

    if(m_devList.isEmpty())
    {
        emit sigLinkStatus(false,"链路设备列表为空");
        return;
    }
    QString portParam = m_devList.first().portParam;

    bool openOk = master.openSerial(portParam);
    if(!openOk)
    {
        emit sigLinkStatus(false, "串口打开失败:" + portParam);
        return;
    }
    emit sigLinkStatus(true, "串口打开成功:"+portParam);

    while(true)
    {
        //原子标记，立刻检测退出
        if(!m_running || m_needExit)
            break;

        for(const auto& dev : m_devList)
        {
            if(!m_running || m_needExit) break;
            if(!dev.enable) continue;

            for(const auto& reg : dev.regList)
            {
                if(!m_running || m_needExit) break;

                QVector<uint16_t> retData;
                int ret = master.readHoldReg(dev.slaveId, reg.addr, reg.len, retData);
                if(ret != 0)
                {
                    emit sigLinkStatus(false, QString("设备%1 寄存器%2读取失败").arg(dev.devName).arg(reg.addr));
                    continue;
                }

                double val = retData.at(0);
                CollectDataItem item;
                item.devUuid = dev.devUuid;
                item.regName = reg.name;
                item.value = val;
                item.time = QDateTime::currentDateTime();
                emit sigCollectData(item);
            }
        }

        QMutexLocker locker(&m_mutex);
        m_waitCond.wait(&m_mutex,300);
    }

    master.close();
    emit sigLinkStatus(false,"采集停止，串口关闭");
    qDebug()<<"LinkWorker采集任务退出";
}
