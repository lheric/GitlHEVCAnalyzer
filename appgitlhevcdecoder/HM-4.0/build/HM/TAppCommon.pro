# ----------------------------------------------------

# ------------------------------------------------------


TEMPLATE = lib
CONFIG(debug, debug|release){
    TARGET = TAppCommond
}
CONFIG(release, debug|release){
    TARGET = TAppCommon
}
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc:INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TAppCommon.pri)
