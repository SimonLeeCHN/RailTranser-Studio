#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QMessageBox"
#include <QtSerialPort/QSerialPortInfo>
#include <QListWidgetItem>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDrag>
#include <QMimeData>
#include <QUrl>
#include <QFile>
#include <QIcon>
#include <QScrollBar>
#include <QStringList>
#include <QPainter>
#include <QFileDialog>

#include "stationport.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_pStationPort = new StationPort();
    connect(m_pStationPort,&StationPort::RequestPrintMessage,this,&MainWindow::printMessage);

    this->initWindowStyle();
    this->fillAvaliablePorts();
    this->initMenu();

}

MainWindow::~MainWindow()
{
    if(m_pStationPort->isOpen())
        m_pStationPort->stopConnect();

    delete m_pStationPort;
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //要求为ACS文件才接收拖动
    if(!event->mimeData()->urls()[0].fileName().right(4).compare("casf"))
    {
        event->acceptProposedAction();//接受动作
        //若不添加此函数，则外部文件无法添加到窗体中
    }
    else
    {
        event->ignore();                //忽略事件
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    qDebug()<<"got dropEvent!";

    const QMimeData *mimeData = event->mimeData();
    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList = mimeData->urls();
        this->addActionScriptFile(urlList);
    }
}

void MainWindow::addActionScriptFile(QList<QUrl> fileList)
{
    for(int index = 0;index < fileList.count();index++)
    {

        QString fileName = fileList.at(index).fileName();
        QString filePath = fileList.at(index).toLocalFile();

        QListWidgetItem *item = new QListWidgetItem(fileName);
        item->setData(Qt::UserRole,filePath);

        //防止重复添加
        bool isRepeat = false;
        for(int i = 0; i < ui->LW_ActionSortcutList->count();i++)
        {
            if(fileName == (ui->LW_ActionSortcutList->item(i))->text())
                isRepeat = true;
        }
        if(isRepeat != true)
            ui->LW_ActionSortcutList->addItem(item);

        qDebug()<<"filePath: "<<filePath;
        qDebug()<<"fileName:"<<fileName;
    }
}

void MainWindow::initWindowStyle()
{
    //设置窗口标题
//    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setWindowTitle(tr("SimpleActionPlayer"));

    //控件初始化
    ui->BTN_Stop->setEnabled(false);
    ui->BTN_ReLocate->setEnabled(false);
    ui->LW_ActionSortcutList->setEnabled(false);
    ui->BTN_Option->setText(tr("Connect"));

    //将LW的拖拽屏蔽，使得MainWindow可以截获拖拽响应
    ui->LW_ActionSortcutList->setAcceptDrops(false);
    this->setAcceptDrops(true);

}

void MainWindow::initMenu()
{
    connect(ui->AC_file_addExistActionScriptFile,&QAction::triggered,this,&MainWindow::OnAddExistActionScriptFile);
}

void MainWindow::fillAvaliablePorts()
{
    //清除端口号
    ui->CB_Port->clear();

    //获取并填充可用串口号进入端口box
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info :infos)
    {
        ui->CB_Port->addItem(info.portName());
    }
}

void MainWindow::componentInit()
{
    //动作播放器
    m_pActionPlayer = new ActionPlayer();
    connect(m_pActionPlayer,&ActionPlayer::RequestPrintMessage,this,&MainWindow::printMessage);

    //动作执行器
    m_pRealActionActuator = new RealActionActuator();
    connect(m_pRealActionActuator,&RealActionActuator::RequestSendPackageData,m_pStationPort,&StationPort::SendPackageData);

    //载体车
    m_iCarrierNum = ui->SB_CarrierNum->value();
    m_pCarrier = new Carrier(m_iCarrierNum,this);
    m_pCarrier->BandViewer(ui->TV_Carrier);

    connect(m_pCarrier,Carrier::RequestPrintDebugMessage,this,MainWindow::printMessage);
    connect(m_pCarrier,Carrier::RequestSendPackageData,m_pStationPort,StationPort::SendPackageData);
    connect(m_pCarrier,Carrier::RequestAfterAllCarStandby,m_pActionPlayer,ActionPlayer::doNextStep);
    connect(m_pActionPlayer,ActionPlayer::RequestTriggerAfterCarrierStandby,m_pCarrier,Carrier::OnActionplayerwaittingTrigger);
    connect(m_pRealActionActuator,RealActionActuator::RequestStartHeartbeatTimer,m_pCarrier,Carrier::OnStartHeartbeatTimer);
    connect(m_pRealActionActuator,RealActionActuator::RequestStopHeartbeatTimer,m_pCarrier,Carrier::OnStopHearbeatTimer);
    connect(m_pStationPort,StationPort::RequestSetCarrierStatus,m_pCarrier,Carrier::OnSetCarrierStatus);
    connect(m_pStationPort,StationPort::RequestSetCarrierProfile,m_pCarrier,Carrier::OnSetCarrierProfile);

}

void MainWindow::componentDeinit()
{
    //动作播放器
    disconnect(m_pActionPlayer,&ActionPlayer::RequestPrintMessage,this,&MainWindow::printMessage);
    delete m_pActionPlayer;

    //动作执行器
    disconnect(m_pRealActionActuator,&RealActionActuator::RequestSendPackageData,m_pStationPort,&StationPort::SendPackageData);
    delete m_pRealActionActuator;

    //载体车
    m_iCarrierNum = 0;
    disconnect(m_pCarrier,Carrier::RequestPrintDebugMessage,this,MainWindow::printMessage);
    disconnect(m_pCarrier,Carrier::RequestSendPackageData,m_pStationPort,StationPort::SendPackageData);
    disconnect(m_pCarrier,Carrier::RequestAfterAllCarStandby,m_pActionPlayer,ActionPlayer::doNextStep);
    disconnect(m_pActionPlayer,ActionPlayer::RequestTriggerAfterCarrierStandby,m_pCarrier,Carrier::OnActionplayerwaittingTrigger);
    disconnect(m_pRealActionActuator,RealActionActuator::RequestStartHeartbeatTimer,m_pCarrier,Carrier::OnStartHeartbeatTimer);
    disconnect(m_pRealActionActuator,RealActionActuator::RequestStopHeartbeatTimer,m_pCarrier,Carrier::OnStopHearbeatTimer);
    disconnect(m_pStationPort,StationPort::RequestSetCarrierStatus,m_pCarrier,Carrier::OnSetCarrierStatus);
    disconnect(m_pStationPort,StationPort::RequestSetCarrierProfile,m_pCarrier,Carrier::OnSetCarrierProfile);
    delete m_pCarrier;
}

/*      SLOT     */

void MainWindow::OnAddExistActionScriptFile()
{
    QList<QUrl> list = QFileDialog::getOpenFileUrls(this,tr("添加现有动作文件"),QUrl("."),"*.casf");
    this->addActionScriptFile(list);
}

void MainWindow::printMessage(QString str)
{
    ui->PTE_MessageWindow->appendPlainText(str);
    ui->PTE_MessageWindow->verticalScrollBar()->setValue(ui->PTE_MessageWindow->verticalScrollBar()->maximumHeight());
}

void MainWindow::on_BTN_Refresh_clicked(bool checked)
{
    Q_UNUSED(checked);

    this->fillAvaliablePorts();
}

void MainWindow::on_BTN_Option_clicked(bool checked)
{
    Q_UNUSED(checked);

    if((m_pStationPort->isOpen()) && (ui->BTN_Option->text() == "Disconnect"))
    {
        //关闭端口
        m_pStationPort->stopConnect();
        this->printMessage(tr("Close SerialPort OK"));

        //界面处理
        ui->BTN_Option->setText(tr("Connect"));
        ui->BTN_Refresh->setEnabled(true);
        ui->BTN_Stop->setEnabled(false);
        ui->BTN_ReLocate->setEnabled(false);
        ui->LW_ActionSortcutList->setEnabled(false);
        ui->SB_CarrierNum->setEnabled(true);

        //解除组件
        this->componentDeinit();
    }
    else
    {
        //打开端口
        QString portNum = ui->CB_Port->currentText();

        if(portNum == "")
        {
            //端口为空
            QMessageBox::warning(this,tr("SerialPort Error"),tr("不能打开空串口"));
            this->printMessage(tr("Can not open empty port"));
            return;
        }

        if(m_pStationPort->startConnect(portNum) == 1)
        {
            //端口打开成功
            QMessageBox::information(this,QString(tr("SerialPort OK")),tr("端口打开成功"));
            this->printMessage(tr("Open SerialPort OK"));

            //加载及连接组件
            this->componentInit();

            //界面处理
            ui->BTN_Option->setText(tr("Disconnect"));
            ui->BTN_Refresh->setEnabled(false);
            ui->BTN_Stop->setEnabled(true);
            ui->BTN_ReLocate->setEnabled(true);
            ui->LW_ActionSortcutList->setEnabled(true);
            ui->SB_CarrierNum->setEnabled(false);

        }
        else
        {
            //端口打开失败
            QMessageBox::warning(this,tr("SerialPort Error"),tr("端口打开失败"));
            this->printMessage(tr("Open SerialPort Error"));
            return;
        }

    }

}

void MainWindow::on_BTN_Stop_clicked(bool checked)
{
    Q_UNUSED(checked);

    QList<QByteArray> tempList;

    for(int i = 0;i < m_iCarrierNum;i++)
    {
        QByteArray tempArray;

        tempArray.append(i+1);      //车辆号
        tempArray.append(1);        //数据

        tempList<<tempArray;
    }
    m_pStationPort->SendPackageData(tempList,PORT_CONTROL_SEND);
    m_pActionPlayer->stopActionPlayer();
}

void MainWindow::on_LW_ActionSortcutList_itemDoubleClicked(QListWidgetItem *item)
{
    if((m_pActionPlayer->getPlayerStatus()) == PLAYERSTU_PLAYING)
    {
        QMessageBox::warning(this,tr("Player error"),tr("当前正在播放"));
        return;
    }

    qDebug()<<"filePath: "<<item->data(Qt::UserRole).toString();

    m_pActionPlayer->loadActionFile(item->data(Qt::UserRole).toString());
    m_pActionPlayer->setActuator(m_pRealActionActuator);
    //m_pActionPlayer->setActuator(new VirtualActionActuator());
    m_pActionPlayer->doNextStep();
}

void MainWindow::on_LW_ActionSortcutList_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    //必须contextMenuPolicy修改为customContextMenuRequested
    qDebug()<<QString("now num: %1").arg(ui->LW_ActionSortcutList->count());

    QListWidgetItem *item;
    int index;

    index = ui->LW_ActionSortcutList->currentRow();
    item = ui->LW_ActionSortcutList->takeItem(index);
    delete item;

    qDebug()<<QString("after delete num: %1").arg(ui->LW_ActionSortcutList->count());
}

void MainWindow::on_BTN_ReLocate_clicked(bool checked)
{
    Q_UNUSED(checked);

    if(!((m_pCarrier->IsAllCarrierStatusSame("运动中")) == 0))
    {
        QList<QByteArray> tempList;

        for(int i = 0;i < m_iCarrierNum;i++)
        {
            QByteArray tempArray;

            tempArray.append(i+1);      //车辆号
            tempArray.append(2);        //数据

            tempList<<tempArray;
        }
        m_pStationPort->SendPackageData(tempList,PORT_CONTROL_SEND);
    }
    else
    {
        this->printMessage(QString(tr("载体车未都处于待机状态")));
    }
}
