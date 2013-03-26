TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(mergedisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


