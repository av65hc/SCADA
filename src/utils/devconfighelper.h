#ifndef DEVCONFIGHELPER_H
#define DEVCONFIGHELPER_H

#include <QVector>
#include "../entity/deviceentity.h"

namespace DevConfigHelper{
    bool saveDevices(const QVector<DeviceEntity>& devlist,const QString &filepath);
    QVector<DeviceEntity> loadDevices(const QString &filepath);
}

#endif