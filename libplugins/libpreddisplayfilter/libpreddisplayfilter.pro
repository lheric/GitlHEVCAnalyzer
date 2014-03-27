greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin
TARGET          = $$qtLibraryTarget(libpreddisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = \
                preddisplayfilter.h

SOURCES         = \
                preddisplayfilter.cpp \

include(../filterconfiggui.pri)
