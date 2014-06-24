TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src

HEADERS += \
    tiledisplayfilter.h

SOURCES += \
    tiledisplayfilter.cpp  \
                  ../../src/model/sequencemanager.cpp \
                  ../../src/model/common/comsequence.cpp \
                  ../../src/model/common/comframe.cpp \
                  ../../src/model/common/comcu.cpp \
                  ../../src/model/common/comtu.cpp
TARGET          = $$qtLibraryTarget(libtiledisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

