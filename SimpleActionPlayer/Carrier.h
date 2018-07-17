#ifndef CARRIER_H
#define CARRIER_H

#include <QStandardItemModel>
#include <QTableView>
#include <QDebug>
#include <QMap>
#include <QList>
#include <QTimer>
#include "StationPort.h"

#define HEARTBEAT_TIME  2000    //ms
#define ENABLE_HEARTBEAT_ERROR_OPTION  0

class Carrier : public QStandardItemModel
{
    Q_OBJECT

public:
    Carrier(int carrierNum, QObject *parent = nullptr);
    ~Carrier();
    void BandViewer(QTableView *viewerpoint);

    int IsAllCarrierStatusSame(QString status);

    void GetCarrierConfig();
    void SetCarrierConfig();
    void CarrierEntiretyControl(QString strCmd);

private:
    QTimer* m_pHeartbeatTimer = nullptr;
    QList<bool> m_HeartbeatRecordList;
    int m_iCarrierNum;

    void InitModelData();

    void StartHeartbeatTimer();
    void StopHearbeatTimer();

private slots:
    void OnHeartbeatTimeup();

public slots:
    void OnSetCarrierStatus(int carNum,int stu,int pos);
    void OnSetCarrierProfile(QByteArray config);

signals:
    void RequestPrintDebugMessage(QString text);

    void RequestSendPackageData(QList<QByteArray> list,int port);
    void RequestAfterAllCarStandby();
};




#endif // CARRIER_H
