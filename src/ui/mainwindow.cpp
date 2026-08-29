#include "mainwindow.h"
#include "../dialog/deviceconfigdialog.h"
#include "../utils/devconfighelper.h"
#include <QToolBar>
#include <QListWidget>
#include <QStatusBar>
#include <QLabel>
#include <QMessageBox>
#include <QSplitter>
#include <QAction>
#include <QUuid>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_dbThread("./scada.db")      // 数据库线程先初始化
{
    m_dbThread.start();
    setupUi();
    setupActions();

    // 数据入库（Day 4 的接线，从 main.cpp 搬到这里）
    connect(&m_linkMgr, &LinkManager::sigCollectData, this, &MainWindow::onCollectData);
    connect(&m_linkMgr, &LinkManager::sigAlarm,       this, &MainWindow::onAlarm);
    connect(&m_linkMgr, &LinkManager::sigLinkStatus,  this, &MainWindow::onLinkStatus);

    loadDevices();   // 启动时加载配置并重建链路
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    setWindowTitle("Modbus SCADA 监控系统");
    resize(1000, 640);

    m_listDevices = new QListWidget;

    m_tabWidget = new QTabWidget;

    // Tab1: 实时数据
    m_tableRealTime = new QTableWidget(0, 4);
    m_tableRealTime->setHorizontalHeaderLabels({"设备", "寄存器", "值", "时间"});
    m_tableRealTime->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableRealTime->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableRealTime->setEditTriggers(QAbstractItemView::NoEditTriggers);   // 只读
    m_tabWidget->addTab(m_tableRealTime, "实时数据");

    // Tab2: 报警
    m_tableAlarm = new QTableWidget(0, 6);
    m_tableAlarm->setHorizontalHeaderLabels({"时间", "设备", "寄存器", "类型", "值", "阈值"});
    m_tableAlarm->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_tableAlarm->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableAlarm->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tabWidget->addTab(m_tableAlarm, "报警记录");

    // Tab3: 实时曲线
    m_plot = new QCustomPlot;
    m_plot->addGraph();
    m_plot->graph(0)->setPen(QPen(QColor(40, 120, 220), 2));
    m_plot->xAxis->setLabel("时间");
    m_plot->yAxis->setLabel("值");
    // x 轴用时间格式（HH:mm:ss）
    QSharedPointer<QCPAxisTickerDateTime> ticker(new QCPAxisTickerDateTime);
    ticker->setDateTimeFormat("HH:mm:ss");
    m_plot->xAxis->setTicker(ticker);

    m_cmbCurve = new QComboBox;
    auto* curveWidget = new QWidget;
    auto* curveLayout = new QVBoxLayout(curveWidget);
    curveLayout->addWidget(m_cmbCurve);
    curveLayout->addWidget(m_plot);
    m_tabWidget->addTab(curveWidget, "实时曲线");

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(m_listDevices);
    splitter->addWidget(m_tabWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 3);
    setCentralWidget(splitter);

    m_statusLabel = new QLabel("就绪");
    statusBar()->addWidget(m_statusLabel);

    connect(m_cmbCurve, &QComboBox::currentTextChanged, this, &MainWindow::onCurveSelect);
}


void MainWindow::setupActions()
{
    auto* tb = addToolBar("main");
    tb->setMovable(false);

    QAction* actAdd  = tb->addAction("添加设备");
    QAction* actEdit = tb->addAction("编辑设备");
    QAction* actDel  = tb->addAction("删除设备");
    tb->addSeparator();
    m_actToggle = tb->addAction("启动采集");

    connect(actAdd,  &QAction::triggered, this, &MainWindow::onAddDevice);
    connect(actEdit, &QAction::triggered, this, &MainWindow::onEditDevice);
    connect(actDel,  &QAction::triggered, this, &MainWindow::onDeleteDevice);
    connect(m_actToggle, &QAction::triggered, this, &MainWindow::onToggleLink);
}

void MainWindow::loadDevices()
{
    m_devices = DevConfigHelper::loadDevices("./devices.json");
    refreshDeviceList();
    rebuildLinks();
}

void MainWindow::saveDevices()
{
    DevConfigHelper::saveDevices(m_devices, "./devices.json");
}

void MainWindow::rebuildLinks()
{
    m_linkMgr.loadFromDeviceList(m_devices);   // 停旧建新
    if (m_linkRunning)
        m_linkMgr.startAllLinks();
}

void MainWindow::refreshDeviceList()
{
    m_uuidToName.clear();
    m_listDevices->clear();
    for (const auto& dev : m_devices) {
        m_uuidToName[dev.devUuid] = dev.devName;
        QString text = QString("%1  [%2] %3")
        .arg(dev.devName)
            .arg(dev.commType.toUpper())
            .arg(dev.portParam);
        m_listDevices->addItem(text);
    }
}

void MainWindow::onAddDevice()
{
    DeviceEntity empty;
    DeviceConfigDialog dlg(empty, this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    DeviceEntity dev = dlg.getDevice();
    if (dev.devName.isEmpty()) {
        QMessageBox::warning(this, "提示", "设备名称不能为空");
        return;
    }
    dev.devUuid = QUuid::createUuid().toString();   // 新设备生成唯一 ID
    m_devices.append(dev);
    saveDevices();
    refreshDeviceList();
    rebuildLinks();
}

void MainWindow::onEditDevice()
{
    int row = m_listDevices->currentRow();
    if (row < 0 || row >= m_devices.size()) {
        QMessageBox::information(this, "提示", "请先选中要编辑的设备");
        return;
    }
    DeviceConfigDialog dlg(m_devices[row], this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    DeviceEntity dev = dlg.getDevice();
    dev.devUuid = m_devices[row].devUuid;   // 保留原 ID
    m_devices[row] = dev;
    saveDevices();
    refreshDeviceList();
    rebuildLinks();
}

void MainWindow::onDeleteDevice()
{
    int row = m_listDevices->currentRow();
    if (row < 0 || row >= m_devices.size()) {
        QMessageBox::information(this, "提示", "请先选中要删除的设备");
        return;
    }
    if (QMessageBox::question(this, "确认", "确定删除该设备？") != QMessageBox::Yes)
        return;
    m_devices.removeAt(row);
    saveDevices();
    refreshDeviceList();
    rebuildLinks();
}

void MainWindow::onToggleLink()
{
    m_linkRunning = !m_linkRunning;
    if (m_linkRunning) {
        m_linkMgr.startAllLinks();
        m_actToggle->setText("停止采集");
    } else {
        m_linkMgr.stopAllLinks();
        m_actToggle->setText("启动采集");
    }
}

void MainWindow::onCollectData(const CollectDataItem& item)
{
    HistoryDataPayload p;
    p.devUuid     = item.devUuid;
    p.regName     = item.regName;
    p.value       = item.value;
    p.collectTime = item.time;
    DbTask task;
    task.type = insert_history;
    task.data = QVariant::fromValue(p);
    m_dbThread.pushTask(task);
    updateRealTimeTable(item);   // 新增：刷新实时表格
    appendCurveData(item.devUuid + "|" + item.regName,
                    item.time.toMSecsSinceEpoch() / 1000.0, item.value);  // 新增：缓存曲线数据
    if (m_cmbCurve->currentText() == item.devUuid + "|" + item.regName)
        updateCurveNow(item);    // 若当前正显示这个点，实时滚动
}

void MainWindow::updateRealTimeTable(const CollectDataItem& item)
{
    QString key = item.devUuid + "|" + item.regName;

    int row = m_rowIndex.value(key, -1);
    if (row < 0) {   // 第一次见到这个点 → 新增一行
        row = m_tableRealTime->rowCount();
        m_tableRealTime->insertRow(row);
        m_tableRealTime->setItem(row, 0, new QTableWidgetItem(m_uuidToName.value(item.devUuid, item.devUuid)));
        m_tableRealTime->setItem(row, 1, new QTableWidgetItem(item.regName));
        m_tableRealTime->setItem(row, 2, new QTableWidgetItem());
        m_tableRealTime->setItem(row, 3, new QTableWidgetItem());
        m_rowIndex[key] = row;
    }

    m_tableRealTime->item(row, 2)->setText(QString::number(item.value));
    m_tableRealTime->item(row, 3)->setText(item.time.toString("HH:mm:ss"));
}

void MainWindow::onAlarm(const AlarmItem& alarm)
{
    AlarmPayload p;
    p.devUuid    = alarm.devUuid;
    p.devName    = alarm.devName;
    p.regName    = alarm.regName;
    p.value      = alarm.value;
    p.threshold  = alarm.threshold;
    p.alarmType  = alarm.alarmType;
    p.occurTime  = alarm.occurTime;
    DbTask task;
    task.type = insert_alarm;
    task.data = QVariant::fromValue(p);
    m_dbThread.pushTask(task);
    addAlarmToTable(alarm);
}

void MainWindow::addAlarmToTable(const AlarmItem& alarm)
{
    QString typeText;
    if (alarm.alarmType == "high")    typeText = "高报警";
    else if (alarm.alarmType == "low") typeText = "低报警";
    else                               typeText = "恢复";

    int row = m_tableAlarm->rowCount();
    m_tableAlarm->insertRow(0);   // 插到最上面，最新报警最显眼
    m_tableAlarm->setItem(0, 0, new QTableWidgetItem(alarm.occurTime.toString("HH:mm:ss")));
    m_tableAlarm->setItem(0, 1, new QTableWidgetItem(alarm.devName));
    m_tableAlarm->setItem(0, 2, new QTableWidgetItem(alarm.regName));
    m_tableAlarm->setItem(0, 3, new QTableWidgetItem(typeText));
    m_tableAlarm->setItem(0, 4, new QTableWidgetItem(QString::number(alarm.value)));
    m_tableAlarm->setItem(0, 5, new QTableWidgetItem(QString::number(alarm.threshold)));

    // 报警行标红，恢复行标绿（可选）
    QColor c = (alarm.alarmType == "recover") ? QColor(0, 150, 0) : QColor(200, 0, 0);
    for (int col = 0; col < 6; ++col)
        m_tableAlarm->item(0, col)->setForeground(c);
}

void MainWindow::onLinkStatus(bool online, const QString& info)
{
    Q_UNUSED(online);
    m_statusLabel->setText(info);
    statusBar()->showMessage(info, 5000);
}

void MainWindow::appendCurveData(const QString& key, double t, double v)
{
    // 缓存每个点的历史（限制最多 600 点，防内存无限涨）
    auto& times = m_curveTimes[key];
    auto& vals  = m_curveValues[key];
    times.append(t);
    vals.append(v);
    if (times.size() > 600) { times.removeFirst(); vals.removeFirst(); }

    // 新点第一次出现 → 加进曲线下拉框
    if (m_cmbCurve->findText(key) < 0)
        m_cmbCurve->addItem(key);
}

void MainWindow::onCurveSelect(const QString& key)
{
    m_plot->graph(0)->data()->clear();
    if (m_curveTimes.contains(key)) {
        const auto& times = m_curveTimes[key];
        const auto& vals  = m_curveValues[key];
        for (int i = 0; i < times.size(); ++i)
            m_plot->graph(0)->addData(times[i], vals[i]);
    }
    m_plot->rescaleAxes();
    m_plot->replot();
}

void MainWindow::updateCurveNow(const CollectDataItem& item)
{
    double t = item.time.toMSecsSinceEpoch() / 1000.0;
    m_plot->graph(0)->addData(t, item.value);
    // 滚动窗口：显示最近 60 秒
    m_plot->graph(0)->data()->removeBefore(t - 60);
    m_plot->xAxis->setRange(t - 60, t);
    m_plot->graph(0)->rescaleValueAxis(false, true);
    m_plot->replot();
}