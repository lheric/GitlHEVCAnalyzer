greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin
CONFIG         += c++11

TARGET          = $$qtLibraryTarget(libbitdisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

HEADERS         = bitdisplayfilter.h
SOURCES         = bitdisplayfilter.cpp \
                  ../../src/model/sequencemanager.cpp \
                  ../../src/model/common/comsequence.cpp \
                  ../../src/model/common/comframe.cpp \
                  ../../src/model/common/comcu.cpp \
                  ../../src/model/common/comtu.cpp

include(../filterconfiggui.pri)
