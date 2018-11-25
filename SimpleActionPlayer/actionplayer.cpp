#include "actionplayer.h"
#include <QFile>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QFileInfo>

#define CMD_STA 1
#define CMD_END 2
#define CMD_MOV 3
#define CMD_DEY 4
#define CMD_CMG 5
#define CMD_JMP 6

int iGroupCmdNum = 1;

QMap<QString,int> map_StrcmdToCode =
{
    {"STA",1},
    {"END",2},
    {"MOV",3},
    {"DEY",4},
    {"CMG",5},
    {"JMP",6}
};

ActionPlayer::ActionPlayer()
{

}
ActionPlayer::~ActionPlayer()
{

}

bool ActionPlayer::loadActionFile(QString fileName)
{
    QFile tempFile;

    //判断文件是否存在
    QFileInfo fileInfo(fileName);
    if(!fileInfo.isFile())
    {
        qDebug()<<"Is not file!";
        return false;
    }

    //加载初始化
    m_iCmdPointer = 0;
    m_lCmdList.clear();
    m_iPlayerStatus = PLAYERSTU_STANDBY;

    //打开文件
    tempFile.setFileName(fileName);
    if(!tempFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"can not open action file!";
        return false;
    }

    //载入至list
    while(!tempFile.atEnd())
    {
        QString tempStr = tempFile.readLine();
        tempStr.chop(1);
        if(!tempStr.isEmpty())
            m_lCmdList << tempStr;
    }

    tempFile.close();

    return true;

}

void ActionPlayer::setActuator(ActionActuator *acac)
{
    m_pActuator = acac;
    acac->m_pParentPlayer = this;
}

void ActionPlayer::stopActionPlayer()
{
    m_iPlayerStatus = PLAYERSTU_STOP;
    m_iCmdPointer = 0;
    m_lCmdList.clear();
}

int ActionPlayer::getPlayerStatus()
{
    return m_iPlayerStatus;
}

void ActionPlayer::doNextStep()
{
    //操作指令-操作自身指针的指令，在自己体内进行解码完成
    //命令指令-对载体车进行操作的指令，需要一起执行的打包list发给actuator执行

    if(m_pActuator == NULL)
    {
        qDebug()<<"PLAYER_ACTUATOR NONE";
        return;
    }

    if(m_iPlayerStatus == PLAYERSTU_STOP)
    {
        emit RequestPrintMessage(QString(tr("Player已被停止")));
        return;
    }

    m_iPlayerStatus = PLAYERSTU_PLAYING;

    QStringList tStrList = m_lCmdList.at(m_iCmdPointer).split(" ");
    m_iCmdPointer++;

    switch(map_StrcmdToCode[tStrList[0]])
    {
        case CMD_STA:
        {
            qDebug()<<"play-cmd: STA";
            emit RequestPrintMessage("STA");
            m_iPlayerStatus = PLAYERSTU_PLAYING;

            this->doNextStep();

            break;
        }
        case CMD_END:
        {
            qDebug()<<"play-cmd: END";
            emit RequestPrintMessage("END");

            m_iPlayerStatus = PLAYERSTU_STANDBY;

            break;
        }
        case CMD_MOV:
        {
            qDebug()<<"play-cmd: MOV ";

            //使指针指向当前MOV指令
            m_iCmdPointer--;

            QList<QString> tempList;
            for(int i = 0;i < iGroupCmdNum;i++)
            {
                QString tempStr;
                QStringList tempStrList = m_lCmdList.at(m_iCmdPointer).split(" ");

                tempStr = tempStrList[1] + " " + tempStrList[2] + " " + tempStrList[3];
                tempList<<tempStr;

                m_iCmdPointer++;

                //提示消息
                QString strMessage = "MOV " + tempStr;
                emit RequestPrintMessage(strMessage);
            }

            iGroupCmdNum = 1;

            this->m_pActuator->generateMotion(tempList);
            this->m_iPlayerStatus = PLAYERSTU_WAITING;

            emit RequestTriggerAfterCarrierStandby();

            break;
        }
        case CMD_DEY:
        {
            qDebug()<<"play-cmd: DEY "<<tStrList[1]<<" s";

            //提示消息
            QString strMessage = "DEY " + tStrList[1] + " s";
            emit RequestPrintMessage(strMessage);

            //使得延迟一段时间
            QTimer::singleShot((tStrList[1]).toInt() * 1000,this,&ActionPlayer::doNextStep);

            break;
        }
        case CMD_CMG:
        {
            qDebug()<<"play-cmd: CMG";

            iGroupCmdNum = (tStrList[1]).toInt();

            emit RequestPrintMessage(QString(tr("CMG %1").arg(iGroupCmdNum)));
            this->doNextStep();

            break;
        }
        case CMD_JMP:
        {
            qDebug()<<"play-cmd: JMP "<<tStrList[1];

            //提示信息
            QString strMessage = "JMP " + tStrList[1];
            emit RequestPrintMessage(strMessage);

            //修改指针
            m_iCmdPointer = tStrList.at(1).toInt();
            this->doNextStep();

            break;
        }
        default:
        {
            qDebug()<<"play-Error cmd!";
            this->stopActionPlayer();

            break;
        }

    }
    return;

}
