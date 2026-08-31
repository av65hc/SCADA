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

    // 按通信类型选择 TCP / RTU，不能一律 openSerial
    bool openOk = isTcp ? master.openTcp(portParam) : master.openSerial(portParam);
    // 首次连接失败不退出，循环重试，直到成功或停止（否则设备后插上也无法恢复）
    while(!openOk && m_running){
        master.close();   // 清掉失败连接对象
        emit sigLinkStatus(false, "链接失败，3秒后重试:" + portParam);
        QMutexLocker locker(&m_mutex);
        m_waitCond.wait(&m_mutex, 3000);
        if(!m_running) return;
        openOk = isTcp ? master.openTcp(portParam) : master.openSerial(portParam);
    }
    if(!m_running) return;
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
                checkAlarm(dev,reg,val);
            }
        }
        if(!anyOk){
            if(++failRound >= 3){
                // 连续 3 轮全部读取失败 → 关闭旧连接后按协议重连
                master.close();
                bool reOk = isTcp ? master.openTcp(portParam) : master.openSerial(portParam);
                if(reOk){
                    emit sigLinkStatus(true, "重连成功:" + portParam);
                    failRound = 0;
                } else {
                    emit sigLinkStatus(false, "重连失败:" + portParam);
                    // failRound 不重置，下一轮继续尝试重连
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

void LinkWorker::checkAlarm(const DeviceEntity& dev, const RegisterItem& reg, double value)
{
    // 未配置报警阈值（两个都是 0）→ 该点不参与报警
    if (reg.lowAlarm == 0.0 && reg.highAlarm == 0.0)
        return;

    QString key = dev.devUuid + "|" + reg.name;
    int oldState = m_alarmState.value(key, 0);   // 0 正常 / 1 高报警 / 2 低报警

    int newState = 0;
    QString alarmType;
    double threshold = 0.0;

    if (value > reg.highAlarm) {
        newState = 1; alarmType = "high"; threshold = reg.highAlarm;
    } else if (value < reg.lowAlarm) {
        newState = 2; alarmType = "low";  threshold = reg.lowAlarm;
    }

    // 状态没变（持续报警 / 持续正常）→ 不重复记录
    if (newState == oldState)
        return;

    m_alarmState[key] = newState;

    AlarmItem alarm;
    alarm.devUuid  = dev.devUuid;
    alarm.devName  = dev.devName;
    alarm.regName  = reg.name;
    alarm.value    = value;
    alarm.occurTime = QDateTime::currentDateTime();

    if (newState == 0) {
        // 从报警恢复到正常
        alarm.alarmType = "recover";
        alarm.threshold = (oldState == 1) ? reg.highAlarm : reg.lowAlarm;
    } else {
        alarm.alarmType = alarmType;
        alarm.threshold = threshold;
    }

    emit sigAlarm(alarm);
}
