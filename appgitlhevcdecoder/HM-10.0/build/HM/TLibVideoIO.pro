# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
CONFIG(debug, debug|release){
    TARGET = TLibVideoIOd
}
CONFIG(release, debug|release){
    TARGET = TLibVideoIO
}
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc:lessThan(QMAKE_MSC_VER, 1600): INCLUDEPATH += ../../compat/msvc  # stdint.h stub only for VS2008 and older
# Qt's MSVC mkspec adds -Zc:strictStrings; old HM passes string literals as char*
msvc: QMAKE_CFLAGS -= -Zc:strictStrings
msvc: QMAKE_CXXFLAGS -= -Zc:strictStrings
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibVideoIO.pri)
