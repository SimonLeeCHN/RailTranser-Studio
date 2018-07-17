#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include "stationport.h"
#include "actionplayer.h"
#include "Carrier.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void printMessage(QString str);
    void OnAddExistActionScriptFile();

private slots:   
    void on_BTN_Refresh_clicked(bool checked);

    void on_BTN_Option_clicked(bool checked);

    void on_BTN_Stop_clicked(bool checked);

    void on_LW_ActionSortcutList_itemDoubleClicked(QListWidgetItem *item);

    void on_LW_ActionSortcutList_customContextMenuRequested(const QPoint &pos);

    void on_BTN_ReLocate_clicked(bool checked);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;
    StationPort *m_pStationPort = NULL;
    int m_iCarrierNum = 1;
    Carrier *m_pCarrier;
    ActionPlayer *m_pActionPlayer = NULL;
    RealActionActuator *m_pRealActionActuator = NULL;

    void addActionScriptFile(QList<QUrl> fileList);
    void initWindowStyle();
    void initMenu();
    void fillAvaliablePorts();

    void componentInit();
    void componentDeinit();
};

#endif // MAINWINDOW_H
