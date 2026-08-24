#include "mainwindow.h"
#include "entity/alarmitem.h"
#include "entity/deviceentity.h"
#include "entity/registeritem.h"
#include "database/sqlitedb.h"
#include "database/dbworker.h"

#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 注册自定义元类型，可跨线程使用
    qRegisterMetaType<AlarmItem>("AlarmItem");
    qRegisterMetaType<DeviceEntity>("DeviceEntity");
    qRegisterMetaType<RegisterItem>("RegisterItem");
    qRegisterMetaType<DbTask>("DbTask");
    qRegisterMetaType<HistoryDataPayload>("HistoryDataPayload");
    qRegisterMetaType<AlarmPayload>("AlarmPayload");

    SqliteDb db;
    if(!db.openDb("./scada.db")){
        qCritical()<<"数据库打开失败";
        return -1;
    }

    DbworkerThread dbThread(&db);
    dbThread.start();

    MainWindow w;
    w.show();

    int ret = a.exec();

    dbThread.quit();
    dbThread.wait();

    return ret;
}
