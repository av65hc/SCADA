#include "mainwindow.h"
#include "entity/alarmitem.h"
#include "entity/deviceentity.h"
#include "entity/registeritem.h"
#include "database/dbworker.h"
#include "comm/linkworker.h"
#include "utils/loghelper.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    LogHelper::install("./scada.log");

    QApplication a(argc, argv);

    qRegisterMetaType<AlarmItem>("AlarmItem");
    qRegisterMetaType<DeviceEntity>("DeviceEntity");
    qRegisterMetaType<RegisterItem>("RegisterItem");
    qRegisterMetaType<DbTask>("DbTask");
    qRegisterMetaType<HistoryDataPayload>("HistoryDataPayload");
    qRegisterMetaType<AlarmPayload>("AlarmPayload");
    qRegisterMetaType<CollectDataItem>("CollectDataItem");
    qRegisterMetaType<HistoryQueryParam>("HistoryQueryParam");
    qRegisterMetaType<HistoryResult>("HistoryResult");
    qRegisterMetaType<AlarmConfirmParam>("AlarmConfirmParam");

    MainWindow w;
    w.show();

    return a.exec();
}
