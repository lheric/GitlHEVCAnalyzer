# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = HM_40

DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .

DESTDIR = $${OUT_PWD}/../../../../decoders

include(TAppDecoder.pri)

LIBS += -L$$OUT_PWD/..
Debug:   LIBS += -lTLibCommond -lTAppCommond -lTLibSysuAnalyzerd -lTLibVideoIOd -lTLibDecoderd
Release: LIBS += -lTLibCommon  -lTAppCommon  -lTLibSysuAnalyzer  -lTLibVideoIO  -lTLibDecoder
DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
