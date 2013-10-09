TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = intradisplayfilter.h
SOURCES         = intradisplayfilter.cpp
TARGET          = $$qtLibraryTarget(libintradisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


