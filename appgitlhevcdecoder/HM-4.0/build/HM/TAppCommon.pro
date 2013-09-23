# ----------------------------------------------------

# ------------------------------------------------------


TEMPLATE = lib
Debug:   TARGET = TAppCommond
Release: TARGET = TAppCommon
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TAppCommon.pri)
