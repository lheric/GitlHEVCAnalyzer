TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = bitdisplayfilter.h
SOURCES         = bitdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libbitdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


