#ifndef ACTIONACTUATOR_H
#define ACTIONACTUATOR_H

#include <qtsingleapplication.h>
#include <QList>
#include <QString>

union setPos
{
    int integer;
    unsigned char uch[4];
};

class ActionPlayer;

class ActionActuator : public QObject
{
    Q_OBJECT
public:
    virtual void generateMotion(QList<QString>) = 0;

    ActionPlayer* m_pParentPlayer = NULL;

private:
    QList<QString> m_lCmdParm;

};

class RealActionActuator : public ActionActuator
{
    Q_OBJECT
public:
    explicit RealActionActuator();
    void generateMotion(QList<QString> list);

signals:
    void RequestStartPlayingAction(QList<QByteArray> actionList);

private:
    QList<QString> m_lCmdParm;

};

class VirtualActionActuator : public ActionActuator
{
    Q_OBJECT
public:
    explicit VirtualActionActuator();
    void generateMotion(QList<QString> list);

private:
    QList<QString> m_lCmdParm;

};

#endif // ACTIONACTUATOR_H
