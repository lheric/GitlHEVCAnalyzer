greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin

TARGET          = $$qtLibraryTarget(libtiledisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS +=  tiledisplayfilter.h

SOURCES +=  tiledisplayfilter.cpp  \
            ../../src/model/sequencemanager.cpp \
            ../../src/model/common/comsequence.cpp \
            ../../src/model/common/comframe.cpp \
            ../../src/model/common/comcu.cpp \
            ../../src/model/common/comtu.cpp


include(../filterconfiggui.pri)
