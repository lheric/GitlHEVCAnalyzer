TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus/src
HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


