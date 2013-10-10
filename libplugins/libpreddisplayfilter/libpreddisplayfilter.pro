TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = \
    preddisplayfilter.h
SOURCES         = \
    preddisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libpreddisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


