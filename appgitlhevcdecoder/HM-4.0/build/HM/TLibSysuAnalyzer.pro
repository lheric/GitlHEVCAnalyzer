# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
CONFIG(debug, debug|release){
    TARGET = TLibSysuAnalyzerd
}
CONFIG(release, debug|release){
    TARGET = TLibSysuAnalyzer
}
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer
msvc:lessThan(QMAKE_MSC_VER, 1600): INCLUDEPATH += ../../compat/msvc  # stdint.h stub only for VS2008 and older
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibSysuAnalyzer.pri)
