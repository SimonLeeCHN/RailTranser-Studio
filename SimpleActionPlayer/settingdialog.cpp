#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>
#include <QString>
#include "QMessageBox"
#include "mainwindow.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    this->initWindowStyle();

}

SettingDialog::~SettingDialog()
{

    delete ui;
}

void SettingDialog::initWindowStyle()
{
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void SettingDialog::fillAvaliablePorts()
{
    ui->CB_PortName->clear();

    QList<QSerialPortInfo> _infos = QSerialPortInfo::availablePorts();
    for(const QSerialPortInfo &info : _infos)
    {
        ui->CB_PortName->addItem(info.portName());
    }

    //若第一个ComboBox项非空，将其发射给mainwindow作为默认端口
    if(ui->CB_PortName->count() != 0)
    {
        QString _portName = ui->CB_PortName->currentText();
        emit RequestSetStationPort(_portName);
    }
}

void SettingDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    this->setVisible(false);
}

void SettingDialog::on_BTN_PortRefrush_clicked()
{
    this->fillAvaliablePorts();
}

/*
 *  TODO:通过检查ZIGBEE内部固定寄存器返回值检测是否链接成功
 */
void SettingDialog::on_BTN_ConnectDetect_clicked()
{

}

void SettingDialog::on_BTN_ConfirmSets_clicked()
{
    /*      TAB1        */
    //设置端口号
    QString _portName = ui->CB_PortName->currentText();
    if(!(_portName.isNull()))
    {
        emit RequestSetStationPort(_portName);

    }else
    {
        QMessageBox::warning(this,tr("端口设置错误"),tr("端口不得为空"));
        return;
    }



    //对话框隐藏
    this->setVisible(false);
}

void SettingDialog::on_BTN_CancelSets_clicked()
{
    //对话框隐藏
    this->setVisible(false);
}

void SettingDialog::on_BTN_LoadApdFile_clicked()
{
    QUrl _fileUrl = QFileDialog::getOpenFileUrl(this,tr("添加现有工程文件"),QUrl("."),"*.apd");
    ui->LBL_ApdFilePath->setText(_fileUrl.toLocalFile());

    MainWindow* _pParent = (MainWindow*)parent();
    QList<QString> _carrierConfigList = _pParent->getFileCarrierConfigList(_fileUrl);

    return;







}
