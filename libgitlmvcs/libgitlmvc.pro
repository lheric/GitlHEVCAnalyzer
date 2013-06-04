#-------------------------------------------------
#
# Project created by QtCreator 2012-10-15T02:29:45
#
#-------------------------------------------------

QT       -= gui

Debug:   TARGET = GitlMVCd
Release: TARGET = GitlMVC

DESTDIR = $${OUT_PWD}/../libs

TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
    gitlabstractcommand.cpp \
    gitlcommandparameter.cpp \
    gitlfrontcontroller.cpp

HEADERS += \
    gitlabstractcommand.h \
    gitlcommandparameter.h \
    gitlcommandrequest.h \
    gitlcommandrespond.h \
    gitlfrontcontroller.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}


INCLUDEPATH += ../libgitlevtbus/src

# libs
#debug:   LIBS += -L$${PWD}/lib -lGitlEvtBusd
#release: LIBS += -L$${PWD}/lib -lGitlEvtBus
