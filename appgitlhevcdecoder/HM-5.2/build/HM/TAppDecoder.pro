# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = HM_52
DESTDIR = $${OUT_PWD}/../../../../decoders

DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
include(TAppDecoder.pri)

LIBS += -L$$OUT_PWD/..

CONFIG(debug, debug|release){
    LIBS += -lTLibCommond -lTAppCommond -lTLibSysuAnalyzerd -lTLibVideoIOd -lTLibDecoderd
}
CONFIG(release, debug|release){
    LIBS += -lTLibCommon  -lTAppCommon  -lTLibSysuAnalyzer  -lTLibVideoIO  -lTLibDecoder
}

DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
