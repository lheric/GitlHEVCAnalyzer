# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
Debug:   TARGET = TLibEncoderd
Release: TARGET = TLibEncoder
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib
msvc: INCLUDEPATH += ../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibEncoder.pri)
