TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = mvdisplayfilter.h
SOURCES         = mvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


