#ifndef CARRIERMANAGER_H
#define CARRIERMANAGER_H

#include "Carrier.h"
#include "stationport.h"

#define CARRIERMANAGER_STATUS_UNTOUCHED         0
#define CARRIERMANAGER_STATUS_STANDBY           1
#define CARRIERMANAGER_STATUS_INMOTION          2

#define CARRIERMANAGER_POLLING_OUTTIME          300

class CarrierManager: public QObject
{
    Q_OBJECT
public:
    CarrierManager();
    ~CarrierManager();

    void initLogicCarrier(QList<QString> profileList, QObject *parent);
    void startTouchRealCarrier();
    void startRealCarrierRelocate();
    void startRealCarrierEmergencyStop();
    void inMotionPolling();

    Carrier* m_pCarrier = NULL;

private:
    QTimer m_pollingTimer;

    int m_iCarrierManagerStatus = 0;
    int m_iPollingCarrierNumber = 1;

signals:
    void RequestPrintDebugMessage(QString data);
    void RequestAfterAllCarrierTouched();

    void RequestUpdateGraphicCarrier(int carNum,int stu,int pus);

    void RequestSendtoRealCarrier(QList<QByteArray> dataList,int port);

    void RequestActionplayerDoNext();
    void RequestActionplayerStop();

private slots:
    void OnPollingTimerOuttime();

public slots:
    void OnStartPlayingAction(QList<QByteArray> actionList);
    void OnRealCarrierHeartbeatBack(int carNumber,int carStatus,int carNowPos,int carGoal);
};

#endif // CARRIERMANAGER_H
