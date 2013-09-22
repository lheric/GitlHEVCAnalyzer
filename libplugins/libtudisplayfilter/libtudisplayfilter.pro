TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = \
    tudisplayfilter.h
SOURCES         = \
    tudisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libtudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


