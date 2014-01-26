# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
CONFIG(debug, debug|release){
    TARGET = TLibDecoderd
}
CONFIG(release, debug|release){
    TARGET = TLibDecoder
}
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
    ../../..
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += . \
    ../../../TLibSysuAnalyzer
DESTDIR = $${OUT_PWD}/..
include(TLibDecoder.pri)
