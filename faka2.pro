#-------------------------------------------------
#
# Project created by QtCreator 2019-05-25T17:09:24
#
#-------------------------------------------------

QT       += core gui network serialport sql texttospeech

LIBS += -lpthread libwsock32 libws2_32

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia multimediawidgets

CONFIG   += c++11

TARGET = faka
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS arma9 gcc47 opengl1 qt32

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        dialoglogin.cpp \
        logindialog.cpp \
    modifypasw.cpp \
    ccrashstack.cpp

HEADERS += \
        mainwindow.h \
        dialoglogin.h \
        logindialog.h \
    modifypasw.h \
    ccrashstack.h

FORMS += \
        mainwindow.ui \
        dialoglogin.ui \
    logindialog.ui \
    modifypasw.ui

include ($$PWD\hardware\hardware.pri)
include ($$PWD\ffmpeg\ffmpeg.pri)
include ($$PWD\soap\soap.pri)
include ($$PWD\ping\ping.pri)

INCLUDEPATH += $$PWD/ffmpeg\
               $$PWD/hardware\
               $$PWD/led\
               $$PWD/soap\
               $$PWD/ping

win32: LIBS += -L$$PWD/led/ -lonbon.api
LIBS += -lpthread libwsock32 libws2_32
DEPENDPATH += $$PWD/led

RESOURCES += \
    resource.qrc

RC_ICONS = $$PWD/image/log.ico

