#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "../entity/deviceentity.h"
#include "../comm/linkmanager.h"
#include "../database/dbworker.h"
#include "../thirdparty/qcustomplot.h"

class QListWidget;
class QLabel;
class QAction;
class QTableWidget;
class QTabWidget;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddDevice();
    void onEditDevice();
    void onDeleteDevice();
    void onToggleLink();
    void onCollectData(const CollectDataItem& item);
    void onAlarm(const AlarmItem& alarm);
    void onLinkStatus(bool online, const QString& info);
    void onCurveSelect(const QString& key);

private:
    void setupUi();
    void setupActions();
    void loadDevices();
    void saveDevices();
    void rebuildLinks();
    void refreshDeviceList();
    void updateRealTimeTable(const CollectDataItem& item);
    void addAlarmToTable(const AlarmItem& alarm);
    void appendCurveData(const QString& key, double t, double v);

    QVector<DeviceEntity> m_devices;
    bool m_linkRunning = false;

    // m_dbThread 在前，析构时 m_linkMgr 先停（逆序析构）
    DbWorkerThread m_dbThread;
    LinkManager    m_linkMgr;
    QHash<QString, int>     m_rowIndex;      // "devUuid|regName" → 实时表格行号
    QHash<QString, QString> m_uuidToName;    // devUuid → devName
    QHash<QString, QVector<double>> m_curveTimes;   // 每个点的曲线时间
    QHash<QString, QVector<double>> m_curveValues;  // 每个点的曲线值
    void updateCurveNow(const CollectDataItem& item);

    QListWidget* m_listDevices = nullptr;
    QAction*     m_actToggle = nullptr;
    QLabel*      m_statusLabel = nullptr;
    QTabWidget*   m_tabWidget = nullptr;
    QTableWidget* m_tableRealTime = nullptr;
    QTableWidget* m_tableAlarm = nullptr;
    QComboBox*    m_cmbCurve = nullptr;
    QCustomPlot*  m_plot = nullptr;
};

#endif // MAINWINDOW_H
