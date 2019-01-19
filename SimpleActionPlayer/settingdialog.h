#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>
#include <QCloseEvent>
#include <QStandardItemModel>
#include "carriertableviewdelegate.h"
#include "apdfilemanager.h"

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

private:
    Ui::SettingDialog *ui;
     ApdFileManager* m_pApdFileManager = NULL;

    QStandardItemModel* m_pCarrier = NULL;
    EnableBoxDelegate m_carrierEnableDelegate;
    ReadOnlyDelegate m_carrierReadOnlyDelegate;


    void initWindowStyle();
    bool writeCarrierProfile();
    void closeSettingDialog();

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();
     void fillAvaliablePorts();

signals:
    void RequestSetStationPort(QString portname);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_BTN_PortRefrush_clicked();

    void on_BTN_ConnectDetect_clicked();

    void on_BTN_ConfirmSets_clicked();

    void on_BTN_CancelSets_clicked();

    void on_BTN_LoadApdFile_clicked();
};

#endif // SETTINGDIALOG_H
