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
    //QPointer：对象销毁自动置nullptr，解决裸指针野指针
    QVector<QPointer<LinkWorker>> m_workerList;
};

#endif // LINKMANAGER_H
