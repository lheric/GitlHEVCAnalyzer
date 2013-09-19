# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
Debug:   TARGET = TLibCommond
Release: TARGET = TLibCommon
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
	../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibCommon.pri)
