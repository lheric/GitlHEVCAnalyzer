# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
debug:   TARGET = TLibCommond
release: TARGET = TLibCommon
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibCommon.pri)
