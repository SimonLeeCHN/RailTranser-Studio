#-------------------------------------------------
#
# Project created by QtCreator 2018-06-22T09:58:17
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RailTranser-Studio
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES -= UNICODE

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include (./qtsingleapplication/src/qtsingleapplication.pri)

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    stationport.cpp \
    actionactuator.cpp \
    actionplayer.cpp \
    Carrier.cpp \
    pathwaygv.cpp \
    rfidgraphicsitem.cpp \
    carriergraphicsitem.cpp \
    plantformapply.cpp \
    carriermanager.cpp \
    settingdialog.cpp \
    toolboxdialog.cpp \
    carriertableviewdelegate.cpp \
    apdfilemanager.cpp

HEADERS += \
        mainwindow.h \
    stationport.h \
    actionactuator.h \
    actionplayer.h \
    Carrier.h \
    pathwaygv.h \
    rfidgraphicsitem.h \
    carriergraphicsitem.h \
    plantformapply.h \
    carriermanager.h \
    settingdialog.h \
    toolboxdialog.h \
    carrierargumentsmap.h \
    carriertableviewdelegate.h \
    apdfilemanager.h

FORMS += \
        mainwindow.ui \
    settingdialog.ui \
    toolboxdialog.ui

RESOURCES += \
    resource.qrc

RC_FILE = rc.rc         #应用程序图标，apd图标及casf图标都在该文件里定义路径

VERSION = DL-Zigbee_V3.2.1
QMAKE_TARGET_COMPANY = Genius
QMAKE_TARGET_DESCRIPTION = RailTranser-Studio
QMAKE_TARGET_COPYRIGHT = All rights recive
