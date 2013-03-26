TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mergedisplayfilter.h
SOURCES         = mergedisplayfilter.cpp
TARGET          = $$qtLibraryTarget(mergedisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


