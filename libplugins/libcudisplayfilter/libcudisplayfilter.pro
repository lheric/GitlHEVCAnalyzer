TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus/src
HEADERS         = cudisplayfilter.h
SOURCES         = cudisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libcudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


