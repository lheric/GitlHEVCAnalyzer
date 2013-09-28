# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = HM_100
DESTDIR = $${OUT_PWD}/../../../../decoders

DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
include(TAppDecoder.pri)

LIBS += -L$$OUT_PWD/..

CONFIG(debug, debug|release){
    LIBS += -lTLibDecoderd -lTAppCommond -lTLibSysuAnalyzerd -lTLibVideoIOd -lTLibCommond
}
CONFIG(release, debug|release){
    LIBS += -lTLibDecoder  -lTAppCommon   -lTLibSysuAnalyzer -lTLibVideoIO  -lTLibCommon
}

DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
