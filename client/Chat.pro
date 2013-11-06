#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T17:40:15
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = Chat
CONFIG   += console
CONFIG   -= app_bundle


TEMPLATE = app

LIBS += -L/home/andrey/boost/lib/
LIBS += -lboost_system -lboost_thread

INCLUDEPATH += /home/andrey/boost/include/

SOURCES += main.cpp \
    mainwindow.cpp \
    logindialog.cpp

HEADERS += \
    mainwindow.h \
    logindialog.h \
    ChatClient.h
