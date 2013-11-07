#-------------------------------------------------
#
# Project created by QtCreator 2013-10-31T17:40:15
#
#-------------------------------------------------

QT       += core gui network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chat
TEMPLATE = app

INCLUDEPATH += ..

SOURCES += main.cpp \
    mainwindow.cpp \
    logindialog.cpp \
    chatclient.cpp

HEADERS += \
    mainwindow.h \
    logindialog.h \
    chatclient.h
