#include "carriermanager.h"
#include "actionactuator.h"
#include "qtsingleapplication.h"
#include <QTimer>

CarrierManager::CarrierManager()
{
    //关联pollingTimer超时定时器槽
    connect(&m_pollingTimer,&QTimer::timeout,this,&CarrierManager::OnPollingTimerOuttime);

    //置初始UNTOUCHED状态
    m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_UNTOUCHED;
}

CarrierManager::~CarrierManager()
{
    if(m_pCarrier != NULL)
        delete m_pCarrier;
}

void CarrierManager::initLogicCarrier(QList<QString> profileList, QObject *parent)
{
    m_pCarrier = new Carrier(profileList,parent);
}

/*
 *  进行载体车接触时，需要先将所有载体车的状态设置为丢失
 *  避免用户关闭又再次打开端口，自动调用载体车接触检测函数进行接触时
 *  上次已经接触过的状态影响再次的接触
 */
void CarrierManager::startTouchRealCarrier()
{
    //先将载体车的状态全设置为丢失
    m_pCarrier->setAllLogicCarrierStatusSame("丢失");

    //再开启接触轮询
    m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_UNTOUCHED;
    m_iPollingCarrierNumber = 1;
    inMotionPolling();
}

void CarrierManager::startRealCarrierRelocate()
{
    if(!(m_pCarrier->isAllLogicCarrierStatusSame("运动中")))
    {
        QList<QByteArray> _tempList;

        for(int _index = 0;_index < m_pCarrier->m_iCarrierNum;_index++)
        {
            QByteArray _tempArray;

            //供stationport使用的车辆号   复位指令协议数据
            _tempArray.append(_index+1).append(2);

            _tempList << _tempArray;
        }

        //发送复位指令
        emit RequestSendtoRealCarrier(_tempList,PORT_CONTROL_SEND);

        //停止actionplayer
        emit RequestActionplayerStop();

        //复位运动轮询
        m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_RELOCATING;
        m_iPollingCarrierNumber = 1;
        inMotionPolling();

    }
    else
    {
        QString _str = "载体车在运动中无法复位";
        emit RequestPrintDebugMessage(_str);
    }
}

void CarrierManager::startRealCarrierEmergencyStop()
{
    QList<QByteArray> _tempList;
    for(int _index = 0;_index < m_pCarrier->m_iCarrierNum;_index++)
    {
        QByteArray _tempArray;

        //供stationport使用的车辆号    急停指令协议数据
        _tempArray.append(_index + 1).append(1);

        _tempList << _tempArray;
    }

    //置CM为停止状态
    m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_STOPED;

    //发送急停指令
    emit RequestSendtoRealCarrier(_tempList,PORT_CONTROL_SEND);

    //停止actionplayer
    emit RequestActionplayerStop();
}

/*
 *  载体车运动中轮询：
 *  向x号车发送查询包
 *  开启polling超时定时器，若超时，重新调用inMotionPolling补发x号车查询包
 *
 *  在OnRealCarrierHeartbeatBack中，判断若CM为INMOTION状态，关polling超时定时器,
 *  若返回目标点位和logicCarrier目标点位不符，重发目标点运动指令
 *  若当前点与目标点重回，且所有载体车都处于待机状态，触发actionplayer播放下一条指令
 */
void CarrierManager::inMotionPolling()
{
    if(m_iCarrierManagerStatus == CARRIERMANAGER_STATUS_STOPED)
        return;

    //跳过未使能车辆
    while(!(m_pCarrier->isCarrierEnabled(m_iPollingCarrierNumber)))
    {
        qDebug()<<QString::number(m_iPollingCarrierNumber) + " 未使能,跳过";
        if(m_pCarrier->isCarrierNumberLegal(m_iPollingCarrierNumber + 1))
            m_iPollingCarrierNumber++;
        else
            m_iPollingCarrierNumber = 1;
    }

    //数据有效性检查
    if(m_pCarrier->isCarrierNumberLegal(m_iPollingCarrierNumber))
    {
        QList<QByteArray> _heartbeatList;
        QByteArray _tempBytearray;

        //先为stationport加入车辆号标识，再按照心跳包查询协议加入车辆号
        _tempBytearray.append(m_iPollingCarrierNumber).append(m_iPollingCarrierNumber);
        _heartbeatList << _tempBytearray;

        //向x号车发送
        emit RequestSendtoRealCarrier(_heartbeatList,PORT_HEARTBEAT_SEND);

        //重新开启polling超时定时器
        m_pollingTimer.stop();
        m_pollingTimer.start(CARRIERMANAGER_POLLING_OUTTIME);
    }
    else
    {
        qDebug()<<"CarrierManager::inMotionPolling - m_iPollingCarrierNumber检查错误";
    }
}

/*              SLOT            */

/*
 *  polling定时器超时，输出调试语句，重新调用inMotionPolling
 */
void CarrierManager::OnPollingTimerOuttime()
{
    //输出调试语句
    QString _str = "Polling超时 ：" + QString::number(m_iPollingCarrierNumber) + " 号车,再次polling...";
    emit RequestPrintDebugMessage(_str);

    //重新调用inMotionPolling
    this->inMotionPolling();
}

/*
 *  在用户点击播放运动列表后，actionplayer读入casf，actionactuator展开列表后，传入
 *  调用该函数。
 *  actionList  ：   车辆号(1) 目标点(4)
 *  函数内更新LogicCarrier的目标点，调用stationport发送目标点运动指令，开启运动中轮询
 *
 */
void CarrierManager::OnStartPlayingAction(QList<QByteArray> actionList)
{
    //更新LogicCarrier目标点
    m_pCarrier->updateLogicCarrierGoal(actionList);

    //为PacketPackage，在每行头添加车辆号
    for(int _index = 0;_index < actionList.count();_index++)
    {
        QByteArray _tempLine = actionList.value(_index);
        char _carNum = _tempLine.at(0);
        _tempLine.prepend(_carNum);
        actionList[_index] = _tempLine;
    }

    //stationport发送目标点运动指令
    emit RequestSendtoRealCarrier(actionList,PORT_GOALMOVE_SEND);

    //置CM为INMOTION状态
    m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_INMOTION;

    //开启运动中轮询
    m_iPollingCarrierNumber = 1;
    this->inMotionPolling();
}

/*
 *  更新logicCarrier状态和当前位置
 *
 *  关polling超时定时器
 *  若CM为INMOTION状态，
 *  若返回目标点位和logicCarrier目标点位不符，重发目标点运动指令,重新polling x号车
 *  若当前点与目标点重回，且所有载体车都处于待机状态，触发actionplayer播放下一条指令
 *  若都不是，polling x+1
 *
 */
void CarrierManager::OnRealCarrierHeartbeatBack(int carNumber, int carStatus, int carNowPos, int carGoal)
{
    //数据有效性检查
    if(!(m_pCarrier->isCarrierNumberLegal(carNumber)))
    {
        qDebug()<<"CarrierManager::OnRealCarrierHeartbeatBack - carNumber检查错误";
        return;
    }
    if(!(m_pCarrier->isCarrierStatusLegal(carStatus)))
    {
        qDebug()<<"CarrierManager::OnRealCarrierHeartbeatBack - carStatus检查错误";
        return;
    }

    //关polling定时器
    m_pollingTimer.stop();

    //更新logiccarrier状态和当前位置
    m_pCarrier->updateLogicCarrierStatus(carNumber,carStatus,carNowPos);

    //更新graphiccarrier
    emit RequestUpdateGraphicCarrier(carNumber,carStatus,carNowPos);

    switch (m_iCarrierManagerStatus)
    {
        case CARRIERMANAGER_STATUS_INMOTION:
        {
            //比较返回目标点位和logicCarrier目标点位
            int _logicGoal = m_pCarrier->getSpecificLogicCarrierGoal(carNumber);
            if(!(_logicGoal == carGoal))
            {
                /*重新发送目标点运动指令*/
                QString _str = "重新发送目标点运动指令 "+QString::number(carNumber)+" 至 "+QString::number(_logicGoal);
                emit RequestPrintDebugMessage(_str);

                QByteArray _tempArray;

                //添加stationport要求的车辆号
                _tempArray.append(carNumber);

                //添加数据包要求的车辆号
                _tempArray.append(carNumber);

                //注意目标点占位4
                setPos _tempGoal;
                _tempGoal.integer = _logicGoal;
                QByteArray _baGoal;
                _baGoal.append(_tempGoal.uch[3]).append(_tempGoal.uch[2]).append(_tempGoal.uch[1]).append(_tempGoal.uch[0]);
                _baGoal = _baGoal.right(4);

                _tempArray.append(_baGoal);

                QList<QByteArray> _tempList;
                _tempList << _tempArray;

                //重新发送目标点运动指令
                emit RequestSendtoRealCarrier(_tempList,PORT_GOALMOVE_SEND);

                //重新polling
                this->inMotionPolling();
            }

            //是否所有载体车当前点等于目标点
            if(m_pCarrier->isAllLogicCarrierMotionAtPoint())
            {
                //是否所有载体车都处于待机状态
                if(m_pCarrier->isAllLogicCarrierStatusSame("待机"))
                {
                    //输出调试语句
                    emit RequestPrintDebugMessage("**载体车运动到位，触发下一语句**");

                    m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_STANDBY;

                    //actionplayer donextstep
                    emit RequestActionplayerDoNext();

                    return;
                }
            }

            //polling x+1
            m_iPollingCarrierNumber++;
            if(!(m_pCarrier->isCarrierNumberLegal(m_iPollingCarrierNumber)))
                m_iPollingCarrierNumber = 1;

            this->inMotionPolling();

            break;
        }
        case CARRIERMANAGER_STATUS_UNTOUCHED:
        {
            if(m_pCarrier->isAllLogicCarrierStatusSame("待机"))
            {
                emit RequestAfterAllCarrierTouched();
                m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_STANDBY;

                //输出调试语句
                emit RequestPrintDebugMessage("**接触成功，用户允许操作**");

                return;
            }

            //polling x+1
            m_iPollingCarrierNumber++;
            if(!(m_pCarrier->isCarrierNumberLegal(m_iPollingCarrierNumber)))
                m_iPollingCarrierNumber = 1;

            this->inMotionPolling();

            break;
        }
        case CARRIERMANAGER_STATUS_RELOCATING:
        {
            if(m_pCarrier->isAllLogicCarrierStatusSame("待机"))
            {
                m_iCarrierManagerStatus = CARRIERMANAGER_STATUS_STANDBY;

                return;
            }

            //poling x+1
            m_iPollingCarrierNumber++;
            if(!(m_pCarrier->isCarrierNumberLegal(m_iPollingCarrierNumber)))
                m_iPollingCarrierNumber = 1;

            this->inMotionPolling();

            break;
        }
        case CARRIERMANAGER_STATUS_STOPED:
        {
            return;
            break;
        }
        default:
            break;
    }









}
