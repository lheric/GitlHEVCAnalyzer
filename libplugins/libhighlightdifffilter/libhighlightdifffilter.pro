TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = highlightdifffilter.h
SOURCES         = highlightdifffilter.cpp
TARGET          = $$qtLibraryTarget(highlightdiff)
DESTDIR         = $${OUT_PWD}/../../plugins


