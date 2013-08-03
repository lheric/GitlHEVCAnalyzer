TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = rdgaindisplayfilter.h
SOURCES         = rdgaindisplayfilter.cpp
TARGET          = $$qtLibraryTarget(librdgaindisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


