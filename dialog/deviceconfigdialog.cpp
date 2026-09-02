#include "deviceconfigdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>

DeviceConfigDialog::DeviceConfigDialog(const DeviceEntity& dev,QWidget* parent): QDialog(parent){
    setupUi();
    fillFromDevice(dev);
}

void DeviceConfigDialog::setupUi(){
    setWindowTitle("设备配置");
    setMinimumSize(600,500);

    m_edtName = new QLineEdit;
    m_spinSlaveId = new QSpinBox;
    m_spinSlaveId->setRange(1,247);
    m_cmbCommType = new QComboBox;
    m_cmbCommType->addItem("RTU","rtu");
    m_cmbCommType->addItem("TCP","tcp");
    m_edtPortParam = new QLineEdit;
    m_edtPortParam->setPlaceholderText("RTU:COM3 / TCP: 192.168.1.100:502");
    m_checkEnable = new QCheckBox("启用");
    auto* form = new QFormLayout;
    form->addRow("设备名称",m_edtName);
    form->addRow("从站地址",m_spinSlaveId);
    form->addRow("通信类型",m_cmbCommType);
    form->addRow("端口参数",m_edtPortParam);
    form->addRow("",m_checkEnable);

    m_tableRegs = new QTableWidget(0,6);
    m_tableRegs->setHorizontalHeaderLabels({"名称", "地址", "长度", "数据类型", "低报警", "高报警"});
    m_tableRegs->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    auto* btnAdd = new QPushButton("添加寄存器");
    auto *btnDel = new QPushButton("删除寄存器");
    connect(btnAdd,&QPushButton::clicked,this,&DeviceConfigDialog::onAddReg);
    connect(btnDel,&QPushButton::clicked,this,&DeviceConfigDialog::onDelReg);
    auto* regBtns = new QHBoxLayout;
    regBtns->addWidget(btnAdd);
    regBtns->addWidget(btnDel);
    regBtns->addStretch();

    auto* btnOk = new QPushButton("确定");
    auto* btnCancel = new QPushButton("取消");
    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    auto* dlgBtns = new QHBoxLayout;
    dlgBtns->addStretch();
    dlgBtns->addWidget(btnOk);
    dlgBtns->addWidget(btnCancel);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(new QLabel("寄存器列表:"));
    root->addWidget(m_tableRegs);
    root->addLayout(regBtns);
    root->addLayout(dlgBtns);
}

void DeviceConfigDialog::fillFromDevice(const DeviceEntity& dev)
{
    m_edtName->setText(dev.devName);
    m_spinSlaveId->setValue(dev.slaveId);
    int idx = m_cmbCommType->findData(dev.commType);
    m_cmbCommType->setCurrentIndex(idx < 0 ? 0 : idx);
    m_edtPortParam->setText(dev.portParam);
    m_checkEnable->setChecked(dev.enable);

    for (const auto& reg : dev.regList) {
        int row = m_tableRegs->rowCount();
        m_tableRegs->insertRow(row);
        m_tableRegs->setItem(row, 0, new QTableWidgetItem(reg.name));
        m_tableRegs->setItem(row, 1, new QTableWidgetItem(QString::number(reg.addr)));
        m_tableRegs->setItem(row, 2, new QTableWidgetItem(QString::number(reg.len)));

        auto* cmbType = new QComboBox;
        cmbType->addItem("Int16",   (int)DataType::Int16);
        cmbType->addItem("UInt16",  (int)DataType::UInt16);
        cmbType->addItem("Int32",   (int)DataType::Int32);
        cmbType->addItem("UInt32",  (int)DataType::UInt32);
        cmbType->addItem("Float32", (int)DataType::Float32);
        cmbType->setCurrentIndex(cmbType->findData((int)reg.dataType));
        m_tableRegs->setCellWidget(row, 3, cmbType);

        m_tableRegs->setItem(row, 4, new QTableWidgetItem(QString::number(reg.lowAlarm)));
        m_tableRegs->setItem(row, 5, new QTableWidgetItem(QString::number(reg.highAlarm)));
    }
}

void DeviceConfigDialog::onAddReg()
{
    int row = m_tableRegs->rowCount();
    m_tableRegs->insertRow(row);
    m_tableRegs->setItem(row, 0, new QTableWidgetItem("新寄存器"));
    m_tableRegs->setItem(row, 1, new QTableWidgetItem("0"));
    m_tableRegs->setItem(row, 2, new QTableWidgetItem("1"));

    auto* cmbType = new QComboBox;
    cmbType->addItem("Int16",   (int)DataType::Int16);
    cmbType->addItem("UInt16",  (int)DataType::UInt16);
    cmbType->addItem("Int32",   (int)DataType::Int32);
    cmbType->addItem("UInt32",  (int)DataType::UInt32);
    cmbType->addItem("Float32", (int)DataType::Float32);
    m_tableRegs->setCellWidget(row, 3, cmbType);

    m_tableRegs->setItem(row, 4, new QTableWidgetItem("0"));
    m_tableRegs->setItem(row, 5, new QTableWidgetItem("0"));
}

void DeviceConfigDialog::onDelReg()
{
    int row = m_tableRegs->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选中要删除的寄存器行");
        return;
    }
    m_tableRegs->removeRow(row);
}

DeviceEntity DeviceConfigDialog::getDevice() const
{
    DeviceEntity dev;
    dev.devName  = m_edtName->text().trimmed();
    dev.slaveId  = m_spinSlaveId->value();
    dev.commType = m_cmbCommType->currentData().toString();
    dev.portParam = m_edtPortParam->text().trimmed();
    dev.enable   = m_checkEnable->isChecked();

    for (int row = 0; row < m_tableRegs->rowCount(); ++row) {
        RegisterItem reg;
        reg.name = m_tableRegs->item(row, 0) ? m_tableRegs->item(row, 0)->text() : "";
        reg.addr = m_tableRegs->item(row, 1) ? m_tableRegs->item(row, 1)->text().toInt() : 0;
        reg.len  = m_tableRegs->item(row, 2) ? m_tableRegs->item(row, 2)->text().toInt() : 1;

        auto* cmb = qobject_cast<QComboBox*>(m_tableRegs->cellWidget(row, 3));
        reg.dataType = cmb ? (DataType)cmb->currentData().toInt() : DataType::Int16;

        reg.lowAlarm  = m_tableRegs->item(row, 4) ? m_tableRegs->item(row, 4)->text().toDouble() : 0;
        reg.highAlarm = m_tableRegs->item(row, 5) ? m_tableRegs->item(row, 5)->text().toDouble() : 0;
        dev.regList.append(reg);
    }
    return dev;
}