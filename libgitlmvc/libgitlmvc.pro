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

SOURCES += abstractcommand.cpp \
    frontcontroller.cpp \
    commandparameter.cpp

HEADERS += abstractcommand.h \
    commandrequest.h \
    commandrespond.h \
    frontcontroller.h \
    commandparameter.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/local/lib
    }
    INSTALLS += target
}


INCLUDEPATH += ../libgitlevtbus/

# libs
#debug:   LIBS += -L$${PWD}/lib -lGitlEvtBusd
#release: LIBS += -L$${PWD}/lib -lGitlEvtBus
