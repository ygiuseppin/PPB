#-------------------------------------------------
#
# Project created by QtCreator 2017-09-06T13:55:50
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ClientePPB
TEMPLATE = app

INCLUDEPATH += $$PWD/qmqtt/build/include
INCLUDEPATH += $$PWD/qmqtt/build/include/qmqtt

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += $$PWD/qmqtt/build/lib/libqmqtt.so

SOURCES += \
        main.cpp \
    publisher.cpp \
    subscriber.cpp \
    clienteppb.cpp \
    logindialog.cpp

HEADERS += \
    publisher.h \
    subscriber.h \
    clienteppb.h \
    logindialog.h

FORMS += \
    clienteppb.ui \
    logindialog.ui

RESOURCES += \
    images.qrc

