#include "mainwindow.h"
#include "entity/alarmitem.h"
#include "entity/deviceentity.h"
#include "entity/registeritem.h"
#include "database/sqlitedb.h"
#include "database/dbworker.h"
#include "comm/linkworker.h"
#include "utils/loghelper.h"


#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    LogHelper::install("./scada.log");

    QApplication a(argc, argv);

    // 注册自定义元类型，可跨线程使用
    qRegisterMetaType<AlarmItem>("AlarmItem");
    qRegisterMetaType<DeviceEntity>("DeviceEntity");
    qRegisterMetaType<RegisterItem>("RegisterItem");
    qRegisterMetaType<DbTask>("DbTask");
    qRegisterMetaType<HistoryDataPayload>("HistoryDataPayload");
    qRegisterMetaType<AlarmPayload>("AlarmPayload");
    qRegisterMetaType<CollectDataItem>("CollectDataItem");

    DbWorkerThread dbThread("./scada.db");
    dbThread.start();

    MainWindow w;
    w.show();

    int ret = a.exec();

    return ret;
}
