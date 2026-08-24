#include "dbworker.h"
#include <QDebug>

DbWorker::DbWorker(SqliteDb *db)
    : m_db(db)
{
}

DbWorker::~DbWorker()
{

}

void DbWorker::pushTask(const DbTask &task)
{
    m_taskQueue.enqueue(task);
}

void DbWorker::run()
{
    m_running = true;
    qDebug()<<"DbWorker线程启动";
    while(m_running)
    {
        auto task = m_taskQueue.dequeue();
        if(!m_running) break;

        QSqlQuery q(m_db->m_db); //数据库连接只在本线程使用

        switch (task.type) {
        case TASK_INSERT_HISTORY:
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
        case TASK_INSERT_ALARM:
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
        case TASK_QUERY_ALARM:
            //后续UI报警页面再实现
            break;
        case TASK_QUERY_HISTORY:
            //后续历史查询页面实现
            break;
        default:
            break;
        }
    }
    qDebug()<<"DbWorker线程退出";
}


DbWorkerThread::DbWorkerThread(SqliteDb *db)
{
    m_worker = new DbWorker(db);
}

DbWorkerThread::~DbWorkerThread()
{
    if(isRunning())
    {
        m_worker->pushTask({});
        quit();
        wait();
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
