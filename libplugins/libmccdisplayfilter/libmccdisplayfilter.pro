TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = mccdisplayfilter.h
SOURCES         = mccdisplayfilter.cpp \
                  ../../src/model/sequencemanager.cpp \
                  ../../src/model/common/comsequence.cpp \
                  ../../src/model/common/comframe.cpp \
                  ../../src/model/common/comcu.cpp \
                  ../../src/model/common/comtu.cpp
TARGET          = $$qtLibraryTarget(libmccdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


