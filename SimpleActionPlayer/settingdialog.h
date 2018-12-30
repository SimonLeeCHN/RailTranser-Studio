#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SettingDialog;
}

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

signals:
    void RequestSetStationPort(QString portname);

private slots:
    void on_BTN_PortRefrush_clicked();

    void on_BTN_ConnectDetect_clicked();

    void on_BTN_ConfirmSets_clicked();

    void on_BTN_CancelSets_clicked();

private:
    Ui::SettingDialog *ui;
    void fillAvaliablePorts();
};

#endif // SETTINGDIALOG_H
