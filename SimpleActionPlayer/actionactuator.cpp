#include "actionactuator.h"
#include "stationport.h"
#include <actionplayer.h>
#include <QDebug>
#include <QMessageBox>
#include <QString>
#include <QStringList>

union setPos
{
    int integer;
    unsigned char uch[4];
};

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

            //先加入一个车辆号，供packerPackage来添加车辆地址，之后再填入数据
            tempArray.append(leaderCar + carIndex);

            //添加数据包要求的车辆号
            tempArray.append(leaderCar + carIndex);

            //注意目标点占位4
            setPos tempGoal ;
            tempGoal.integer = leaderCarGoal + carIndex;

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

    qDebug()<<packedList;
    emit RequestSendPackageData(packedList,PORT_GOALMOVE_SEND);

    //应该等待载体车全部返回停止指令再调用doNextStep
//    m_pParentPlayer->doNextStep();
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
    m_pParentPlayer->doNextStep();
}
