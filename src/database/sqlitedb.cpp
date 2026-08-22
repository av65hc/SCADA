#include "sqlitedb.h"
#include <QDebug>

SqliteDb::SqliteDb(QObject *parent)
    : QObject(parent)
{

}

SqliteDb::~SqliteDb()
{
    closeDb();
}

bool SqliteDb::openDb(const QString &dbFilePath)
{
    if(m_bOpen)
    {
        return true;
    }

    // 给连接起名字，避免多连接冲突
    m_db = QSqlDatabase::addDatabase("QSQLITE","scada_db_conn");
    m_db.setDatabaseName(dbFilePath);

    if(!m_db.open())
    {
        qCritical()<<"数据库打开失败："<< m_db.lastError().text();
        m_bOpen = false;
        return false;
    }
    m_bOpen = true;

    // 打开之后立刻建表
    if(!createTables())
    {
        qCritical()<<"数据表创建失败";
    }
    return true;
}

void SqliteDb::closeDb()
{
    if(m_bOpen)
    {
        m_db.close();
        QSqlDatabase::removeDatabase("scada_db_conn");
        m_bOpen = false;
    }
}

bool SqliteDb::isOpen() const
{
    return m_bOpen;
}

bool SqliteDb::createTables()
{
    if(!m_bOpen) return false;

    QSqlQuery query(m_db);

    // 1.历史数据表 t_history
    QString sqlHistory = R"(
CREATE TABLE IF NOT EXISTS t_history(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    devUuid TEXT,
    regName TEXT,
    value REAL,
    collectTime DATETIME
);
)";
    if(!query.exec(sqlHistory))
    {
        qCritical()<<"建t_history失败:"<<query.lastError().text();
        return false;
    }

    //2.报警表 t_alarm
    QString sqlAlarm = R"(
CREATE TABLE IF NOT EXISTS t_alarm(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    devUuid TEXT,
    devName TEXT,
    regName TEXT,
    value REAL,
    threshold REAL,
    alarmType TEXT,
    occurTime DATETIME,
    isConfirm INTEGER DEFAULT 0
);
)";
    if(!query.exec(sqlAlarm))
    {
        qCritical()<<"建t_alarm失败:"<<query.lastError().text();
        return false;
    }

    //3.日志表 t_log
    QString sqlLog = R"(
CREATE TABLE IF NOT EXISTS t_log(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    logTime DATETIME,
    content TEXT
);
)";
    if(!query.exec(sqlLog))
    {
        qCritical()<<"建t_log失败:"<<query.lastError().text();
        return false;
    }

    qDebug()<<"三张数据表初始化完成";
    return true;
}
