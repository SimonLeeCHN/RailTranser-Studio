#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define CASFCREATOR_PATH "../CASF-Creator/CASF-Creator.exe"

#include <QMainWindow>
#include <QListWidgetItem>
#include <QUrl>
#include <QProcess>
#include "stationport.h"
#include "actionplayer.h"
#include "carriermanager.h"
#include "settingdialog.h"
#include "toolboxdialog.h"
#include "apdfilemanager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadProjectFile(QUrl fileUrl);

signals:
    void RequestActStartLinkSetEnabled(bool);
    void RequestActStopLinkSetEnabled(bool);
    void RequestActRelocateSetEnabled(bool);
    void RequestActEmergencyStopSetEnabled(bool);
    void RequestActToolBoxSetEnabled(bool);

public slots:
    void printMessage(QString str);
    void OnAddExistActionScriptFile();
    void OnAddExistProjectFile();
    void OnAroseCasfCreator();
    void OnAroseCasfCreatorError(QProcess::ProcessError error);
    void OnRegesitFileRelation();
    void OnEnsureAllCarrierAlive();
    void OnSetStationPort(QString portname);

private slots:
    void on_LW_ActionSortcutList_itemDoubleClicked(QListWidgetItem *item);

    void on_LW_ActionSortcutList_customContextMenuRequested(const QPoint &pos);

    void on_PTE_MessageWindow_customContextMenuRequested(const QPoint &pos);

    void onActStartLinkTriggered();
    void onActStopLinkTriggered();
    void onActRelocateTriggered();
    void onActEmergencyStopTriggered();
    void onActAroseSettingDialog();
    void onActAroseToolBoxDialog();

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

private:
    Ui::MainWindow *ui;
    bool m_bIsProjectFileLoaded = false;
    SettingDialog *m_pSettingDialog;
    ToolBoxDialog *m_pToolBoxDialog;

    StationPort *m_pStationPort = NULL;
    CarrierManager* m_pCarrierManager = NULL;
    ActionPlayer *m_pActionPlayer = NULL;
    RealActionActuator *m_pRealActionActuator = NULL;

    QProcess *m_pCasfCreatorProcess = NULL;
    ApdFileManager* m_pApdFileManager = NULL;

    void addActionScriptFile(QList<QUrl> fileList);
    void initWindowStyle();
    void initMenu();
    void disableUserInterface();
    void enableUserInterface();

    void componentInit();
    void componentDeinit();
};

#endif // MAINWINDOW_H
