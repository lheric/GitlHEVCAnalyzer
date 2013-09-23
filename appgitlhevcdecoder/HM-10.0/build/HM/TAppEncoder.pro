# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = HM_100


DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .

DESTDIR = $${OUT_PWD}/../../../../decoders

include(TAppEncoder.pri)

LIBS += -L$$OUT_PWD/..

Debug:   LIBS += -lTLibCommond -lTAppCommond -lTLibVideoIOd -lTLibEncoderd
Release: LIBS += -lTLibCommon  -lTAppCommon  -lTLibVideoIO  -lTLibEncoder

DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
