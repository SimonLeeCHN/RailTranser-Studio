#ifndef TOOLBOXDIALOG_H
#define TOOLBOXDIALOG_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class ToolBoxDialog;
}

class ToolBoxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ToolBoxDialog(QWidget *parent = 0);
    ~ToolBoxDialog();

signals:
    void RequestActionPlayerLoadList(QList<QString> directList);
    void RequestActionPlayerDoNextStep();

private slots:
    void on_BTN_ClearDirectCmd_clicked();

    void on_BTN_StartDirectCmd_clicked();

private:
    Ui::ToolBoxDialog *ui;
    void initWindowStyle();

protected:
    void closeEvent(QCloseEvent *event);
};

#endif // TOOLBOXDIALOG_H
