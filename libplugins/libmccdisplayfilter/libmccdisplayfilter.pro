TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = mccdisplayfilter.h
SOURCES         = mccdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(mccdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


