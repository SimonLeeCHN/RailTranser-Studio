#ifndef APDFILEMANAGER_H
#define APDFILEMANAGER_H

#include <QObject>
#include <QString>

class ApdFileManager : public QObject
{
    Q_OBJECT

public:
    ApdFileManager(QWidget *parent = 0);

    QList<QString> getFileRfidConfigList(QString filePath);
    QList<QString> getFileCarrierConfigList(QString filePath);
    bool setFileCarrierConfigList(QString filePath,QList<QString> carrierConfigList);
    bool setFileCarrierEnabled(QString filePath,QList<QString> enableList);

private:
    QWidget* m_pParentWidget = NULL;

};

#endif // APDFILEMANAGER_H
