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
    if(argc > 1)
    {
        //"file:///C:/Users/SEMENTS_LAPTAP/Desktop/temp.apd"
        //C:\Users\SEMENTS_LAPTAP\Desktop\temp.apd
        QString str = argv[1];
        str.replace("\\","/");
        QUrl fileUrl(str);
        fileUrl.setScheme("file");

        w.loadProjectFile(fileUrl);
        w.printMessage(QString(argv[1]));
    }
    w.show();

    return a.exec();
}
