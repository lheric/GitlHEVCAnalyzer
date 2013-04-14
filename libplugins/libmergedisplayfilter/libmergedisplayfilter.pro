TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mergedisplayfilter.h
SOURCES         = mergedisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmergedisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


