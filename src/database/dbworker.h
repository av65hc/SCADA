#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include <QThread>
#include <QVariant>
#include "../utils/threadqueue.h"
#include  "sqlitedb.h"

enum DbtaskType{
    insert_history,
    insert_alarm,
    query_alarm,
    query_history
};

//历史数据任务载荷
struct HistoryDataPayload
{
    QString devUuid;
    QString regName;
    double value;
    QDateTime collectTime;
};

//报警任务载荷
struct AlarmPayload
{
    QString devUuid;
    QString devName;
    QString regName;
    double value;
    double threshold;
    QString alarmType;
    QDateTime occurTime;
};

//真正的数据库操作（增删改查）
struct DbTask{
    DbtaskType type;
    QVariant data;
};

Q_DECLARE_METATYPE(HistoryDataPayload)
Q_DECLARE_METATYPE(AlarmPayload)


class DbWorker : public QObject
{
    Q_OBJECT
public:
    explicit DbWorker(const QString &dbPath);
    ~DbWorker();

    void pushTask(const DbTask &task);
    void stop();
    void run();

private:
    SqliteDb m_db;
    ThreadQueue<DbTask> m_taskqueue;
    QString m_dbPath;
};

class DbWorkerThread : public QThread{
    Q_OBJECT
public:
    explicit DbWorkerThread(const QString &dbPath);
    ~DbWorkerThread();
    void pushTask(const DbTask &t);
protected:
    void run() override;
private:
    DbWorker *m_worker = nullptr;

};

#endif
