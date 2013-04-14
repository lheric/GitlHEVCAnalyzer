TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mccdisplayfilter.h
SOURCES         = mccdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmccdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


