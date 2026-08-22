#ifndef DEVICEENTITY_H
#define DEVICEENTITY_H

#include <QString>
#include <QVector>
#include "registeritem.h"
#include <QMetaType>

struct DeviceEntity{
    QString devUuid;
    QString devName;
    int slaveId = 1;
    QString commType;
    QString portParam;
    bool enable = false;
    QVector<RegisterItem> regList;
};

Q_DECLARE_METATYPE(DeviceEntity);

#endif
