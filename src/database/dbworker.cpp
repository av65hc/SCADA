#include "dbworker.h"
#include <QDebug>

DbWorker::DbWorker(const QString &dbPath): m_dbPath(dbPath)
{
}

DbWorker::~DbWorker()
{

}

void DbWorker::pushTask(const DbTask &task)
{
    m_taskqueue.enqueue(task);
}

void DbWorker::run()
{
    if(!m_db.openDb(m_dbPath)){
        qCritical()<<"DbWorker线程打开数据库失败："<<m_dbPath;
        return;
    }
    qDebug()<<"DbWorker线程启动，数据库就绪";
    DbTask task;
    while(m_taskqueue.dequeue(task))
    {
        QSqlQuery q(m_db.database());

        switch (task.type) {
        case insert_history:
        {
            HistoryDataPayload payload = task.data.value<HistoryDataPayload>();
            q.prepare(R"(INSERT INTO t_history(devUuid,regName,value,collectTime) VALUES(?,?,?,?))");
            q.addBindValue(payload.devUuid);
            q.addBindValue(payload.regName);
            q.addBindValue(payload.value);
            q.addBindValue(payload.collectTime);
            if(!q.exec())
            {
                qWarning()<<"插入历史数据失败:"<<q.lastError().text();
            }
            break;
        }
        case insert_alarm:
        {
            AlarmPayload payload = task.data.value<AlarmPayload>();
            q.prepare(R"(INSERT INTO t_alarm(devUuid,devName,regName,value,threshold,alarmType,occurTime,isConfirm)
                         VALUES(?,?,?,?,?,?,?,0))");
            q.addBindValue(payload.devUuid);
            q.addBindValue(payload.devName);
            q.addBindValue(payload.regName);
            q.addBindValue(payload.value);
            q.addBindValue(payload.threshold);
            q.addBindValue(payload.alarmType);
            q.addBindValue(payload.occurTime);
            if(!q.exec())
            {
                qWarning()<<"插入报警失败:"<<q.lastError().text();
            }
            break;
        }
        case query_alarm:
            //后续UI报警页面再实现
            break;
        case query_history:
            //后续历史查询页面实现
            break;
        default:
            break;
        }
    }
    qDebug()<<"DbWorker线程退出";
}


DbWorkerThread::DbWorkerThread(const QString &dbPath)
{
    m_worker = new DbWorker(dbPath);
}

DbWorkerThread::~DbWorkerThread()
{
    if(isRunning())
    {
        if (m_worker) m_worker->stop();   // ① 唤醒任务循环，让它退出
        wait();                           // ② 等 run() 真正返回（线程结束）
        delete m_worker;                  // ③ 线程结束后才安全删除
        m_worker = nullptr;
    }
    delete m_worker;
}

void DbWorkerThread::pushTask(const DbTask &t)
{
    if(m_worker)
    {
        m_worker->pushTask(t);
    }
}

void DbWorkerThread::run()
{
    m_worker->run();
}
