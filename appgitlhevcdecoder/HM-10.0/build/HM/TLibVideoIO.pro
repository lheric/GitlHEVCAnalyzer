# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
debug:   TARGET = TLibVideoIOd
release: TARGET = TLibVideoIO
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibVideoIO.pri)
