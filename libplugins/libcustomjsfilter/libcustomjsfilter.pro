QT += script
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../analyzer \
                  ../../libgitlevtbus
HEADERS         = customjsfilter.h
SOURCES         = customjsfilter.cpp
TARGET          = $$qtLibraryTarget(libcustomjsfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


