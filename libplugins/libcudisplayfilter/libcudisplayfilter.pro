greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin

TARGET          = $$qtLibraryTarget(libcudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = cudisplayfilter.h
SOURCES         = cudisplayfilter.cpp \
                  ../../src/model/sequencemanager.cpp \
                  ../../src/model/common/comsequence.cpp \
                  ../../src/model/common/comframe.cpp \
                  ../../src/model/common/comcu.cpp \
                  ../../src/model/common/comtu.cpp \
                  ../../src/model/selectionmanager.cpp

include(../filterconfiggui.pri)

