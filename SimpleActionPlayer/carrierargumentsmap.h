#ifndef CARRIERARGUMENTSMAP_H
#define CARRIERARGUMENTSMAP_H

#include <QMap>

extern QMap<QString,int> map_StatusCmd;
extern QMap<QString,int> map_SpeedCmd;
extern QMap<QString,int> map_SpeedcurveCmd;
extern QMap<QString,int> map_CarenableCmd;
extern QMap<QString,int> map_ControlCmd;

inline int ConvertStringToCmd(QMap<QString, int> &map, QString str)
{
    return map.value(str);
}

inline QString ConvertCmdToString(QMap<QString, int> &map, int val)
{
    return map.key(val);
}

#endif // CARRIERARGUMENTSMAP_H
