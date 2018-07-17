#ifndef ACTIONPLAYER_H
#define ACTIONPLAYER_H

#include <QString>
#include <QFile>
#include <QMap>
#include "actionactuator.h"

#define PLAYERSTU_STANDBY   0
#define PLAYERSTU_PLAYING   1
#define PLAYERSTU_WAITING   2

#define CMD_STA 1
#define CMD_END 2
#define CMD_MOV 3
#define CMD_DEY 4
#define CMD_CMG 5


class ActionPlayer :public QObject
{
    Q_OBJECT
public:
    ActionPlayer();
    ~ActionPlayer();
    bool loadActionFile(QString fileName);
    void setActuator(ActionActuator* acac);
    void stopActionPlayer();

    bool isPlaying();
    bool isWaitingTriger();

public slots:
    void doNextStep();

signals:
    void RequestPrintMessage(QString str);

private:
    QList<QString> m_lCmdList;
    ActionActuator* m_pActuator = NULL;
    int m_iCmdPointer;
    int m_iPlayerStatus = 0;
};

#endif // ACTIONPLAYER_H
