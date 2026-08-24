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

    //1.按串口参数分组，同端口设备归为一组
    QMap<QString, QVector<DeviceEntity>> portGroup;
    for(const auto& dev : devList)
    {
        portGroup[dev.portParam].append(dev);
    }

    //2.每组创建1个worker+QThread
    for(auto iter = portGroup.cbegin(); iter != portGroup.cend(); ++iter)
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
        m_workerList.append(QPointer<LinkWorker>(worker));
    }
}

void LinkManager::startAllLinks()
{
    for(auto wp : m_workerList)
    {
        if(!wp) continue;
        wp->startWork();
        QThread* th = wp->thread();
        if(th && !th->isRunning())
        {
            th->start(); //真正启动操作系统线程，拉起事件循环exec()
        }
    }
}

void LinkManager::stopAllLinks()
{

    for(auto wp : m_workerList)
    {
        if(wp) wp->stopWork();
    }

    for(auto wp : m_workerList)
    {
        if(!wp) continue;
        QThread* th = wp->thread();
        if(th)
        {   th->quit();
            th->wait(1500);
        }
        delete wp;
        delete th;
    }
    m_workerList.clear();
}
