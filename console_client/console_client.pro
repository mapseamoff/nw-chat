#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T00:26:39
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = console_client
CONFIG   += console
INCLUDEPATH += ../client

TEMPLATE = app

SOURCES += main.cpp \
    ../client/chatclient.cpp

HEADERS += \
    ../client/chatclient.h \
    ../chatmessage.h \
    RunChat.h
