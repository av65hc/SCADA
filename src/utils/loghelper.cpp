#include "loghelper.h"

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QDebug>

namespace {
QFile g_logFile;
QMutex g_mutex;
void messageHandler(QtMsgType type, const QMessageLogContext&ctx, const QString& msg){
    Q_UNUSED(ctx);
    QString level;
    switch (type){
    case QtDebugMsg: level = "DEBUG"; break;
    case QtInfoMsg: level = "INFO"; break;
    case QtWarningMsg: level = "WARN";break;
    case QtCriticalMsg: level = "ERROR";break;
    case QtFatalMsg: level = "FATAL";break;
    }
    QString line = QString("[%1][%2] %3").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")).arg(level).arg(msg);
    fprintf(stderr,"%s\n", qPrintable(line));
    QMutexLocker locker(&g_mutex);
    if(g_logFile.isOpen()){
        QTextStream stream(&g_logFile);
        stream<<line<<"\n";
        stream.flush();
    }
}
}

namespace LogHelper{
void install(const QString& filePath){
    g_logFile.setFileName(filePath);
    g_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    qInstallMessageHandler(messageHandler);
}
}