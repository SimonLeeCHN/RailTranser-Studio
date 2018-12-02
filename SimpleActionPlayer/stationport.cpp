#include "StationPort.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QCoreApplication>
#include <QTime>
#include <QTimer>
#include <QThread>
#include "QMessageBox"
#include "mainwindow.h"

#define PACKAGE_HEAD    0xFE
#define PACKAGE_TAIL    0xFF

#define PACKAGE_CMD_POS     2
#define PACKAGE_HEARTBEAT_LEN               15  //去掉了包头
#define PACKAGE_HEARTBEAT_CARNUM_POS        5
#define PACKAGE_HEARTBEAT_CARSTATUS_POS     6
#define PACKAGE_HEARTBEAT_POSHEAD_POS       7
#define PACKAGE_HEARTBEAT_GOALHEAD_POS      11

#define PACKAGE_CONFIG_LEN                 9
#define PACKAGE_CONFIG_CARNUM_POS          5
#define PACKAGE_CONFIG_DATALENG            5

#define ADDRESS_CARRIER_BASE    0x10
#define DATALEN_FIXED_VALUE     4

QByteArray HexStringToByteArray(QString HexString)
{
    bool ok;
    QByteArray ret;
    HexString = HexString.trimmed();
    HexString = HexString.simplified();
    QStringList sl = HexString.split(" ");

    foreach (QString s, sl)
    {
        if(!s.isEmpty())
        {
            char c = s.toInt(&ok,16)&0xFF;
            if(ok)
            {
                ret.append(c);
            }
        }
    }
    return ret;
}

void TransferDataBeforeEmit(QByteArray &data)
{
    //先 FE -> FEFC
    int replaceIndex = 4;
    while(1)
    {
        replaceIndex = data.indexOf(QByteArray("\xFE"),replaceIndex);
        if(replaceIndex == -1)
            break;
        else
        {
            data.replace(replaceIndex,1,QByteArray("\xFE\xFC"));
            replaceIndex += 2;
        }
    }

    //再 FF -> FEFD
    replaceIndex = 4;
    while(1)
    {
        replaceIndex = data.indexOf(QByteArray("\xFF"),replaceIndex);
        if((replaceIndex == -1) || (replaceIndex == (data.count() - 1)))
            break;
        else
        {
            data.replace(replaceIndex,1,QByteArray("\xFE\xFD"));
            replaceIndex += 2;
        }
    }
}

void TransferDataAfterRecive(QByteArray &data)
{
    //先 FEFD->FF
    int replaceIndex = 4;
    while(1)
    {
        replaceIndex = data.indexOf(QByteArray("\xFE\xFD"),replaceIndex);
        if(replaceIndex == -1)
            break;
        else
        {
            data.replace(replaceIndex,2,QByteArray("\xFF"));
            replaceIndex += 1;
        }
    }

    //再 FEFC->FE
    replaceIndex = 4;
    while(1)
    {
        replaceIndex = data.indexOf(QByteArray("\xFE\xFC"),replaceIndex);
        if(replaceIndex == -1)
            break;
        else
        {
            data.replace(replaceIndex,2,QByteArray("\xFE"));
            replaceIndex += 1;
        }
    }
}

StationPort::StationPort()
{
    //参数初始化
    portSettings.baudrate = QSerialPort::Baud115200;
    portSettings.databits = QSerialPort::Data8;
    portSettings.parity = QSerialPort::NoParity;
    portSettings.stopbits = QSerialPort::OneStop;
    portSettings.flowcontrol = QSerialPort::NoFlowControl;

    connect(this , &StationPort::readyRead , this , &StationPort::OnStationPortDataCome);

    //线程初始化
//    DataSendWorker* pDataSendWorker = new DataSendWorker(this);
//    pDataSendWorker->moveToThread(&m_tDataSendThread);
//    connect(&m_tDataSendThread,&QThread::finished,pDataSendWorker,&QObject::deleteLater);
//    connect(this,&StationPort::RequestThreadSendData,pDataSendWorker,&DataSendWorker::PackAndSendData);
//    m_tDataSendThread.start();

}

StationPort::~StationPort()
{
    m_tDataSendThread.quit();
    m_tDataSendThread.wait();

    disconnect(this,&StationPort::readyRead , this , &StationPort::OnStationPortDataCome);
}

int StationPort::startConnect(QString portname)
{
    if(this->isOpen())
        this->close();

    if(portname != "")
    {
        portSettings.portname = portname;
        this->setPortName(portSettings.portname);
        this->setBaudRate(portSettings.baudrate);
        this->setDataBits(portSettings.databits);
        this->setParity(portSettings.parity);
        this->setStopBits(portSettings.stopbits);
        this->setFlowControl(portSettings.flowcontrol);

        if(this->open(QIODevice::ReadWrite))
        {
            return 1;
        }else
        {
            return 0;
        }
    }else
    {
        return -1;
    }
}

bool StationPort::stopConnect()
{
    if(this->isOpen())
        this->close();

    return true;
}

void StationPort::packetPackage(QList<QByteArray> &list,int port)
{
    //接受列表化的数据包体，为每一项按照协议加入包头
    //第一个为要发向的车辆号，后面紧跟的按协议来的数据
    //注意，对于目标点运动指令来说，实际上list中每行开头有两车辆号
    for(int i = 0;i < list.count();i++)
    {
        //提取车辆号并移除
        int carNum = list[i].at(0);
        list[i].remove(0,1);

        int dataLen = list.value(i).length();

        //添加地址
        list[i].prepend(char(0));
        list[i].prepend(char(carNum + ADDRESS_CARRIER_BASE));

        //转译:包长度不用管.数据、地址、端口号中 FF->FE FD ,FE->FE FC
        //因为端口号避免了问题，在添加地址后进行转译即可
        TransferDataBeforeEmit(list[i]);

        //添加端口 *2
        list[i].prepend(port);
        list[i].prepend(port);
        //添加数据长度
        list[i].prepend(dataLen + DATALEN_FIXED_VALUE);
        //添加包头
        list[i].prepend(PACKAGE_HEAD);

        //添加包尾
        list[i].append(PACKAGE_TAIL);
    }
}

void StationPort::SplitPortdataPackage()
{
    //将串口读来的数据进行分包加入list
    //去掉了包头包尾

    QByteArray packageTail("\xFF",1);
    int index = 0,packageTailLen = packageTail.length(),packageHeadLen = 1;
    int from = packageHeadLen;
    bool isPackdataCame = false;    //缓冲区内是否有数据包存在

    while((index = m_RawData.indexOf(packageTail,from)) != -1)
    {
        m_List_PackageData << m_RawData.mid(from,index - from);

        //翻译:包长度不用管.数据、地址、端口号中 FE FD->FF ,FE FC->FE
        TransferDataAfterRecive(m_List_PackageData.last());

        from = index + packageHeadLen + packageTailLen;

        isPackdataCame = true;
    }

    if(isPackdataCame)
        m_RawData = m_RawData.right(m_RawData.length() - (from - packageTailLen));
    else
        m_RawData.clear();
}

void StationPort::IdentifyListCommand()
{
    int count = m_List_PackageData.count();
    for(int index = 0;index < count;index++)
    {
        //防止错误
        if(!(m_List_PackageData.first().count() > PACKAGE_CMD_POS))
        {
            m_List_PackageData.removeFirst();
            continue;
        }

        unsigned char port = (m_List_PackageData.first())[PACKAGE_CMD_POS];
        switch (port)
        {
            case PORT_HEARTBEAT_BACK:
            {
                //包长度检查
                if(m_List_PackageData.first().length() == PACKAGE_HEARTBEAT_LEN)
                {
                    //发射信号，返回车辆号,状态编码,目前位置,目标位置
                    int carNum = (m_List_PackageData.first())[PACKAGE_HEARTBEAT_CARNUM_POS];
                    int status = (m_List_PackageData.first())[PACKAGE_HEARTBEAT_CARSTATUS_POS];
                    int pos = 0;
                    for(int i = 0;i < 4;i++)
                        pos |= ((unsigned char)(m_List_PackageData.first().at(PACKAGE_HEARTBEAT_POSHEAD_POS + i)) << (8*(3 - i)));

                    int goal = 0;
                    for(int i = 0;i < 4;i++)
                        goal |= ((unsigned char)(m_List_PackageData.first().at(PACKAGE_HEARTBEAT_GOALHEAD_POS + i)) << (8*(3 - i)));
                    emit RequestSetCarrierStatus(carNum,status,pos,goal);
                }
                else
                {
                    QString tempStr = m_List_PackageData.first().toHex();
                    tempStr.prepend(QString("错误的心跳包长度:  "));
                    emit RequestPrintMessage(tempStr);
                }

                break;
            }
            case PORT_CONFIG_BACK:
            {
                //包长度检查
                if(m_List_PackageData.first().length() == PACKAGE_CONFIG_LEN)
                {
                    //车辆号 速度 方向 速度曲线 使能
                    QByteArray carProfile = m_List_PackageData.first().mid(PACKAGE_CONFIG_CARNUM_POS,PACKAGE_CONFIG_DATALENG);

                    emit RequestSetCarrierProfile(carProfile);
                }
                else
                {
                    QString tempStr = m_List_PackageData.first().toHex();
                    tempStr.prepend(QString("错误的配置包长度:  "));
                    emit RequestPrintMessage(tempStr);
                }

                break;
            }
            default:
            {
                QString tempStr = m_List_PackageData.first().toHex();
                tempStr.prepend(QString("错误的端口:  "));
                emit RequestPrintMessage(tempStr);

                break;
            }
        }
        m_List_PackageData.removeFirst();
    }
}




/*      SLOT        */
void StationPort::OnStationPortDataCome()
{
    QByteArray tempData = this->readAll();

    //将本次读来的数据加入缓冲区，以防一次读不全包
    m_RawData.append(tempData);

#if SHOW_SERIALRECV
    QString tempStr = tempData.toHex();
    tempStr.prepend("REC: ");
    emit RequestPrintMessage(tempStr);
#endif

    //分析包并归类、响应
    SplitPortdataPackage();
    IdentifyListCommand();

}

void StationPort::SendPackageData(QList<QByteArray> list,int port)
{
    //数据打包
    packetPackage(list,port);

    //将列表中的数据发送
    for(int i = 0; i < list.count();i++)
    {

#if SHOW_SERIALSEND
        QString tempstr = list[i].toHex();
        tempstr.prepend("SEND: ");
        emit RequestPrintMessage(tempstr);
#endif

        this->write(list[i]);
    }

    /*
    //交给发送线程进行打包和发送
    packetPackage(list,port);
    emit RequestThreadSendData(list);
    */
}


///////////////////////////////////////////////////////////////////
void DataSendWorker::PackAndSendData(QList<QByteArray> list)
{
    /*
    for(int i = 0; i < list.count();i++)
    {
        m_pParentStationPort->write(list[i]);
    }

    //延迟
    QTime _Timer=QTime::currentTime();
    QTime _NowTimer;
    do{
        _NowTimer=QTime::currentTime();
    }while(_Timer.msecsTo(_NowTimer) <= PORT_DATASEND_DEY);
    */
}


