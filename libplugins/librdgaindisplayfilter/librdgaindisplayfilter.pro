TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = rdgaindisplayfilter.h
SOURCES         = rdgaindisplayfilter.cpp
TARGET          = $$qtLibraryTarget(librdgaindisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


