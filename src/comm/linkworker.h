#ifndef LINKWORKER_H
#define LINKWORKER_H

#include <QObject>
#include <QThread>
#include <QDateTime>
#include "../entity/deviceentity.h"
#include "../utils/threadqueue.h"

//单条通信链路采集数据，上报给UI的结构体
struct CollectDataItem
{
    QString devUuid;
    QString regName;
    double value;
    QDateTime time;
};
Q_DECLARE_METATYPE(CollectDataItem)

class LinkWorker : public QObject
{
    Q_OBJECT
public:
    explicit LinkWorker(QObject *parent = nullptr);
    ~LinkWorker();

    //设置本链路下所有从站设备
    void setDeviceList(const QVector<DeviceEntity>& devList);
    void startWork();
    void stopWork();

signals:
    //采集到数据，通知UI主线程
    void sigCollectData(const CollectDataItem& item);
    //链路状态变化：在线/离线
    void sigLinkStatus(bool online, const QString& info);

public slots:
    void run();

private:
    QVector<DeviceEntity> m_devList;
    bool m_running = false;
};

#endif // LINKWORKER_H
