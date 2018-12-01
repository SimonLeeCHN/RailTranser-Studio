#ifndef ACTIONPLAYER_H
#define ACTIONPLAYER_H

#include <QString>
#include <QFile>
#include <QMap>
#include "actionactuator.h"

#define PLAYERSTU_STANDBY   1
#define PLAYERSTU_PLAYING   2
#define PLAYERSTU_WAITING   3
#define PLAYERSTU_STOP      4


class ActionPlayer :public QObject
{
    Q_OBJECT
public:
    ActionPlayer();
    ~ActionPlayer();
    bool loadActionFile(QString fileName);
    void setActuator(ActionActuator* acac);


    int getPlayerStatus();

public slots:
    void doNextStep();
    void stopActionPlayer();

signals:
    void RequestPrintMessage(QString str);

private:
    QList<QString> m_lCmdList;
    ActionActuator* m_pActuator = NULL;
    int m_iCmdPointer;
    int m_iPlayerStatus = 0;
};

#endif // ACTIONPLAYER_H
