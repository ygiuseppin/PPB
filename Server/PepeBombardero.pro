#-------------------------------------------------
#
# Project created by QtCreator 2017-08-31T19:05:07
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PepeBombardero
TEMPLATE = app

INCLUDEPATH += $$PWD/qmqtt/build/include
INCLUDEPATH += $$PWD/qmqtt/build/include/qmqtt

DEFINES += QT_DEPRECATED_WARNINGS

LIBS += $$PWD/qmqtt/build/lib/libqmqtt.so

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    game.cpp \
    publisher.cpp \
    subscriber.cpp

HEADERS += \
        mainwindow.h \
    game.h \
    publisher.h \
    subscriber.h

FORMS += \
        mainwindow.ui

RESOURCES += \
    images.qrc
