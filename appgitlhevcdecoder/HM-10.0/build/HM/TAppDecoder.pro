# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = TAppDecoder

DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
include(TAppDecoder.pri)

LIBS += -L$$OUT_PWD/..
debug:   LIBS += -lTLibCommond -lTAppCommond -lTLibSysuAnalyzerd -lTLibVideoIOd -lTLibDecoderd
release: LIBS += -lTLibCommon  -lTAppCommon  -lTLibSysuAnalyzer  -lTLibVideoIO  -lTLibDecoder
DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
