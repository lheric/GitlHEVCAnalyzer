TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


