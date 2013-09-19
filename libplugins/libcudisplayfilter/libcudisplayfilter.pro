TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = cudisplayfilter.h
SOURCES         = cudisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libcudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


