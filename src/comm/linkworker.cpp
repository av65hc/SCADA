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

void LinkWorker::startWork()
{
    m_running = true;
}

void LinkWorker::stopWork()
{
    m_running = false;
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
    const QString commType = m_devList.first().commType;
    const QString portParam = m_devList.first().portParam;
    const bool isTcp = (commType.compare("tcp", Qt::CaseInsensitive) == 0);

    bool openOk = master.openSerial(portParam);
    if(!openOk)
    {
        emit sigLinkStatus(false, "链接失败:" + portParam);
        return;
    }
    emit sigLinkStatus(true, "链接成功:"+portParam);
    int failRound = 0;
    while(m_running)
    {
        bool anyOk = false;

        for(const auto& dev : m_devList)
        {
            if(!m_running) break;
            if(!dev.enable) continue;

            for(const auto& reg : dev.regList)
            {
                if(!m_running) break;

                QVector<uint16_t> raw;
                int ret = master.readHoldReg(dev.slaveId, reg.addr, reg.len, raw);
                if(ret != 0)
                {
                    continue;
                }
                anyOk = true;
                double val = ModbusMaster::decodeValue(raw,reg.dataType);
                CollectDataItem item;
                item.devUuid = dev.devUuid;
                item.regName = reg.name;
                item.value = val;
                item.time = QDateTime::currentDateTime();
                emit sigCollectData(item);
            }
        }
        if(!anyOk){
            if(++failRound >= 3){
                bool openError = master.openSerial(portParam);
                emit sigLinkStatus(false,QString("链路中断，尝试重连：%1").arg(openError));
                master.close();
                openOk = isTcp ? master.openTcp(portParam) : openError;
                if(openOk){
                    emit sigLinkStatus(true,"重连成功"+portParam);
                    failRound = 0;
                }
            }
        }
        else{
            failRound = 0;
        }
        QMutexLocker locker(&m_mutex);
        m_waitCond.wait(&m_mutex,300);
    }

    master.close();
    emit sigLinkStatus(false,"采集停止，串口关闭");
    qDebug()<<"LinkWorker采集任务退出";
}
