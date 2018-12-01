#include "actionactuator.h"
#include "stationport.h"
#include <actionplayer.h>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTime>
#include <QString>
#include <QStringList>

/*
 *  RealActionActuator
 */
RealActionActuator::RealActionActuator()
{

}

void RealActionActuator::generateMotion(QList<QString> list)
{
    //分离命令，拓展
    QList<QByteArray> packedList;
    int rawListNum = list.count();

    //多行命令要每行都拓展
    for(int index = 0;index < rawListNum;index++)
    {
        QStringList cmdStrList =  list[index].split(" ");
        int leaderCar = (cmdStrList[0]).toInt();
        int groupCarCount = (cmdStrList[1]).toInt();
        int leaderCarGoal = (cmdStrList[2]).toInt();

        //数值非零
        if(!leaderCar || !groupCarCount || !leaderCarGoal)
        {
            QMessageBox::warning(NULL,tr("command error"),tr("数量错误！"));
            return;
        }

        //将一行拓展
        for(int carIndex = 0;carIndex < groupCarCount;carIndex++)
        {
            QByteArray tempArray;

            //添加数据包要求的车辆号
            tempArray.append(leaderCar + carIndex);

            //注意目标点占位4
            setPos tempGoal ;
            tempGoal.integer = leaderCarGoal + (carIndex * 1);

            QByteArray baGoal;
            baGoal.append(tempGoal.uch[3]);
            baGoal.append(tempGoal.uch[2]);
            baGoal.append(tempGoal.uch[1]);
            baGoal.append(tempGoal.uch[0]);
            baGoal = baGoal.right(4);

            tempArray.append(baGoal);

            packedList<<tempArray;
        }
    }

    //发送动作列表
    emit RequestStartPlayingAction(packedList);
}



/*
 *  VirtualActionActuator
 */

VirtualActionActuator::VirtualActionActuator()
{

}

void VirtualActionActuator::generateMotion(QList<QString> list)
{
    qDebug()<<"VirtualActuator Motion:"<<list;

    QTime _Timer = QTime::currentTime().addMSecs(1000);
    while( QTime::currentTime() < _Timer )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    m_pParentPlayer->doNextStep();
}
