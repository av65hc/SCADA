#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QString>

namespace LogHelper {
//安装全局日志：qDebug/qInfo/qWarning/qCritical
void install(const QString& filePath);
}

#endif