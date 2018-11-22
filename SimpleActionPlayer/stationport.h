#ifndef STATIONSERIAL_H
#define STATIONSERIAL_H

#include <QtSerialPort/QSerialPort>

#define SHOW_SERIALRECV  0   //允许显示串口接收数据
#define SHOW_SERIALSEND  1   //允许显示串口发送数据

#define PORT_HEARTBEAT_SEND     0x80
#define PORT_HEARTBEAT_BACK     0x81
#define PORT_CONFIG_SET         0x90
#define PORT_CONFIG_REQUEST     0x91
#define PORT_CONFIG_BACK        0x92
#define PORT_GOALMOVE_SEND      0xA0
#define PORT_CONTROL_SEND       0xB0


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

    void packetPackage(QList<QByteArray> &list,int port);
    void SplitPortdataPackage();
    void IdentifyListCommand();

private slots:
    void OnStationPortDataCome();

public slots:
    void SendPackageData(QList<QByteArray> list,int port);

signals:
    void RequestPrintMessage(QString text);

    void RequestSetCarrierStatus(int carNum,int stu,int pos);
    void RequestSetCarrierProfile(QByteArray config);

};

#endif // STATIONSERIAL_H
