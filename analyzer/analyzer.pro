#-------------------------------------------------
#
# Project created by QtCreator 2012-05-31T04:32:35
#
#-------------------------------------------------

# extended initializer syntax is only available in C++0x

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = analyzer

CONFIG -= app_bundle
CONFIG -= console
CONFIG += static
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    common/comsequence.cpp \
    common/comframe.cpp \
    common/comlcu.cpp \
    drawengine/drawengine.cpp \
    appgui/mainwindow.cpp \
    io/ioyuv.cpp \
    parsers/meparser.cpp \
    model/modellocator.cpp \
    commands/openyuvcommand.cpp \
    commands/opencupucommand.cpp \
    commands/openmecommand.cpp \
    commands/nextframecommand.cpp \
    commands/prevframecommand.cpp \
    commands/jumptoframecommand.cpp \
    commands/appfrontcontroller.cpp \
    appgui/bitstreamversionselector.cpp \
    parsers/bitstreamparser.cpp \
    parsers/encodergeneralparser.cpp \
    parsers/decodergeneralparser.cpp \
    commands/openencodergeneralcommand.cpp \
    commands/decodebitstreamcommand.cpp \
    commands/opendecodergeneralcommand.cpp \
    parsers/spsparser.cpp \
    parsers/cupuparser.cpp \
    parsers/predparser.cpp \
    parsers/mvparser.cpp \
    parsers/mergeparser.cpp \
    parsers/intraparser.cpp \
    io/yuv420rgbbuffer.cpp \
    commands/switchsequencecommand.cpp \
    appgui/frameview.cpp \
    drawengine/filterloader.cpp \
    commands/printscreencommand.cpp \
    appgui/busydialog.cpp \
    io/analyzermsgsender.cpp \
    appgui/msgviewer.cpp \
    appgui/aboutdialog.cpp \
    commands/jumptopercentcommand.cpp \
    appgui/pluginfilterlist.cpp \
    appgui/pluginfilteritem.cpp \
    commands/refreshscreencommand.cpp \
    appgui/sequencelist.cpp \
    appgui/sequencelistitem.cpp \
    commands/zoomframecommand.cpp \
    commands/configfiltercommand.cpp \
    common/comrom.cpp \
    commands/filterorderupcommand.cpp \
    commands/filterorderdowncommand.cpp \
    commands/checkupdatecommand.cpp

HEADERS += \
    common/comsequence.h \
    common/comframe.h \
    drawengine/drawengine.h \
    appgui/mainwindow.h \
    io/ioyuv.h \
    parsers/meparser.h \
    model/modellocator.h \
    common/commv.h \
    commands/openyuvcommand.h \
    commands/opencupucommand.h \
    commands/openmecommand.h \
    commands/nextframecommand.h \
    commands/prevframecommand.h \
    commands/jumptoframecommand.h \
    commands/appfrontcontroller.h \
    appgui/bitstreamversionselector.h \
    parsers/bitstreamparser.h \
    parsers/encodergeneralparser.h \
    parsers/decodergeneralparser.h \
    commands/openencodergeneralcommand.h \
    commands/decodebitstreamcommand.h \
    commands/opendecodergeneralcommand.h \
    parsers/spsparser.h \
    parsers/cupuparser.h \
    parsers/predparser.h \
    parsers/mvparser.h \
    parsers/mergeparser.h \
    parsers/intraparser.h \
    io/yuv420rgbbuffer.h \
    commands/switchsequencecommand.h \
    appgui/frameview.h \
    drawengine/filterloader.h \
    commands/printscreencommand.h \
    appgui/busydialog.h \
    io/analyzermsgsender.h \
    events/eventnames.h \
    appgui/msgviewer.h \
    appgui/aboutdialog.h \
    drawengine/abstractfilter.h \
    exceptions/nosequencefoundexception.h \
    commands/jumptopercentcommand.h \
    exceptions/invaildfilterindexexception.h \
    appgui/pluginfilterlist.h \
    appgui/pluginfilteritem.h \
    common/compu.h \
    commands/refreshscreencommand.h \
    appgui/sequencelist.h \
    appgui/sequencelistitem.h \
    commands/zoomframecommand.h \
    common/comcu.h \
    exceptions/decodernotfoundexception.h \
    exceptions/decodingfailexception.h \
    exceptions/bitstreamnotfoundexception.h \
    commands/configfiltercommand.h \
    common/comrom.h \
    commands/filterorderupcommand.h \
    commands/filterorderdowncommand.h \
    commands/checkupdatecommand.h

#include & libs
INCLUDEPATH += . \
               ../libgitlevtbus/src \
               ../libgitlmvc/src    \
               ../3rdparty/WinSparkle-0.3/include

LIBS += -L$${OUT_PWD}/../libgitlevtbus -L$${OUT_PWD}/../libgitlmvc -L$${PWD}/../3rdparty/WinSparkle-0.3/lib

Debug:   LIBS += -lGitlMVCd -lGitlEvtBusd -lWinSparkle
Release: LIBS += -lGitlMVC  -lGitlEvtBus  -lWinSparkle


FORMS += \
    appgui/mainwindow.ui \
    appgui/bitstreamversionselector.ui \
    appgui/busydialog.ui \
    appgui/aboutdialog.ui \
    appgui/pluginfilteritem.ui \
    appgui/sequencelist.ui \
    appgui/sequencelistitem.ui

#icon
RC_FILE = icons/appicon.rc

RESOURCES += \
    resources/resources.qrc
