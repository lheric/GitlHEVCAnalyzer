TEMPLATE        = lib
CONFIG         += plugin
CONFIG         += c++11
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = bitdisplayfilter.h
SOURCES         = bitdisplayfilter.cpp \
                  ../../src/model/sequencemanager.cpp \
                  ../../src/model/common/comsequence.cpp \
                  ../../src/model/common/comframe.cpp \
                  ../../src/model/common/comcu.cpp \
                  ../../src/model/common/comtu.cpp
TARGET          = $$qtLibraryTarget(libbitdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


