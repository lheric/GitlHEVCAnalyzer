TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = nzmvdisplayfilter.h
SOURCES         = nzmvdisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libnzmvdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


