# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
Debug:   TARGET = TLibVideoIOd
Release: TARGET = TLibVideoIO
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
	../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibVideoIO.pri)
