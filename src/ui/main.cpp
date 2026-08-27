#include "mainwindow.h"
#include "entity/alarmitem.h"
#include "entity/deviceentity.h"
#include "entity/registeritem.h"
#include "database/sqlitedb.h"
#include "database/dbworker.h"
#include "comm/linkworker.h"
#include "utils/loghelper.h"
#include "comm/linkmanager.h"


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

    LinkManager mgr;   // 栈对象，活到 main 结束
    QObject::connect(&mgr, &LinkManager::sigCollectData,
                     [](const CollectDataItem& item) {
                         qDebug() << "采集到:" << item.devUuid
                                  << item.regName << item.value
                                  << item.time.toString("HH:mm:ss.zzz");
                     });

    DeviceEntity dev;
    dev.devUuid   = "dev-1";
    dev.devName   = "测试设备1";
    dev.slaveId   = 1;
    dev.commType  = "tcp";
    dev.portParam = "127.0.0.1:502";
    dev.enable    = true;

    RegisterItem r1; r1.name = "温度"; r1.addr = 0; r1.len = 1;
    r1.dataType = DataType::Int16; r1.highAlarm = 100;
    RegisterItem r2; r2.name = "压力"; r2.addr = 1; r2.len = 1;
    r2.dataType = DataType::Int16;
    dev.regList = { r1, r2 };

    mgr.loadFromDeviceList({ dev });
    mgr.startAllLinks();

    MainWindow w;
    w.show();

    int ret = a.exec();

    return ret;
}
