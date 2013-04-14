TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = highlightdifffilter.h
SOURCES         = highlightdifffilter.cpp
TARGET          = $$qtLibraryTarget(libhighlightdifffilter)
DESTDIR         = $${OUT_PWD}/../../plugins


