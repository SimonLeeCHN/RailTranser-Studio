#include "actionplayer.h"
#include <QFile>
#include <QDebug>
#include <QTime>
#include <QTimer>
#include <QtSingleApplication>
#include <QFileInfo>

QMap<QString,int> map_StrcmdToCode =
{
    {"STA",1},
    {"END",2},
    {"MOV",3},
    {"DEY",4},
    {"CMG",5}
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
    m_iPlayerStatus = PLAYERSTU_STANDBY;
    m_iCmdPointer = 0;
    m_lCmdList.clear();
}

bool ActionPlayer::isPlaying()
{
    if(PLAYERSTU_PLAYING == this->m_iPlayerStatus)
        return true;
    else
        return false;
}

bool ActionPlayer::isWaitingTriger()
{
    if(PLAYERSTU_WAITING == this->m_iPlayerStatus)
        return true;
    else
        return false;
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

    QStringList tStrList = m_lCmdList.at(m_iCmdPointer).split(" ");
    m_iCmdPointer++;

    m_iPlayerStatus = PLAYERSTU_PLAYING;

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

            //发送移动某块车辆指令
            QList<QString> tempList ;
            QString tempStr;
            tempStr = tStrList[1] + " " + tStrList[2] + " " + tStrList[3];
            tempList<<tempStr;

            //提示消息
            QString strMessage = "MOV " + tempStr;
            emit RequestPrintMessage(strMessage);

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

            //TODO:使得延迟一段时间
            QTimer::singleShot((tStrList[1]).toInt() * 1000,this,doNextStep);

            break;
        }
        case CMD_CMG:
        {
            qDebug()<<"play-cmd: CMG";
            emit RequestPrintMessage("CMG");

            //将后x个MOV指令一同打包发送
            QList<QString> tempList;
            for(int i = 0; i < (tStrList[1]).toInt() ; i++)
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

            this->m_pActuator->generateMotion(tempList);
            this->m_iPlayerStatus = PLAYERSTU_WAITING;
            emit RequestTriggerAfterCarrierStandby();

            break;
        }
        default:
        {
            qDebug()<<"play-Error cmd!";
            this->stopActionPlayer();

            return;
        }

    }

}
