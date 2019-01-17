#include "apdfilemanager.h"
#include <QMessageBox>
#include <QFile>
#include <QtCore>

#define PROJECTFILE_TEXT_RFID           "RFID_POS\n"
#define PROJECTFILE_TEXT_CARRIER        "CARRIER_PRF\n"

ApdFileManager::ApdFileManager(QWidget *parent)
{
    m_pParentWidget = parent;
}

QList<QString> ApdFileManager::getFileRfidConfigList(QString filePath)
{
    QList<QString> _rfidList,_tempList;

    QFile _projectFile(filePath);
    if(!_projectFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无法打开文件"));
        return _rfidList;
    }

    /*  加载RFID点位    */
    int _iRfidNum = 0;
    _projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_RFID,_projectFile.readLine()))
        {
            _iRfidNum = QString(_projectFile.readLine()).toInt();
            break;
        }

        if(_projectFile.atEnd())
        {
            QMessageBox::critical(m_pParentWidget,tr("错误的文件格式"),("未能正确加载坐标信息"));
            _projectFile.close();
            return _rfidList;
        }
    }

    for(int i = 0;i < _iRfidNum;i++)
    {
        if(_projectFile.atEnd())
        {
            QMessageBox::critical(m_pParentWidget,tr("错误的文件格式"),("未能正确加载坐标信息"));
            _projectFile.close();
            return _rfidList;
        }

        QString _lineData = _projectFile.readLine();
        _lineData.trimmed();
        if(_lineData.isEmpty())
            break;

        QStringList _lineConfigList = _lineData.split(" ");
        if(_lineConfigList.count() != 3)
        {
            QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无效的RFID点位格式"));
            _projectFile.close();
            return _rfidList;
        }

        QString _x = _lineConfigList.value(0);
        QString _y = _lineConfigList.value(1);
        QString _number = _lineConfigList.value(2);

        _tempList << QString(_x + " " + _y + " " + _number).trimmed();
    }

    _projectFile.close();
    _rfidList = _tempList;
    return _rfidList;
}

QList<QString> ApdFileManager::getFileCarrierConfigList(QString filePath)
{
    QList<QString> _carrierConfigList,_tempList;

    QFile _projectFile(filePath);
    if(!_projectFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无法打开文件"));
        return _carrierConfigList;
    }

    /*  加载carrier配置 */
    int _iCarrierCount = 0;
    _projectFile.seek(0);
    while(1)
    {
        if(0 == QString::compare(PROJECTFILE_TEXT_CARRIER,_projectFile.readLine()))
        {
            _iCarrierCount = QString(_projectFile.readLine()).toInt();
            break;
        }

        if(_projectFile.atEnd())
        {
            QMessageBox::critical(m_pParentWidget,tr("错误的文件格式"),("未能正确加载载体车信息"));
            _projectFile.close();
            return _carrierConfigList;
        }
    }

    for(int i = 0;i < _iCarrierCount;i++)
    {
        if(_projectFile.atEnd())
        {
            QMessageBox::critical(m_pParentWidget,tr("错误的文件格式"),("未能正确加载载体车信息"));
            _projectFile.close();
            return _carrierConfigList;
        }

        QString _lineData = _projectFile.readLine();
        _lineData.trimmed();
        if(_lineData.isEmpty())
            break;

        QStringList _lineConfigList = _lineData.split(" ");
        if(_lineConfigList.count() != 5)
        {
            QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无效的载体车数据格式"));
            _projectFile.close();
            return _carrierConfigList;
        }

        QString _strModel = QString(_lineConfigList.value(0));
        QString _strPos = QString(_lineConfigList.value(1));
        QString _strSpeed = QString(_lineConfigList.value(2));
        QString _strStatus = QString(_lineConfigList.value(3));
        QString _strEnabled = QString(_lineConfigList.value(4));

        _tempList << QString(_strModel + " " + _strPos + " " + _strSpeed + " "
                               + _strStatus + " " + _strEnabled).trimmed();
    }

    _projectFile.close();
    _carrierConfigList = _tempList;
    return _carrierConfigList;
}

bool ApdFileManager::setFileCarrierConfigList(QString filePath, QList<QString> carrierConfigList)
{
    /*      读取配置文件到qstring      */
    QFile _projectFile(filePath);
    if(!_projectFile.open(QIODevice::ReadOnly| QIODevice::Text))
    {
            QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无法打开文件"));
            return false;
    }

    QString _strFileData = _projectFile.readAll();


    /*      截掉旧的CARRIER配置部分    */
    //注意这里由于是字符串sizeof，已经包含了换行符
    int _writeBeginPos = _strFileData.indexOf(PROJECTFILE_TEXT_CARRIER) + sizeof(PROJECTFILE_TEXT_CARRIER) - 1;
    _strFileData.chop(_strFileData.length() - _writeBeginPos);


    /*      写入新配置       */

    //写入载体车数量
    int _countNum = carrierConfigList.count();
    _strFileData.append(QString::number(_countNum) + '\n');

    //写入列表项
    for(int _index = 0;_index < _countNum;_index++)
    {
        _strFileData.append(carrierConfigList.value(_index) + '\n');
    }

    //写到文件
    _projectFile.close();
    _projectFile.remove();
    if(!_projectFile.open(QIODevice::WriteOnly| QIODevice::Text))
    {
        QMessageBox::critical(m_pParentWidget,tr("Cannot Open File"),tr("无法打开文件"));
        return false;
    }
    _projectFile.write(_strFileData.toUtf8());
    _projectFile.close();
    return true;

}

bool ApdFileManager::setFileCarrierEnabled(QString filePath, QList<QString> enableList)
{
    /*      获取      */
    QList<QString> _oldCarrierConfigList = this->getFileCarrierConfigList(filePath);
    if(enableList.count() != _oldCarrierConfigList.count())
    {
        QMessageBox::critical(m_pParentWidget,tr("设置使能数据失败"),tr("新旧数据量不一致"));
        return false;
    }
    int _replacePos = _oldCarrierConfigList.value(0).lastIndexOf(' ') + 1;

    /*      替换      */
    QList<QString> _newCarrierConfigList;
    for(int _index = 0;_index < enableList.count();_index++)
    {
        QString _tempStr = _oldCarrierConfigList.value(_index);
        _tempStr.replace(_replacePos,1,enableList.value(_index));
        _newCarrierConfigList << _tempStr;
    }

    /*      写入      */
    if(this->setFileCarrierConfigList(filePath,_newCarrierConfigList))
        return true;
    else
        return false;
}
