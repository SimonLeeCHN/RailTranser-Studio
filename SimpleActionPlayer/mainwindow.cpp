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

    //工作站端口
    m_pStationPort = new StationPort();
    connect(m_pStationPort,&StationPort::RequestPrintMessage,this,&MainWindow::printMessage);

    //动作播放器
    m_pActionPlayer = new ActionPlayer();
    connect(m_pActionPlayer,&ActionPlayer::RequestPrintMessage,this,&MainWindow::printMessage);

    //动作执行器
    m_pRealActionActuator = new RealActionActuator();
    m_pActionPlayer->setActuator(m_pRealActionActuator);

    //设置窗口
    m_pSettingDialog = new SettingDialog(this);
    connect(m_pSettingDialog,&SettingDialog::RequestSetStationPort,this,&MainWindow::OnSetStationPort);
    m_pSettingDialog->fillAvaliablePorts();

    //工具箱窗口
    m_pToolBoxDialog = new ToolBoxDialog(this);
    connect(m_pToolBoxDialog,&ToolBoxDialog::RequestActionPlayerLoadList,m_pActionPlayer,&ActionPlayer::loadActionList);
    connect(m_pToolBoxDialog,&ToolBoxDialog::RequestActionPlayerDoNextStep,m_pActionPlayer,&ActionPlayer::doNextStep);

    this->initWindowStyle();
    this->initMenu();

}

MainWindow::~MainWindow()
{
    //工作站端口
    if(m_pStationPort->isOpen())
        m_pStationPort->stopConnect();
    delete m_pStationPort;

    //动作播放器
    disconnect(m_pActionPlayer,&ActionPlayer::RequestPrintMessage,this,&MainWindow::printMessage);
    delete m_pActionPlayer;

    //动作执行器
    delete m_pRealActionActuator;

    //设置窗口
    delete m_pSettingDialog;

    //工具箱窗口
    disconnect(m_pToolBoxDialog,&ToolBoxDialog::RequestActionPlayerLoadList,m_pActionPlayer,&ActionPlayer::loadActionList);
    disconnect(m_pToolBoxDialog,&ToolBoxDialog::RequestActionPlayerDoNextStep,m_pActionPlayer,&ActionPlayer::doNextStep);
    delete m_pToolBoxDialog;

    //casf-creator进程
    if((m_pCasfCreatorProcess != NULL) && (m_pCasfCreatorProcess->state() == QProcess::Running))
    {
        m_pCasfCreatorProcess->close();
        delete m_pCasfCreatorProcess;
    }

    //carriermanager
    if(m_pCarrierManager != NULL)
        delete m_pCarrierManager;

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
        if(fileList.value(index).fileName().right(4).compare(CARRIERAUTOSCRIPTFILE_SUFFIX))
        {
            //不为对应文件
            QMessageBox::critical(this,tr("Cannot Open file"),tr("非自动脚本文件"));
            return;
        }

        QString fileName = fileList.value(index).fileName();
        QString filePath = fileList.value(index).toLocalFile();

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
    setWindowTitle(tr("CASF-ActionPlayer V3.2.1"));

    //控件初始化
    ui->LW_ActionSortcutList->setEnabled(false);

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
    connect(ui->AC_file_settings,&QAction::triggered,this,&MainWindow::onActAroseSettingDialog);

    //toolbar
    QAction *_actStartLink = new QAction(QIcon(":/img/ctrol_startLink"),tr("Start Link"),this);
    _actStartLink->setStatusTip(tr("开启连接"));
    _actStartLink->setEnabled(false);
    ui->toolBar->addAction(_actStartLink);
    connect(this,&MainWindow::RequestActStartLinkSetEnabled,_actStartLink,&QAction::setEnabled);
    connect(_actStartLink,&QAction::triggered,this,&MainWindow::onActStartLinkTriggered);

    QAction *_actStopLink = new QAction(QIcon(":/img/ctrol_stopLink"),tr("Stop Link"),this);
    _actStopLink->setStatusTip(tr("断开连接"));
    _actStopLink->setEnabled(false);
    ui->toolBar->addAction(_actStopLink);
    connect(this,&MainWindow::RequestActStopLinkSetEnabled,_actStopLink,&QAction::setEnabled);
    connect(_actStopLink,&QAction::triggered,this,&MainWindow::onActStopLinkTriggered);

    QAction *_actRelocate = new QAction(QIcon(":/img/ctrol_relocate"),tr("Relocate"),this);
    _actRelocate->setStatusTip(tr("载体车复位"));
    _actRelocate->setEnabled(false);
    ui->toolBar->addAction(_actRelocate);
    connect(this,&MainWindow::RequestActRelocateSetEnabled,_actRelocate,&QAction::setEnabled);
    connect(_actRelocate,&QAction::triggered,this,&MainWindow::onActRelocateTriggered);

    QAction *_actEmergencyStop = new QAction(QIcon(":/img/ctrol_emergencyStop"),tr("Emergency Stop"),this);
    _actEmergencyStop->setStatusTip(tr("载体车复位"));
    _actEmergencyStop->setEnabled(false);
    ui->toolBar->addAction(_actEmergencyStop);
    connect(this,&MainWindow::RequestActEmergencyStopSetEnabled,_actEmergencyStop,&QAction::setEnabled);
    connect(_actEmergencyStop,&QAction::triggered,this,&MainWindow::onActEmergencyStopTriggered);

    ui->toolBar->addSeparator();

    QAction *_actToolBox = new QAction(QIcon(":/img/ctrol_toolBox"),tr("Tool Box"),this);
    _actToolBox->setStatusTip(tr("工具箱"));
    _actToolBox->setEnabled(false);
    ui->toolBar->addAction(_actToolBox);
    connect(this,&MainWindow::RequestActToolBoxSetEnabled,_actToolBox,&QAction::setEnabled);
    connect(_actToolBox,&QAction::triggered,this,&MainWindow::onActAroseToolBoxDialog);

}

/*
 *  打开串口后，系统将开启心跳包定时器
 * 在确保所有载体车都有心跳回馈之前，该函数将用户可以操作的界面进行
 * disable
 *
 */
void MainWindow::disableUserInterface()
{
    //toolbar
    emit RequestActEmergencyStopSetEnabled(false);
    emit RequestActToolBoxSetEnabled(false);


    //动作列表
    ui->LW_ActionSortcutList->setEnabled(false);
}

/*
 * 在判断所有载体车都有心跳包回馈后，在重新关开串口前，永久关闭心跳包定时器
 * 并用该函数使能用户可以操作的界面
 */

void MainWindow::enableUserInterface()
{
    //toolbar
    emit RequestActEmergencyStopSetEnabled(true);
    emit RequestActToolBoxSetEnabled(true);

    //动作列表
    ui->LW_ActionSortcutList->setEnabled(true);
}

void MainWindow::loadProjectFile(QUrl fileUrl)
{
    //从文件读取工程配置：RFID点，载体车

    //避免重复加载
    if(m_bIsProjectFileLoaded)
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("已加载文件"));
        return;
    }

    /*  RFID    */
    QList<QString> _rfidList = this->getFileRfidConfigList(fileUrl);
    if(_rfidList.isEmpty())
    {
        QMessageBox::critical(this,tr("加载文件失败"),tr("获得RFID列表空"));
        return;
    }

    /*  CARRIER */
    QList<QString> _carrierConfigList = this->getFileCarrierConfigList(fileUrl);
    if(_carrierConfigList.isEmpty())
    {
        QMessageBox::critical(this,tr("加载文件失败"),tr("获得载体车配置列表空"));
        return;
    }

    //给pathwayGV传入rfidList初始化RFID视图
    ui->GV_CarrierPathway->initPathwayRfid(_rfidList);

    //初始化载体车
    m_pCarrierManager = new CarrierManager();
    m_pCarrierManager->initLogicCarrier(_carrierConfigList,this);
    m_pCarrierManager->m_pCarrier->bandViewer(ui->TV_Carrier);
    ui->GV_CarrierPathway->initGraphicCarrier(_carrierConfigList);


    //已加载工程文件
    m_bIsProjectFileLoaded = true;
    emit RequestActStartLinkSetEnabled(true);
    ui->GV_CarrierPathway->setEnabled(true);
}

QList<QString> MainWindow::getFileRfidConfigList(QUrl fileUrl)
{
    QList<QString> _rfidList,_tempList;

    /*  文件确认    */
    if(fileUrl.fileName().right(3).compare(PROJECTFILE_SUFFIX))
    {
        //不为对应文件
        QMessageBox::critical(this,tr("Cannot Open file"),tr("非工程文件"));
        return _rfidList;
    }

    QFile _projectFile(fileUrl.toLocalFile());
    if(!_projectFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("无法打开文件"));
        _projectFile.close();
        return _rfidList;
    }

    /*  加载RFID点位    */
    int _iRfidNum = 0;
    _projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_RFID,_projectFile.readLine()))
        {
            _iRfidNum = QString(_projectFile.readLine()).toInt();
            break;
        }

        if(_projectFile.atEnd())
        {
            QMessageBox::critical(this,tr("错误的文件格式"),("未能正确加载坐标信息"));
            _projectFile.close();
            return _rfidList;
        }
    }

    for(int i = 0;i < _iRfidNum;i++)
    {
        if(_projectFile.atEnd())
        {
            QMessageBox::critical(this,tr("错误的文件格式"),("未能正确加载坐标信息"));
            _projectFile.close();
            return _rfidList;
        }

        QString _lineData = _projectFile.readLine();
        _lineData.trimmed();
        if(_lineData.isEmpty())
            break;

        QStringList _lineConfigList = _lineData.split(" ");
        if(_lineConfigList.count() != 3)
        {
            QMessageBox::critical(this,tr("Cannot Open File"),tr("无效的RFID点位格式"));
            _projectFile.close();
            return _rfidList;
        }

        QString _x = _lineConfigList.value(0);
        QString _y = _lineConfigList.value(1);
        QString _number = _lineConfigList.value(2);

        _tempList << QString(_x + " " + _y + " " + _number).trimmed();
    }

    _projectFile.close();
    _rfidList = _tempList;
    return _rfidList;
}

QList<QString> MainWindow::getFileCarrierConfigList(QUrl fileUrl)
{
    QList<QString> _carrierConfigList,_tempList;

    /*  文件确认    */
    if(fileUrl.fileName().right(3).compare(PROJECTFILE_SUFFIX))
    {
        //不为对应文件
        QMessageBox::critical(this,tr("Cannot Open file"),tr("非工程文件"));
        return _carrierConfigList;
    }

    QFile _projectFile(fileUrl.toLocalFile());
    if(!_projectFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this,tr("Cannot Open File"),tr("无法打开文件"));
        _projectFile.close();
        return _carrierConfigList;
    }

    /*  加载carrier配置 */
    int _iCarrierCount = 0;
    _projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_CARRIER,_projectFile.readLine()))
        {
            _iCarrierCount = QString(_projectFile.readLine()).toInt();
            break;
        }

        if(_projectFile.atEnd())
        {
            QMessageBox::critical(this,tr("错误的文件格式"),("未能正确加载载体车信息"));
            _projectFile.close();
            return _carrierConfigList;
        }
    }

    for(int i = 0;i < _iCarrierCount;i++)
    {
        if(_projectFile.atEnd())
        {
            QMessageBox::critical(this,tr("错误的文件格式"),("未能正确加载载体车信息"));
            _projectFile.close();
            return _carrierConfigList;
        }

        QString _lineData = _projectFile.readLine();
        _lineData.trimmed();
        if(_lineData.isEmpty())
            break;

        QStringList _lineConfigList = _lineData.split(" ");
        if(_lineConfigList.count() != 5)
        {
            QMessageBox::critical(this,tr("Cannot Open File"),tr("无效的载体车数据格式"));
            _projectFile.close();
            return _carrierConfigList;
        }

        QString _strModel = QString(_lineConfigList.value(0));
        QString _strPos = QString(_lineConfigList.value(1));
        QString _strSpeed = QString(_lineConfigList.value(2));
        QString _strStatus = QString(_lineConfigList.value(3));
        QString _strEnabled = QString(_lineConfigList.value(4));

        _tempList << QString(_strModel + " " + _strPos + " " + _strSpeed + " "
                               + _strStatus + " " + _strEnabled).trimmed();
    }

    _projectFile.close();
    _carrierConfigList = _tempList;
    return _carrierConfigList;
}

void MainWindow::componentInit()
{
    //载体车管理器
    connect(m_pCarrierManager,&CarrierManager::RequestPrintDebugMessage,this,&MainWindow::printMessage);
    connect(m_pCarrierManager,&CarrierManager::RequestAfterAllCarrierTouched,this,&MainWindow::OnEnsureAllCarrierAlive);
    connect(m_pCarrierManager,&CarrierManager::RequestUpdateGraphicCarrier,ui->GV_CarrierPathway,&PathwayGV::onUpdateGraphicCarrier);
    connect(m_pCarrierManager,&CarrierManager::RequestSendtoRealCarrier,m_pStationPort,&StationPort::SendPackageData);
    connect(m_pCarrierManager,&CarrierManager::RequestActionplayerDoNext,m_pActionPlayer,&ActionPlayer::doNextStep);
    connect(m_pCarrierManager,&CarrierManager::RequestActionplayerStop,m_pActionPlayer,&ActionPlayer::stopActionPlayer);

    connect(m_pStationPort,&StationPort::RequestSetCarrierStatus,m_pCarrierManager,&CarrierManager::OnRealCarrierHeartbeatBack);

    connect(m_pRealActionActuator,&RealActionActuator::RequestStartPlayingAction,m_pCarrierManager,&CarrierManager::OnStartPlayingAction);

}

void MainWindow::componentDeinit()
{
    //载体车管理器
    disconnect(m_pCarrierManager,&CarrierManager::RequestPrintDebugMessage,this,&MainWindow::printMessage);
    disconnect(m_pCarrierManager,&CarrierManager::RequestAfterAllCarrierTouched,this,&MainWindow::OnEnsureAllCarrierAlive);
    disconnect(m_pCarrierManager,&CarrierManager::RequestUpdateGraphicCarrier,ui->GV_CarrierPathway,&PathwayGV::onUpdateGraphicCarrier);
    disconnect(m_pCarrierManager,&CarrierManager::RequestSendtoRealCarrier,m_pStationPort,&StationPort::SendPackageData);
    disconnect(m_pCarrierManager,&CarrierManager::RequestActionplayerDoNext,m_pActionPlayer,&ActionPlayer::doNextStep);
    disconnect(m_pCarrierManager,&CarrierManager::RequestActionplayerStop,m_pActionPlayer,&ActionPlayer::stopActionPlayer);

    disconnect(m_pStationPort,&StationPort::RequestSetCarrierStatus,m_pCarrierManager,&CarrierManager::OnRealCarrierHeartbeatBack);

    disconnect(m_pRealActionActuator,&RealActionActuator::RequestStartPlayingAction,m_pCarrierManager,&CarrierManager::OnStartPlayingAction);
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

void MainWindow::OnEnsureAllCarrierAlive()
{
    this->enableUserInterface();
}

void MainWindow::OnSetStationPort(QString portname)
{
    m_pStationPort->setPort(portname);
}

void MainWindow::printMessage(QString str)
{
    QTime _NowTime = QTime::currentTime();
    str.prepend(QString(_NowTime.toString("hh-mm-ss:  ")));

    ui->PTE_MessageWindow->appendPlainText(str);
    ui->PTE_MessageWindow->verticalScrollBar()->setValue(ui->PTE_MessageWindow->verticalScrollBar()->maximumHeight());
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

void MainWindow::on_PTE_MessageWindow_customContextMenuRequested(const QPoint &pos)
{
    Q_UNUSED(pos);

    if(QMessageBox::Ok == QMessageBox::information(this,tr("清除调试信息"),tr("是否要清除调试信息？"),QMessageBox::Ok,QMessageBox::Cancel))
    {
        ui->PTE_MessageWindow->clear();
    }

    return;
}

void MainWindow::onActStartLinkTriggered()
{
    if(m_pStationPort->startConnect() == 1)
    {
        //端口打开成功
        QMessageBox::information(this,QString(tr("SerialPort OK")),tr("端口打开成功"));
        this->printMessage(tr("--打开端口成功--"));

        //界面处理
        emit RequestActStartLinkSetEnabled(false);
        emit RequestActStopLinkSetEnabled(true);
        emit RequestActRelocateSetEnabled(true);
        emit RequestActEmergencyStopSetEnabled(false);
        this->disableUserInterface();

        //组件加载
        this->componentInit();

        //carriermanager 开始接触
        m_pCarrierManager->startTouchRealCarrier();

    }
    else
    {
        //端口打开失败
        QMessageBox::warning(this,tr("SerialPort Error"),tr("端口打开失败"));
        this->printMessage(tr("-!端口打开失败!-"));
        return;
    }
}

void MainWindow::onActStopLinkTriggered()
{
    //关闭端口
    if(m_pStationPort->isOpen())
    {
        m_pStationPort->stopConnect();
        this->printMessage(tr("--成功关闭端口--"));
    }

    //界面处理
    emit RequestActStartLinkSetEnabled(true);
    emit RequestActStopLinkSetEnabled(false);
    emit RequestActRelocateSetEnabled(false);
    emit RequestActEmergencyStopSetEnabled(false);
    this->disableUserInterface();

    //组件卸载
    this->componentDeinit();
}

void MainWindow::onActRelocateTriggered()
{
    m_pCarrierManager->startRealCarrierRelocate();
}

void MainWindow::onActEmergencyStopTriggered()
{
    m_pCarrierManager->startRealCarrierEmergencyStop();
}

void MainWindow::onActAroseSettingDialog()
{
    m_pSettingDialog->open();
}

void MainWindow::onActAroseToolBoxDialog()
{
    m_pToolBoxDialog->open();
}
