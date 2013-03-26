TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = highlightlcufilter.h
SOURCES         = highlightlcufilter.cpp
TARGET          = $$qtLibraryTarget(highlightlcu)
DESTDIR         = $${OUT_PWD}/../../plugins


