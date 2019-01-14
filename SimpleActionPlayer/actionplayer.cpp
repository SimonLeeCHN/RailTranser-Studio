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

int iGroupCmdNum = 0;

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
    iGroupCmdNum = 0;
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

        //抛弃注释部分
        if(tempStr.contains(PLAYER_COMMENT,Qt::CaseInsensitive))
        {
            int _pos = tempStr.indexOf(PLAYER_COMMENT);
            tempStr = tempStr.left(_pos);
        }

        //去除字符串首尾空格，换行符
        tempStr = tempStr.trimmed();

        //非空，则加入list
        if(!tempStr.isEmpty())
        {
            m_lCmdList << tempStr;
        }

    }

    tempFile.close();

    return true;

}

bool ActionPlayer::loadActionList(QList<QString> listName)
{
    //加载初始化
    iGroupCmdNum = 0;
    m_iCmdPointer = 0;
    m_lCmdList.clear();
    m_iPlayerStatus = PLAYERSTU_STANDBY;

    //将输入list载入至内部list
    while(!listName.isEmpty())
    {
        QString _tempStr = listName.first();
        listName.removeFirst();

        //抛弃注释部分
        if(_tempStr.contains(PLAYER_COMMENT,Qt::CaseInsensitive))
        {
            int _pos = _tempStr.indexOf(PLAYER_COMMENT);
            _tempStr = _tempStr.left(_pos);
        }

        //去除字符串首尾空格
        _tempStr = _tempStr.trimmed();

        //非空，则加入list
        if(!_tempStr.isEmpty())
        {
            m_lCmdList << _tempStr;
        }
    }

    return true;

}

void ActionPlayer::setActuator(ActionActuator *acac)
{
    m_pActuator = acac;
    acac->m_pParentPlayer = this;
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

    QStringList tStrList = m_lCmdList.value(m_iCmdPointer).split(" ");
    m_iCmdPointer++;

    switch(map_StrcmdToCode[tStrList[0]])
    {
        case CMD_STA:
        {
            qDebug()<<"play-cmd: STA";
            emit RequestPrintMessage("STA");
            m_iPlayerStatus = PLAYERSTU_PLAYING;

            /*
             *  自调用，不使用this->doNextStep这种方法，避免递归深度过深
             *  使用单定时触发器，定时极短时间后触发doNextStep
             */
            //this->doNextStep();
            QTimer::singleShot(10,this,&ActionPlayer::doNextStep);

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
            QList<QString> _cmdList;

            //先将当前MOV命令加入列表
            _cmdList << QString(tStrList.value(1) + " " +
                                tStrList.value(2) + " " +
                                tStrList.value(3));

            //若有需要组合执行的命令
            if(iGroupCmdNum > 0)
            {
                //已将当前MOV命令加入列表
                iGroupCmdNum--;

                while(1)
                {
                    QStringList _strList = m_lCmdList.value(m_iCmdPointer).split(" ");

                    //有需要组合执行的命令，且同为MOV，则一并加入list发送
                    if((iGroupCmdNum > 0) && (_strList.value(0) == "MOV"))
                    {
                        _cmdList << QString(_strList.value(1) + " " +
                                            _strList.value(2) + " " +
                                            _strList.value(3));

                        iGroupCmdNum--;
                        m_iCmdPointer++;
                    }
                    else
                        break;
                }
            }

            //发送命令列表
            this->m_pActuator->generateMotion(_cmdList);
            this->m_iPlayerStatus = PLAYERSTU_WAITING;

            //若仍有需要组合执行的指令，自调用
            if(iGroupCmdNum > 0)
            {
                QTimer::singleShot(10,this,&ActionPlayer::doNextStep);
            }

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

            /*
             *  自调用，不使用this->doNextStep这种方法，避免递归深度过深
             *  使用单定时触发器，定时极短时间后触发doNextStep
             */
            //this->doNextStep();
            QTimer::singleShot(10,this,&ActionPlayer::doNextStep);

            break;
        }
        case CMD_JMP:
        {
            qDebug()<<"play-cmd: JMP "<<tStrList[1];

            //提示信息
            QString strMessage = "JMP " + tStrList[1];
            emit RequestPrintMessage(strMessage);

            //修改指针
            m_iCmdPointer = tStrList.value(1).toInt();

            /*
             *  自调用，不使用this->doNextStep这种方法，避免递归深度过深
             *  使用单定时触发器，定时极短时间后触发doNextStep
             */
            //this->doNextStep();
            QTimer::singleShot(10,this,&ActionPlayer::doNextStep);

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

void ActionPlayer::stopActionPlayer()
{
    m_iPlayerStatus = PLAYERSTU_STOP;
    m_iCmdPointer = 0;
    m_lCmdList.clear();
}
