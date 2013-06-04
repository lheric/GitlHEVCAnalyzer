TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus/src
HEADERS         = highlightlcufilter.h
SOURCES         = highlightlcufilter.cpp
TARGET          = $$qtLibraryTarget(libhighlightlcufilter)
DESTDIR         = $${OUT_PWD}/../../plugins


