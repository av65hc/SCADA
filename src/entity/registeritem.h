#ifndef REGISTERITEM_H
#define REGISTERITEM_H

#include <QString>
#include <QMetaType>

struct RegisterItem{
    QString name;
    int addr = 0;
    int len =1;
    double lowAlarm  = 0;
    double highAlarm = 0;
};

Q_DECLARE_METATYPE(RegisterItem);

#endif