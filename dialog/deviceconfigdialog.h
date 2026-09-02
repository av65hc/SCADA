#ifndef DEVICECONFIGDIALOG_H
#define DEVICECONFIGDIALOH_H

#include <QDialog>
#include "entity/deviceentity.h"

class QLineEdit;
class QSpinBox;
class QComboBox;
class QCheckBox;
class QTableWidget;

class DeviceConfigDialog: public QDialog{
    Q_OBJECT
public :
    explicit DeviceConfigDialog(const DeviceEntity &dev,QWidget* parent = nullptr);
    DeviceEntity getDevice() const;
private slots:
    void onAddReg();
    void onDelReg();
private:
    void setupUi();
    void fillFromDevice(const DeviceEntity &dev);
    QLineEdit* m_edtName = nullptr;
    QSpinBox* m_spinSlaveId = nullptr;
    QComboBox* m_cmbCommType = nullptr;
    QLineEdit* m_edtPortParam = nullptr;
    QCheckBox* m_checkEnable = nullptr;
    QTableWidget* m_tableRegs = nullptr;
};

#endif
