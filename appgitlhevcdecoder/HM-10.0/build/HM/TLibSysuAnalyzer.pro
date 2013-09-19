# ----------------------------------------------------

# ------------------------------------------------------

TEMPLATE = lib
Debug:   TARGET = TLibSysuAnalyzerd
Release: TARGET = TLibSysuAnalyzer
CONFIG += staticlib
DEFINES += _CRT_SECURE_NO_WARNINGS
INCLUDEPATH += ../../source/Lib \
    ../../../TLibSysuAnalyzer  \
	../../compat/msvc
DEPENDPATH += .
DESTDIR = $${OUT_PWD}/..
include(TLibSysuAnalyzer.pri)
