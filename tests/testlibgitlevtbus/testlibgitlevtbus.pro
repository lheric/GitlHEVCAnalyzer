#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T02:26:05
#
#-------------------------------------------------

QT       += core testlib

QT       -= gui

TARGET = TestGitlEvtBus
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

INCLUDEPATH    += ../../libgitlevtbus/src

SOURCES += \
    testcase.cpp

LIBS += -L$${OUT_PWD}/../../libgitlevtbus

Debug:   LIBS += -lGitlEvtBusd
Release: LIBS += -lGitlEvtBus
