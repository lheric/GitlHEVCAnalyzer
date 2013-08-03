TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


