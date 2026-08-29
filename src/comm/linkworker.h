#ifndef LINKWORKER_H
#define LINKWORKER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <QDateTime>
#include "../entity/deviceentity.h"
#include "../entity/alarmitem.h"

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
    ~LinkWorker() override;

    // 同一串口下的全部设备
    void setDeviceList(const QVector<DeviceEntity>& devList);

    void startWork();
    void stopWork();

signals:
    void sigCollectData(const CollectDataItem& item);
    void sigLinkStatus(bool online, const QString& info);
    void sigAlarm(const AlarmItem& alarm);

public slots:
    void slotTaskLoop();

private:
    QVector<DeviceEntity> m_devList;
    std::atomic<bool> m_running{false};
    void checkAlarm(const DeviceEntity& dev, const RegisterItem& reg, double value);
    QHash<QString, int> m_alarmState;
    QMutex m_mutex;
    QWaitCondition m_waitCond;
};

#endif // LINKWORKER_H
