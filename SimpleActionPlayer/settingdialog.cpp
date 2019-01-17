#include "settingdialog.h"
#include "ui_settingdialog.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QFileDialog>
#include <QString>
#include "QMessageBox"
#include "mainwindow.h"
#include "carrierargumentsmap.h"

SettingDialog::SettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingDialog)
{
    ui->setupUi(this);

    this->initWindowStyle();

    //apd文件管理器
    m_pApdFileManager = new ApdFileManager(this);

}

SettingDialog::~SettingDialog()
{
    if(m_pApdFileManager != NULL)
        delete m_pApdFileManager;

    if(m_pCarrier != NULL)
        delete m_pCarrier;

    delete ui;
}

void SettingDialog::initWindowStyle()
{
    this->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

bool SettingDialog::writeCarrierProfile()
{
    if(m_pCarrier == NULL)
        return false;

    /*      从表读取    */
    QList<QString> _newCarrierEnabledList;
    for(int _index = 0;_index < m_pCarrier->rowCount();_index++)
    {
        QModelIndex _tempIndex = m_pCarrier->index(_index,1);
        QString _strEnabled = (m_pCarrier->data(_tempIndex)).toString();
        _newCarrierEnabledList << QString::number(ConvertStringToCmd(map_CarenableCmd,_strEnabled));
    }

    /*      调用apdfilemanager写入      */
    if(m_pApdFileManager->setFileCarrierEnabled(ui->LBL_ApdFilePath->text(),_newCarrierEnabledList))
        return true;
    else
        return false;

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
        QMessageBox::warning(this,tr("端口设置错误"),tr("端口不得为空,端口未设置"));
    }

    /*      TAB2        */
    if(m_pCarrier != NULL)
    {
        if(this->writeCarrierProfile())
        {
            QMessageBox::critical(this,tr("车辆配置成功"),tr("写入文件成功，请重新载入工程文件"));
        }
        else
        {
            QMessageBox::critical(this,tr("车辆配置失败"),tr("写入文件错误"));
        }
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
    /*  获取参数    */
    QUrl _fileUrl = QFileDialog::getOpenFileUrl(this,tr("添加现有工程文件"),QUrl("."),"*.apd");
    ui->LBL_ApdFilePath->setText(_fileUrl.toLocalFile());

    QList<QString> _carrierConfigList = m_pApdFileManager->getFileCarrierConfigList(_fileUrl.toLocalFile());

    /*  添加  */
    if(m_pCarrier != NULL)
        delete m_pCarrier;

    m_pCarrier = new QStandardItemModel(_carrierConfigList.count(),2);
    QStringList _modelHeader = {tr("Model"),tr("Enabled")};
    m_pCarrier->setHorizontalHeaderLabels(_modelHeader);

    for(int row = 0;row < _carrierConfigList.count();row++)
    {
        QStringList _profileList = QString(_carrierConfigList.value(row)).split(" ");
        for(int column = 0 ; column < 2 ; column++)
        {
            QModelIndex _modelIndex = m_pCarrier->index(row,column);
            switch(column)
            {
                case 0:
                    m_pCarrier->setData(_modelIndex,QVariant(QString(_profileList.value(0))));
                    break;
                case 1:
                    m_pCarrier->setData(_modelIndex,QVariant(ConvertCmdToString(map_CarenableCmd,QString(_profileList.value(4)).toInt())));
                    break;
                default:
                    break;
            }
        }
    }

    //设置delegate
    ui->TV_CarrierConfig->setItemDelegateForColumn(1,&m_carrierEnableDelegate);

    ui->TV_CarrierConfig->setModel(m_pCarrier);

    return;

}
