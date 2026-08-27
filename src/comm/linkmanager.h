#ifndef LINKMANAGER_H
#define LINKMANAGER_H

#include <QObject>
#include <QVector>
#include <QPointer>
#include "linkworker.h"
#include "../entity/deviceentity.h"

class LinkManager : public QObject
{
    Q_OBJECT
public:
    explicit LinkManager(QObject *parent = nullptr);
    ~LinkManager() override;

    void loadFromDeviceList(const QVector<DeviceEntity>& devList);
    void startAllLinks();
    void stopAllLinks();

signals:
    void sigCollectData(const CollectDataItem& item);

private:
    struct WorkerInfo {
        QPointer<LinkWorker> worker;   // QPointer：对象销毁自动置 null，防野指针
        QPointer<QThread>   thread;
    };
    QVector<WorkerInfo> m_workerList;
};

#endif // LINKMANAGER_H
