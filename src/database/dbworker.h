#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include <QThread>
#include <qvariant.h>
#include "../utils/threadqueue.h"
#include  "sqlitedb.h"

enum DbtaskType{
    insert_history,
    insert_alarm,
    query_alarm,
    query_history
};

struct Dbtask{
    DbtaskType type;
    QVariant data;
};

class DbWorker : public QObject
{
    Q_OBJECT
public:
    explicit DbWorker(SqliteDb *db);
    ~DbWorker();

    void pushTask(const Dbtask &task);

public slots:
    void run();

private:
    SqliteDb *m_db = nullptr;
    ThreadQueue<Dbtask> m_taskqueue;
    bool m_running = false;
};

class DbworkerThread : public QThread{
    Q_OBJECT
public:
    explicit DbworkerThread(SqliteDb *db);
    ~DbworkerThread();
    void pushTask(const Dbtask &t);
protected:
    void run() override;
private:
    DbWorker *m_worker = nullptr;

};

#endif
