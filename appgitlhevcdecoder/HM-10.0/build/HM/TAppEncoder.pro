# ----------------------------------------------------

# ------------------------------------------------------
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt


TEMPLATE = app
TARGET = TAppEncoder


DEFINES += _CONSOLE _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc:lessThan(QMAKE_MSC_VER, 1600): INCLUDEPATH += ../../compat/msvc  # stdint.h stub only for VS2008 and older
# Qt's MSVC mkspec adds -Zc:strictStrings; old HM passes string literals as char*
msvc: QMAKE_CFLAGS -= -Zc:strictStrings
msvc: QMAKE_CXXFLAGS -= -Zc:strictStrings
DEPENDPATH += .

include(TAppEncoder.pri)

LIBS += -L$$OUT_PWD/..

CONFIG(debug, debug|release){
    LIBS += -lTLibEncoderd -lTAppCommond -lTLibVideoIOd -lTLibCommond
}
CONFIG(release, debug|release){
    LIBS += -lTLibEncoder  -lTAppCommon   -lTLibVideoIO  -lTLibCommon
}


DEPENDPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
