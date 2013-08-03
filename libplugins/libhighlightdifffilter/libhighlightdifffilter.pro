TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = highlightdifffilter.h
SOURCES         = highlightdifffilter.cpp
TARGET          = $$qtLibraryTarget(libhighlightdifffilter)
DESTDIR         = $${OUT_PWD}/../../plugins


