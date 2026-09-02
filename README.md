# Modbus SCADA 监控系统

一个基于 **Qt 6** 的多设备 Modbus 上位机监控系统（SCADA）。支持 **Modbus RTU（串口）** 和 **Modbus TCP（网络）** 两种协议，实现设备数据采集、实时显示、实时曲线、上下限报警、历史存储与查询、日志记录，可打包为独立可执行文件交付。

## 功能特性

- **双协议采集**：一套代码同时支持 Modbus RTU 与 Modbus TCP，按设备配置自动切换
- **多设备 / 多链路**：按「通信类型 + 端口」分组，每个物理链路一个独立采集线程，互不阻塞
- **多数据类型**：支持 Int16 / UInt16 / Int32 / UInt32 / Float32 五种解码，大端字序
- **实时监控**：实时数据表格、实时曲线（最近 60 秒滚动窗口）
- **报警管理**：上下限报警 + 状态机去抖（仅在状态跳变时记录）+ 报警恢复 + 报警确认
- **断线重连**：连续读失败自动重连，首次连接失败也会循环重试
- **数据持久化**：SQLite 存储历史数据与报警记录，JSON 存储设备配置，重启不丢
- **历史查询**：按设备 / 寄存器 / 时间范围查询历史并绘制曲线
- **全局日志**：qDebug / qWarning 等自动落盘到 `scada.log`
- **一键打包**：`windeployqt` 打成独立 exe

## 技术栈

| 层 | 技术 |
|---|---|
| 框架 | Qt 6（Core / Widgets / SerialPort / SerialBus / Network / Sql / PrintSupport） |
| 构建 | CMake（≥ 3.19） |
| 协议 | QModbusClient（QModbusRtuSerialClient / QModbusTcpClient） |
| 数据库 | SQLite（QtSql） |
| 绘图 | QCustomPlot（源码内嵌于 `thirdparty/`） |
| 并发 | QThread + moveToThread + 信号槽 + 自研线程安全队列 |

## 目录结构

```
Scada/
├── src/
│   ├── ui/            # 主窗口、程序入口
│   ├── comm/          # 协议封装、采集线程、链路管理
│   ├── database/      # SQLite 封装、异步入库/查询线程
│   ├── entity/        # 数据结构（设备/寄存器/报警）
│   ├── utils/         # 线程安全队列、日志、配置读写
│   └── protocol/      # CRC16（预留，未使用）
├── dialog/            # 设备配置对话框
├── thirdparty/        # QCustomPlot 源码
├── CMakeLists.txt
└── README.md
```

核心模块说明：

| 模块 | 文件 | 职责 |
|---|---|---|
| 协议层 | `src/comm/modbusmaster.*` | 封装 QModbus 连接 / 读寄存器 / 值解码 |
| 采集层 | `src/comm/linkworker.*` | 采集循环、报警判断、断线重连（跑在子线程） |
| 链路管理 | `src/comm/linkmanager.*` | 按物理链路分组、创建 / 启停采集线程 |
| 存储层 | `src/database/dbworker.*` | 异步入库 / 查询 / 报警确认（跑在子线程） |
| 存储层 | `src/database/sqlitedb.*` | 建表、连接管理 |
| UI 层 | `src/ui/mainwindow.*` | 主窗口、五个页面、设备 CRUD |
| UI 层 | `dialog/deviceconfigdialog.*` | 设备 / 寄存器配置对话框 |
| 工具层 | `src/utils/threadqueue.h` | 线程安全阻塞队列（生产者-消费者） |
| 工具层 | `src/utils/loghelper.*` | 全局日志重定向到文件 |
| 工具层 | `src/utils/devconfighelper.*` | 设备配置 JSON 读写 |

## 构建

### 环境要求

- Windows（本机已验证）或 Linux
- Qt 6.5+（开发使用 Qt 6.11.1，MinGW 64-bit）
- CMake 3.19+
- 编译器：MinGW（Windows）/ GCC（Linux）

### 方式一：Qt Creator（推荐）

1. 打开 Qt Creator，菜单 `文件 → 打开文件或项目`，选择项目根目录的 `CMakeLists.txt`
2. 选择对应的构建套件（如 `Desktop Qt 6.11.1 MinGW 64-bit`）
3. 点左下角三角按钮编译运行

### 方式二：命令行

```bash
cmake -S . -B build
cmake --build build --config Release
```

构建产物在 `build/` 目录下。

## 使用

### 1. 添加设备

1. 点击工具栏「添加设备」
2. 填写设备信息：

| 字段 | 说明 | 示例 |
|---|---|---|
| 设备名称 | 任意 | 温控仪 |
| 从站地址 | Modbus 从站 ID（1~247） | 1 |
| 通信类型 | RTU 或 TCP | TCP |
| 端口参数 | 见下方格式说明 | `127.0.0.1:502` |
| 启用 | 勾选后参与采集 | ✔ |

3. 在寄存器列表里添加要采集的寄存器（名称 / 地址 / 长度 / 数据类型 / 低报警 / 高报警）
4. 点确定

### 2. 启动采集

点工具栏「启动采集」，状态栏会显示连接状态，实时数据表开始刷新。

### 3. 各页面说明

| 页面 | 功能 |
|---|---|
| 实时数据 | 表格展示各点最新值 |
| 报警记录 | 报警 / 恢复记录，可选中后「确认」 |
| 实时曲线 | 下拉选择数据点，展示最近 60 秒曲线 |
| 历史查询 | 选设备 / 点 / 时间范围查询历史曲线 |
| 日志 | 展示 `scada.log` 内容 |

## 配置说明

### 端口参数格式

- **RTU**：`端口,波特率,数据位,校验,停止位`，例如 `COM4,9600,8,N,1`
  - 校验位：`N` 无校验 / `E` 偶校验 / `O` 奇校验
- **TCP**：`主机:端口`，例如 `192.168.1.100:502` 或本机环回 `127.0.0.1:502`

### 数据类型

| 枚举值 | 类型 | 说明 |
|---|---|---|
| 0 | Int16 | 有符号 16 位，1 个寄存器 |
| 1 | UInt16 | 无符号 16 位，1 个寄存器 |
| 2 | Int32 | 有符号 32 位，2 个寄存器（大端） |
| 3 | UInt32 | 无符号 32 位，2 个寄存器（大端） |
| 4 | Float32 | 单精度浮点，2 个寄存器（IEEE754） |

### devices.json 格式

设备配置保存在运行目录下的 `devices.json`，结构如下：

```json
[
  {
    "devUuid": "xxxxxxxx-xxxx-...",
    "devName": "温控仪",
    "slaveId": 1,
    "commType": "tcp",
    "portParam": "127.0.0.1:502",
    "enable": true,
    "regList": [
      {
        "name": "温度",
        "addr": 0,
        "len": 1,
        "dataType": 0,
        "lowAlarm": 0,
        "highAlarm": 100
      }
    ]
  }
]
```

## 数据存储

- **`scada.db`**：SQLite 数据库，含三张表
  - `t_history`：历史数据（devUuid / regName / value / collectTime）
  - `t_alarm`：报警记录（含 `isConfirm` 确认标记）
  - `t_log`：日志表（预留）
- **`devices.json`**：设备配置
- **`scada.log`**：运行日志

## 测试

没有真实 Modbus 设备时，用模拟器搭建测试环境：

- **TCP**：用 Modbus Slave（或 modRSsim2）在 `127.0.0.1:502` 模拟从站
- **RTU**：用 com0com 创建虚拟串口对（如 `COM3 ↔ COM4`），Modbus Slave 连一端，程序连另一端

重点测试场景：

1. **断线重连**：采集运行中关闭模拟器再打开，应显示「重连成功」并恢复数据
2. **报警状态机**：值在阈值附近抖动，应只在跳变时记录，不刷屏
3. **多设备并发**：同时开 TCP + RTU 两个链路，停一个不影响另一个
4. **数据解码**：重点测 Float32（设 `1.0` 应显示 1.0）

纯逻辑单元（如 `ModbusMaster::decodeValue`）可用 Qt Test 编写自动化测试。

## 打包发布

1. Qt Creator 切到 `Release` 配置编译
2. 将生成的 `Scada.exe` 拷到一个新目录
3. 打开 Qt 自带的命令行，进入该目录执行：

```bash
windeployqt Scada.exe
```

4. 双击 `Scada.exe` 验证可独立运行（不依赖 Qt Creator）

程序运行时会在 exe 同目录自动创建 `scada.db` / `devices.json` / `scada.log`。

## 已知局限

- 历史报警查询（`query_alarm`）预留但未实现，重启后报警表只展示本次运行的报警
- 删除设备后，实时表格行与曲线缓存未清理（残留旧数据）
- 采集统一使用功能码 0x03（读保持寄存器），未按寄存器种类区分 0x03 / 0x04
- `src/ui/mainwindow.ui` 为历史遗留的空表单，UI 已改为纯代码构建，可删除

## 许可证

- QCustomPlot 遵循 GPL 授权，商用需购买商业许可或遵守 GPL 条款
