#-------------------------------------------------
#
# Project created by QtCreator 2013-11-07T15:36:22
#
#-------------------------------------------------

QT       -= core gui

TARGET = server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

LIBS += -lboost_system -lboost_thread
INCLUDEPATH += ..
SOURCES += \
    nw-server.cpp
