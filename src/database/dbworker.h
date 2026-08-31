#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include <QThread>
#include <QVariant>
#include <QVector>
#include "../utils/threadqueue.h"
#include  "sqlitedb.h"

enum DbtaskType{
    insert_history,
    insert_alarm,
    query_alarm,
    query_history,
    confirm_alarm     // 报警确认
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

// 历史查询参数
struct HistoryQueryParam{
    QString devUuid;
    QString regName;
    QDateTime start;
    QDateTime end;
};
Q_DECLARE_METATYPE(HistoryQueryParam)

// 历史查询结果的一行
struct HistoryRow{
    double timeMs = 0.0;   // epoch 毫秒，曲线用
    double value  = 0.0;
};
Q_DECLARE_METATYPE(HistoryRow)

// 历史查询结果
struct HistoryResult{
    QString devUuid;
    QString regName;
    QVector<HistoryRow> rows;
};
Q_DECLARE_METATYPE(HistoryResult)

// 报警确认参数
struct AlarmConfirmParam{
    QString devUuid;
    QString regName;
    QDateTime occurTime;
};
Q_DECLARE_METATYPE(AlarmConfirmParam)

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

signals:
    void sigHistoryResult(const HistoryResult& result);   // 查询结果返回

private:
    SqliteDb m_db;
    ThreadQueue<DbTask> m_taskqueue;
    QString m_dbPath;
    std::atomic<bool> m_running{true}; //增加运行标记
};

class DbWorkerThread : public QThread{
    Q_OBJECT
public:
    explicit DbWorkerThread(const QString &dbPath);
    ~DbWorkerThread();
    void pushTask(const DbTask &t);
signals:
    void sigHistoryResult(const HistoryResult& result);   // 转发给主线程
protected:
    void run() override;
private:
    DbWorker *m_worker = nullptr;

};

#endif
