TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = highlightdifffilter.h
SOURCES         = highlightdifffilter.cpp
TARGET          = $$qtLibraryTarget(libhighlightdifffilter)
DESTDIR         = $${OUT_PWD}/../../plugins


