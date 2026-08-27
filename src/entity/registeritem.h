#ifndef REGISTERITEM_H
#define REGISTERITEM_H

#include <QString>
#include <QMetaType>

enum class DataType{
    Int16 = 0,
    UInt16,
    Int32,
    UInt32,
    Float32
};

struct RegisterItem{
    QString name;
    int addr = 0;
    int len =1;
    DataType dataType = DataType::Int16;
    double lowAlarm  = 0;
    double highAlarm = 0;
};

Q_DECLARE_METATYPE(RegisterItem);

#endif