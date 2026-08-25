#ifndef SQLITEDB_H
#define SQLITEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

class SqliteDb : public QObject
{
    Q_OBJECT
public:
    explicit SqliteDb(QObject *parent = nullptr);
    ~SqliteDb();

    // 打开数据库，传入数据库文件路径
    bool openDb(const QString& dbFilePath);
    void closeDb();
    bool isOpen() const;
    QSqlDatabase database() const { return m_db; }
    // 执行建表，open成功后调用
    bool createTables();

private:
    QSqlDatabase m_db;
    bool m_bOpen = false;
};

#endif // SQLITEDB_H
