#ifndef STATIONSERIAL_H
#define STATIONSERIAL_H

#include <QtSerialPort/QSerialPort>
#include <QThread>

#define SHOW_SERIALRECV  0   //允许显示串口接收数据
#define SHOW_SERIALSEND  0   //允许显示串口发送数据

#define PORT_HEARTBEAT_SEND     0x80
#define PORT_HEARTBEAT_BACK     0x81
#define PORT_CONFIG_SET         0x90
#define PORT_CONFIG_REQUEST     0x91
#define PORT_CONFIG_BACK        0x92
#define PORT_GOALMOVE_SEND      0xA0
#define PORT_CONTROL_SEND       0xB0

class DataSendWorker;

class StationPort : public QSerialPort
{
    Q_OBJECT

public:
    StationPort();
    ~StationPort();

    QByteArray m_RawData;
    QList<QByteArray> m_List_PackageData;

    int startConnect(QString portname);
    bool stopConnect();

private:
    struct {
        QString portname;
        int baudrate;
        QSerialPort::DataBits databits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopbits;
        QSerialPort::FlowControl flowcontrol;
    }portSettings;

    QThread m_tDataSendThread;

    void packetPackage(QList<QByteArray> &list,int port);
    void SplitPortdataPackage();
    void IdentifyListCommand();

private slots:
    void OnStationPortDataCome();

public slots:
    void SendPackageData(QList<QByteArray> list,int port);

signals:
    void RequestPrintMessage(QString text);

    void RequestThreadSendData(QList<QByteArray> list);

    void RequestSetCarrierStatus(int carNum,int stu,int pos,int goal);
    void RequestSetCarrierProfile(QByteArray config);

};

class DataSendWorker : public QObject
{
    Q_OBJECT

public:
    DataSendWorker(StationPort* parent)
    {m_pParentStationPort = parent;}

    StationPort* m_pParentStationPort;

public slots:
    void PackAndSendData(QList<QByteArray> list);
};


#endif // STATIONSERIAL_H
