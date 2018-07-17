#include "qtsingleapplication.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    //QApplication a(argc, argv);
    QtSingleApplication a(argc,argv);
    if(a.isRunning())
    {
        QMessageBox::warning(NULL,QString("error running"),QString("App is already running!"));
        return 0;
    }
    MainWindow w;
    w.show();

    return a.exec();
}
