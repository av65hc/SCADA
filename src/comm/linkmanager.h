#ifndef LINKMANAGER_H
#define LINKMANAGER_H

#include <QObject>
#include <QVector>
#include "linkworker.h"
#include "../entity/deviceentity.h"

//链路管理器：管理全部LinkWorker实例
class LinkManager : public QObject
{
    Q_OBJECT
public:
    explicit LinkManager(QObject *parent = nullptr);
    ~LinkManager();

    //加载设备配置，根据配置创建多条链路worker
    void loadFromDeviceList(const QVector<DeviceEntity>& devList);

    void startAllLinks();
    void stopAllLinks();

signals:
    //转发采集数据信号给到UI
    void sigCollectData(const CollectDataItem& item);

private:
    QVector<LinkWorker*> m_workerList;
};

#endif // LINKMANAGER_H
