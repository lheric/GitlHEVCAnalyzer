greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin
TARGET          = $$qtLibraryTarget(libtudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = \
    tudisplayfilter.h

SOURCES         = \
    tudisplayfilter.cpp

include(../filterconfiggui.pri)
