#ifndef ALARMITEM_H
#define ALARMITEM_H

#include <QString>
#include <QDateTime>
#include <QMetaType>

struct AlarmItem{
    qint64 id = 0;
    QString devUuid;
    QString regName;
    QString devName;
    double value = 0;
    double threshold = 0;
    QString alarmType;
    QDateTime occurTime;
    bool isConfirm = false;
};

Q_DECLARE_METATYPE(AlarmItem);

#endif
