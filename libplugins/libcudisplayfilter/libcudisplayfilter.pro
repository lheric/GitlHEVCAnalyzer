TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = cudisplayfilter.h
SOURCES         = cudisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libcudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


