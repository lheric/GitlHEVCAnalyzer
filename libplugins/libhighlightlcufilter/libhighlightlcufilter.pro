TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = highlightlcufilter.h
SOURCES         = highlightlcufilter.cpp
TARGET          = $$qtLibraryTarget(libhighlightlcufilter)
DESTDIR         = $${OUT_PWD}/../../plugins


