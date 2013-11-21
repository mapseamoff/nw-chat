#-------------------------------------------------
#
# Project created by QtCreator 2013-11-20T00:26:39
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = console_client
CONFIG   += console
INCLUDEPATH += ..

TEMPLATE = app

HEADERS += ../client/chatclient.h
SOURCES += main.cpp ../client/chatclient.cpp
