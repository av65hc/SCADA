#include "linkmanager.h"
#include <QThread>
#include <QDebug>
#include <QMap>

LinkManager::LinkManager(QObject *parent)
    : QObject{parent}
{
}

LinkManager::~LinkManager()
{
    stopAllLinks();
}

void LinkManager::loadFromDeviceList(const QVector<DeviceEntity> &devList)
{
    stopAllLinks();

    //1.commType+portParam分组
    QMap<QString, QVector<DeviceEntity>> linkGroup;
    for(const auto& dev : devList)
    {   if (!dev.enable || dev.portParam.isEmpty())
            continue;
        QString key = dev.commType + "|" + dev.portParam;
        linkGroup[key].append(dev);
    }

    //2.每组创建1个worker+QThread
    for(auto iter = linkGroup.cbegin(); iter != linkGroup.cend(); ++iter)//cbegin,csend返回只读容器
    {
        QThread* th = new QThread;
        LinkWorker* worker = new LinkWorker;
        worker->setDeviceList(iter.value());

        worker->moveToThread(th);

        //线程started → 启动采集业务函数
        connect(th, &QThread::started, worker, &LinkWorker::slotTaskLoop);

        //转发采集数据信号
        connect(worker, &LinkWorker::sigCollectData,
                this, &LinkManager::sigCollectData);
        connect(worker, &LinkWorker::sigAlarm, this, &LinkManager::sigAlarm);
        connect(worker, &LinkWorker::sigLinkStatus, this, &LinkManager::sigLinkStatus);

        LinkManager::WorkerInfo info;
        info.worker = worker;
        info.thread = th;
        m_workerList.append(info);
    }
}

void LinkManager::startAllLinks()
{
    for(auto& wp : m_workerList)
    {
        if(!wp.worker || !wp.thread) continue;
        wp.worker->startWork();      // 先置运行标记 m_running=true，再启动线程
        if(!wp.thread->isRunning())
        {
            wp.thread->start();      // 启动后触发 started → slotTaskLoop
        }
    }
}

void LinkManager::stopAllLinks()
{

    for(auto wp : m_workerList)
    {
        if(wp.worker) wp.worker->stopWork();
    }

    for(auto wp : m_workerList)
    {
        if (wp.thread) {
            wp.thread->quit();   // 关键：让 exec() 返回（Day 1 的教训）
            wp.thread->wait();   // 现在 wait 才真正生效
        }
        delete wp.worker;        // 线程结束后安全删除
        delete wp.thread;
    }
    m_workerList.clear();
}
