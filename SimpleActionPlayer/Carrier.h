#ifndef CARRIER_H
#define CARRIER_H

#include <QStandardItemModel>
#include <QTableView>
#include <QDebug>
#include <QMap>
#include <QList>
#include <QTimer>
#include "stationport.h"

#define ENABLE_HEARTBEAT_ERROR_OPTION  0

class Carrier : public QStandardItemModel
{
    Q_OBJECT

public:
    Carrier(QList<QString> profileList, QObject *parent = nullptr);
    ~Carrier();

    void bandViewer(QTableView *viewerpointer);
    void setAllLogicCarrierStatusSame(QString status);
    bool isAllLogicCarrierStatusSame(QString status);
    bool isAllLogicCarrierMotionAtPoint();
    bool isCarrierNumberLegal(int carrierNumber);
    bool isCarrierStatusLegal(int status);
    void updateLogicCarrierStatus(int carNum,int stu,int pos);
    void updateLogicCarrierGoal(QList<QByteArray> actionList);
    int getSpecificLogicCarrierGoal(int carrierNumber);

    int m_iCarrierNum;

signals:
    void RequestPrintDebugMessage(QString text);
};




#endif // CARRIER_H
