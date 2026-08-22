#include "mainwindow.h"
#include "entity/alarmitem.h"
#include "entity/deviceentity.h"
#include "entity/registeritem.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qRegisterMetaType<AlarmItem>("AlarmItem");
    qRegisterMetaType<DeviceEntity>("DeviceEntity");
    qRegisterMetaType<RegisterItem>("RegisterItem");
    MainWindow w;
    w.show();
    return QApplication::exec();
}
