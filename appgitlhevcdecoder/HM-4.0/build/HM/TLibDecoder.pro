# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
Debug:   TARGET = TLibDecoderd
Release: TARGET = TLibDecoder
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
	../../compat/msvc \
    ../../..
DEPENDPATH += . \
    ../../../TLibSysuAnalyzer
DESTDIR = $${OUT_PWD}/..
include(TLibDecoder.pri)
