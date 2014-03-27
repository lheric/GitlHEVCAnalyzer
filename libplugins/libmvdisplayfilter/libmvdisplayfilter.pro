greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin

TARGET          = $$qtLibraryTarget(libmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp

include(../filterconfiggui.pri)




