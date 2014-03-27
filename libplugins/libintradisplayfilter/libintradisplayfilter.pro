greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin

TARGET          = $$qtLibraryTarget(libintradisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = intradisplayfilter.h
SOURCES         = intradisplayfilter.cpp

include(../filterconfiggui.pri)
