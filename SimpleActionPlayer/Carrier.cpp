#include "Carrier.h"
#include "QMessageBox"
#include <QModelIndex>

#define INFORM_NUM                  6
#define INFORM_CARMODEL_COLUMN      0           //车辆模型号
#define INFORM_CARPOS_COLUMN        1           //车辆位置
#define INFORM_CARGOAL_COLUMN       2           //车辆目标
#define INFORM_CARSPEED_COLUMN      3           //车辆速度
#define INFORM_CARSTATUS_COLUMN     4           //车辆状态
#define INFORM_CARENABLED_COLUMN    5           //车辆是否使能

#define CARSTATUS_ERROR         0x01
#define CARSTATUS_STANDBY       0x02
#define CARSTATUS_RUNNING       0x03
#define CARSTATUS_MISSING       0x04

/*  Translate Map   */
QMap<QString,int> map_StatusCmd =
{
    {"错误",0x01},{"待机",0x02},{"运动中",0x03},{"丢失",0x04}
};

QMap<QString,int> map_SpeedCmd =
{
    {"低速",0x01},{"中低速",0x02},{"中速",0x03},{"中高速",0x04},{"高速",0x05}
};

QMap<QString,int> map_SpeedcurveCmd =
{
    {"S曲线",0x01},{"定常曲线",0x02}
};

QMap<QString,int> map_CarenableCmd =
{
    {"启用",0x01},{"停用",0x02}
};

QMap<QString,int> map_ControlCmd =
{
    {"急停",0x01},{"复位",0x02}
};

int ConvertStringToCmd(QMap<QString, int> &map, QString str)
{
    return map.value(str);
}

QString ConvertCmdToString(QMap<QString, int> &map, int val)
{
    return map.key(val);
}



Carrier::Carrier(QList<QString> profileList, QObject *parent) : QStandardItemModel(profileList.count(),INFORM_NUM,parent)
{
    //设置表头
    QStringList modelHeader={tr("Model"),tr("Position"),tr("Goal"),tr("Speed"),tr("Status"),tr("Enabled")};
    this->setHorizontalHeaderLabels(modelHeader);

    m_iCarrierNum = profileList.count();

    for(int row = 0; row < m_iCarrierNum;row++)
    {
        QStringList carProfList = QString(profileList.value(row)).split(" ");
        for(int column = 0;column < INFORM_NUM;column++)
        {
            QModelIndex index = this->index(row,column);
            switch (column)
            {
                case INFORM_CARMODEL_COLUMN:
                    this->setData(index,QVariant(QString(carProfList.value(0))));
                    break;
                case INFORM_CARPOS_COLUMN:
                    this->setData(index,QVariant(QString(carProfList.value(1)).toInt()));
                    break;
                case INFORM_CARGOAL_COLUMN:
                    this->setData(index,0);
                    break;
                case INFORM_CARSPEED_COLUMN:
                    this->setData(index,QVariant(ConvertCmdToString(map_SpeedCmd,QString(carProfList.value(2)).toInt())));
                    break;
                case INFORM_CARSTATUS_COLUMN:
                    this->setData(index,QVariant(ConvertCmdToString(map_StatusCmd,QString(carProfList.value(3)).toInt())));
                    break;
                case INFORM_CARENABLED_COLUMN:
                    this->setData(index,QVariant(ConvertCmdToString(map_CarenableCmd,QString(carProfList.value(4)).toInt())));
                    break;
                default:
                    break;
            }
        }
    }
}

Carrier::~Carrier()
{
}

void Carrier::bandViewer(QTableView *viewerpointer)
{
    //绑定模型与视图
    viewerpointer->setModel(this);
}

void Carrier::setAllLogicCarrierStatusSame(QString status)
{
    //设置所有载体车状态一致
    for(int i = 0;i < m_iCarrierNum;i++)
    {
        QModelIndex tempIndex = this->index(i,INFORM_CARSTATUS_COLUMN);
        this->setData(tempIndex,QVariant(status));
    }
}

/*
 *  检查是否所有载体车已经运动到点
 */
bool Carrier::isAllLogicCarrierMotionAtPoint()
{
    for(int _carIndex = 1;_carIndex <= m_iCarrierNum;_carIndex++)
    {
        //跳过未使能车辆
        if(!(this->isCarrierEnabled(_carIndex)))
            continue;

        //取得车辆目标点
        int _logicGoal = this->getSpecificLogicCarrierGoal(_carIndex);

        //取得车辆当前点
        QModelIndex _tempIndex = this->index(_carIndex - 1,INFORM_CARPOS_COLUMN);
        int _nowPos = (this->data(_tempIndex)).toInt();

        //比较是否相等
        if(!(_nowPos == _logicGoal))
            return false;
    }
    return true;
}

bool Carrier::isAllLogicCarrierStatusSame(QString status)
{
    //检测是否每台载体车都是处于同样的status

    for(int i = 0;i < m_iCarrierNum;i++)
    {
        //跳过未使能车辆
        if(!(this->isCarrierEnabled(i + 1)))
            continue;

        QString _tempStr = this->data(this->index(i,INFORM_CARSTATUS_COLUMN)).toString();
        if(_tempStr != status)
            return false;
    }
    return true;
}

bool Carrier::isCarrierNumberLegal(int carrierNumber)
{
    if((carrierNumber > 0) && (carrierNumber <= m_iCarrierNum))
        return true;
    else
        return false;
}

bool Carrier::isCarrierStatusLegal(int status)
{
    if((status >= CARSTATUS_ERROR) && (status <= CARSTATUS_MISSING))
        return true;
    else
        return false;
}

bool Carrier::isCarrierEnabled(int carrierNumber)
{
    if(!isCarrierNumberLegal(carrierNumber))
        return false;

    //取得车辆是否使能
    QString _enabled = this->data(this->index(carrierNumber-1,INFORM_CARENABLED_COLUMN)).toString();
    if(_enabled == "启用")
        return true;
    else
        return false;
}

void Carrier::updateLogicCarrierStatus(int carNum, int stu, int pos)
{
    //车辆号有效性判断
    if(isCarrierNumberLegal(carNum))
    {
        /*******************************设置状态**************************************/
        if(isCarrierStatusLegal(stu))
        {
            QModelIndex tempIndex = this->index(carNum - 1,INFORM_CARSTATUS_COLUMN);
            this->setData(tempIndex,QVariant(ConvertCmdToString(map_StatusCmd,stu)));

            //修改状态图标
            QStandardItem* tempItem = this->item(carNum - 1);
            switch (stu)
            {
                case CARSTATUS_ERROR:
                {
                    tempItem->setIcon(QIcon(":/img/carrier_error"));
                    break;
                }
                case CARSTATUS_STANDBY:
                {
                    tempItem->setIcon(QIcon(":/img/carrier_standby"));
                    break;
                }
                case CARSTATUS_RUNNING:
                {
                    tempItem->setIcon(QIcon(":/img/carrier_running"));
                    break;
                }
                default:
                {
                    break;
                }
            }

        #if  ENABLE_HEARTBEAT_ERROR_OPTION

            //若任何车发回了错误状态，则发送紧急停车
            bool compStatu = false;
            compStatu = (QString("错误") == ConvertCmdToString(map_StatusCmd,stu));
            if(compStatu == true)
            {
                //若状态为 错误 则进行停车操作 其他状态不进行操作
                CarrierEntiretyControl("急停");
            }

        #endif

         /*******************************设置位置**************************************/
            if(pos >= 0)
            {
                QModelIndex tempIndex = this->index(carNum - 1,INFORM_CARPOS_COLUMN);
                this->setData(tempIndex,QVariant((unsigned int)(pos)));
            }
            else
            {
                //位置有效性判断错误
                qDebug()<<"Carrier::updateLogicCarrierStatus - 位置有效性检查错误";
                return;
            }
        }
        else
        {
            //状态有效性判断错误
            qDebug()<<"Carrier::updateLogicCarrierStatus - 状态有效性检查错误";
            return;
        }
    }
    else
    {
        //车辆号错误
        qDebug()<<"Carrier::updateLogicCarrierStatus - 车辆号错误";
        return;
    }

}

/*
 *  根据传入的动作列表，修改对应载体车的目标点
 *  actionList  ：   车辆号(1) 目标点(4)
 */
void Carrier::updateLogicCarrierGoal(QList<QByteArray> actionList)
{
    int _carCount = actionList.count();
    for(int _index = 0;_index < _carCount;_index++)
    {
        //取得当前设置车辆号
        QByteArray _lineBytearray = actionList.value(_index);
        int _carNumber = _lineBytearray.at(0);

        //取得目标点
        unsigned int _carGoal = 0;
        for(int i = 0 ; i < 4 ; i++ )
            _carGoal |= ((unsigned char)(_lineBytearray.at(1 + i)) << (8*(3 - i)));

        //数据检查
        if(isCarrierNumberLegal(_carNumber))
        {
            if(_carGoal > 0)
            {
                //对应车辆号写入目标点
                QModelIndex _tempIndex = this->index(_carNumber - 1,INFORM_CARGOAL_COLUMN);
                this->setData(_tempIndex,QVariant((unsigned int)_carGoal));
            }
            else
            {
                qDebug()<< "Carrier::updateLogicCarrierGoal - 要求写入的目标点不在范围内";
                return;
            }
        }
        else
        {
            qDebug()<< "Carrier::updateLogicCarrierGoal - 要求写入的车辆号不在范围内";
            return;
        }
    }
}

/*
 *  根据特定车辆号，取得logiccarrier的目标点
 */
int Carrier::getSpecificLogicCarrierGoal(int carrierNumber)
{
    //数据检查
    if(isCarrierNumberLegal(carrierNumber))
    {
        QModelIndex _tempIndex = this->index(carrierNumber - 1,INFORM_CARGOAL_COLUMN);
        int _goal = (this->data(_tempIndex)).toInt();
        return _goal;
    }
    else
    {
        qDebug() << "getSpecificLogicCarrierGoal:要求的车辆号不在范围内";
    }
    return 0;
}
