greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin

TARGET          = $$qtLibraryTarget(libcudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = cudisplayfilter.h
SOURCES         = cudisplayfilter.cpp

include(../filterconfiggui.pri)

