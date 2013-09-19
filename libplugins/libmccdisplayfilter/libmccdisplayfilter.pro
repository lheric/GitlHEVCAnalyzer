TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = mccdisplayfilter.h
SOURCES         = mccdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmccdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


