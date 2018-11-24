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
#include <QTime>
#include <QFileDialog>

#include "stationport.h"
#include "plantformapply.h"

#define PROJECTFILE_TEXT_RFID           "RFID_POS\n"
#define PROJECTFILE_TEXT_CARRIER        "CARRIER_PRF\n"

#define PROJECTFILE_SUFFIX              "apd"
#define CARRIERAUTOSCRIPTFILE_SUFFIX    "casf"

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

    if((m_pCasfCreatorProcess != NULL) && (m_pCasfCreatorProcess->state() == QProcess::Running))
    {
        m_pCasfCreatorProcess->close();
        delete m_pCasfCreatorProcess;
    }

    delete m_pStationPort;
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //要求为自动脚本文件才接收拖动
    if(!event->mimeData()->urls()[0].fileName().right(4).compare(CARRIERAUTOSCRIPTFILE_SUFFIX))
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
        if(fileList.at(index).fileName().right(4).compare(CARRIERAUTOSCRIPTFILE_SUFFIX))
        {
            //不为对应文件
            QMessageBox::critical(this,tr("Cannot Open file"),tr("非自动脚本文件"));
            return;
        }

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
    setWindowTitle(tr("CASF-ActionPlayer V3.0.6"));

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
    connect(ui->AC_file_addExistProjectFile,&QAction::triggered,this,&MainWindow::OnAddExistProjectFile);
    connect(ui->AC_help_RegesitFileRelation,&QAction::triggered,this,&MainWindow::OnRegesitFileRelation);
    connect(ui->AC_arose_casfCreator,&QAction::triggered,this,&MainWindow::OnAroseCasfCreator);
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

void MainWindow::loadProjectFile(QUrl fileUrl)
{
    //从文件读取工程配置：RFID点，载体车

    if(fileUrl.fileName().right(3).compare(PROJECTFILE_SUFFIX))
    {
        //不为对应文件
        QMessageBox::critical(this,tr("Cannot Open file"),tr("非工程文件"));
        return;
    }


    //避免重复加载
    if(m_bIsProjectFillLoaded)
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("已加载文件"));
        return;
    }

    QString filePath = fileUrl.toLocalFile();
    QFile projectFile;
    QFileInfo fileInfo(filePath);

    //判断文件是否存在
    if(!fileInfo.isFile())
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("文件不存在"));
        return;
    }

    //打开文件
    projectFile.setFileName(filePath);
    if(!projectFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("无法打开文件"));
        return;
    }

    //1--加载RFID点位
    int iRfidNum = 0;
    QList<QString> rfidList;
    projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_RFID,projectFile.readLine()))
        {
            iRfidNum = QString(projectFile.readLine()).toInt();
            break;
        }
    }
    for(int i = 0;i < iRfidNum;i++)
    {
        QString lineData = projectFile.readLine();
        if(lineData.isEmpty())
            break;

        QStringList lineConfigList = lineData.split(" ");
        if(lineConfigList.count() != 3)
        {
            QMessageBox::critical(this,tr("Cannot Open File"),tr("无效的RFID点位格式"));
            return;
        }

        QString x = lineConfigList.at(0);
        QString y = lineConfigList.at(1);
        QString number = lineConfigList.at(2);
        QString str = x + " " + y + " " + number;
        rfidList<<str;
    }
    //给pathwayGV传入rfidList初始化RFID视图
    ui->GV_CarrierPathway->initPathwayRfid(rfidList);


    //2--加载载体车信息
    m_iCarrierNum = 1;
    QList<QString> carrierList;
    projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_CARRIER,projectFile.readLine()))
        {
            m_iCarrierNum = QString(projectFile.readLine()).toInt();
            break;
        }
    }
    for(int i = 0;i < m_iCarrierNum;i++)
    {
        QString lineData = projectFile.readLine();
        if(lineData.isEmpty())
            break;

        QStringList lineConfigList = lineData.split(" ");
        if(lineConfigList.count() != 4)
        {
            QMessageBox::critical(this,tr("Cannot Open File"),tr("无效的RFID点位格式"));
            return;
        }

        QString model = QString(lineConfigList.at(0));
        QString pos = QString(lineConfigList.at(1));
        QString speed = QString(lineConfigList.at(2));
        QString status = QString(lineConfigList.at(3));

        QString str = model + " " + pos + " " + speed + " " + status;
        carrierList<<str;

    }
    //初始化载体车
    m_pCarrier = new Carrier(carrierList,this);
    m_pCarrier->BandViewer(ui->TV_Carrier);
    ui->GV_CarrierPathway->initGraphicCarrier(carrierList);


    //已加载工程文件
    m_bIsProjectFillLoaded = true;
    ui->BTN_Option->setEnabled(true);
    ui->GV_CarrierPathway->setEnabled(true);
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
    m_pCarrier->OnStartHeartbeatTimer();

    connect(m_pCarrier,&Carrier::RequestPrintDebugMessage,this,&MainWindow::printMessage);
    connect(m_pCarrier,&Carrier::RequestSendPackageData,m_pStationPort,&StationPort::SendPackageData);
    connect(m_pCarrier,&Carrier::RequestAfterAllCarStandby,m_pActionPlayer,&ActionPlayer::doNextStep);

    connect(m_pActionPlayer,&ActionPlayer::RequestTriggerAfterCarrierStandby,m_pCarrier,&Carrier::OnActionplayerwaittingTrigger);
    connect(m_pRealActionActuator,&RealActionActuator::RequestStartHeartbeatTimer,m_pCarrier,&Carrier::OnStartHeartbeatTimer);
    connect(m_pRealActionActuator,&RealActionActuator::RequestStopHeartbeatTimer,m_pCarrier,&Carrier::OnStopHearbeatTimer);

    connect(m_pStationPort,&StationPort::RequestSetCarrierStatus,m_pCarrier,&Carrier::OnSetCarrierStatus);
    connect(m_pStationPort,&StationPort::RequestSetCarrierStatus,ui->GV_CarrierPathway,&PathwayGV::onUpdateGraphicCarrier);
    connect(m_pStationPort,&StationPort::RequestSetCarrierProfile,m_pCarrier,&Carrier::OnSetCarrierProfile);

}

void MainWindow::componentDeinit()
{
    disconnect(m_pCarrier,&Carrier::RequestPrintDebugMessage,this,&MainWindow::printMessage);
    disconnect(m_pCarrier,&Carrier::RequestSendPackageData,m_pStationPort,&StationPort::SendPackageData);
    disconnect(m_pCarrier,&Carrier::RequestAfterAllCarStandby,m_pActionPlayer,&ActionPlayer::doNextStep);

    disconnect(m_pActionPlayer,&ActionPlayer::RequestTriggerAfterCarrierStandby,m_pCarrier,&Carrier::OnActionplayerwaittingTrigger);
    disconnect(m_pRealActionActuator,&RealActionActuator::RequestStartHeartbeatTimer,m_pCarrier,&Carrier::OnStartHeartbeatTimer);
    disconnect(m_pRealActionActuator,&RealActionActuator::RequestStopHeartbeatTimer,m_pCarrier,&Carrier::OnStopHearbeatTimer);

    disconnect(m_pStationPort,&StationPort::RequestSetCarrierStatus,m_pCarrier,&Carrier::OnSetCarrierStatus);
    disconnect(m_pStationPort,&StationPort::RequestSetCarrierStatus,ui->GV_CarrierPathway,&PathwayGV::onUpdateGraphicCarrier);
    disconnect(m_pStationPort,&StationPort::RequestSetCarrierProfile,m_pCarrier,&Carrier::OnSetCarrierProfile);

    //动作播放器
    disconnect(m_pActionPlayer,&ActionPlayer::RequestPrintMessage,this,&MainWindow::printMessage);
    delete m_pActionPlayer;

    //动作执行器
    disconnect(m_pRealActionActuator,&RealActionActuator::RequestSendPackageData,m_pStationPort,&StationPort::SendPackageData);
    delete m_pRealActionActuator;

    //载体车
    m_pCarrier->OnStopHearbeatTimer();
}

/*      SLOT     */

void MainWindow::OnAddExistActionScriptFile()
{
    QList<QUrl> list = QFileDialog::getOpenFileUrls(this,tr("添加现有动作文件"),QUrl("."),"*.casf");
    if(!list.isEmpty())
        this->addActionScriptFile(list);
}

void MainWindow::OnAddExistProjectFile()
{
    QUrl fileUrl = QFileDialog::getOpenFileUrl(this,tr("添加现有工程文件"),QUrl("."),"*.apd");
    if(!fileUrl.isEmpty())
        this->loadProjectFile(fileUrl);
}

void MainWindow::OnAroseCasfCreator()
{
    //创建process，唤起CASF-Creator
    m_pCasfCreatorProcess = new QProcess();

    //错误处理
    connect(m_pCasfCreatorProcess,&QProcess::errorOccurred,this,&MainWindow::OnAroseCasfCreatorError);

    m_pCasfCreatorProcess->start(CASFCREATOR_PATH);

}

void MainWindow::OnAroseCasfCreatorError(QProcess::ProcessError error)
{
    //唤起CASF-Creator错误处理
    switch(error)
     {
      case QProcess::FailedToStart:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
      case QProcess::Crashed:
        QMessageBox::information(0,"Crashed","Crashed");
        break;
      case QProcess::Timedout:
        QMessageBox::information(0,"FailedToStart","FailedToStart");
        break;
      case QProcess::WriteError:
        QMessageBox::information(0,"Timedout","Timedout");
        break;
      case QProcess::ReadError:
        QMessageBox::information(0,"ReadError","ReadError");
        break;
      case QProcess::UnknownError:
        QMessageBox::information(0,"UnknownError","UnknownError");
        break;
      default:
        QMessageBox::information(0,"default","default");
        break;
     }
}

void MainWindow::OnRegesitFileRelation()
{
    if(QMessageBox::Ok == QMessageBox::information(this,tr("Information"),tr("注册文件关联可能需要管理员权限运行！"),QMessageBox::Ok,QMessageBox::Cancel))
    {
        QString strApplicationFilePath = qApp->applicationFilePath();
        strApplicationFilePath.replace("/","\\");
        QString strApdIconPath = strApplicationFilePath + QString(",1");
        QString strCasfIconPath = strApplicationFilePath + QString(",2");

        QByteArray ba = strApplicationFilePath.toLocal8Bit();
        char* charAppFilePath = new char[ba.length()];
        strcpy(charAppFilePath,ba.data());

        ba = strApdIconPath.toLocal8Bit();
        char* charApdPath = new char[ba.length()];
        strcpy(charApdPath,ba.data());

        ba = strCasfIconPath.toLocal8Bit();
        char* charCasfPath = new char[ba.length()];
        strcpy(charCasfPath,ba.data());


        RegisterFileRelation((char*)".apd",
                             (char*)"ActionPlayerData.Image.1",
                                charAppFilePath,
                                charApdPath,
                                (char*)"ActionPlayer Project Data");

        RegisterFileRelation((char*)".casf",
                             (char*)"CarrierAutoScriptFile.Image.2",
                                (char*)"",
                                charCasfPath,
                                (char*)"Carrier Auto Script File");
    }

}

void MainWindow::printMessage(QString str)
{
    QTime _NowTime = QTime::currentTime();
    str.prepend(QString(_NowTime.toString("hh-mm-ss:  ")));

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
        m_pActionPlayer->stopActionPlayer();
    }
    else
    {
        this->printMessage(QString(tr("载体车未都处于待机状态")));
    }
}

void MainWindow::on_PTE_MessageWindow_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    if(QMessageBox::Ok == QMessageBox::information(this,tr("清除调试信息"),tr("是否要清除调试信息？"),QMessageBox::Ok,QMessageBox::Cancel))
    {
        ui->PTE_MessageWindow->clear();
    }

    return;
}
