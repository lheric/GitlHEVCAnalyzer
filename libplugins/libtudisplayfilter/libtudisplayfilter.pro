greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin
TARGET          = $$qtLibraryTarget(libtudisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins

INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src

HEADERS         = \
    tudisplayfilter.h \
    ../../src/views/filterconfigdialog.h \
    ../../src/views/filterconfigslider.h \
    ../../src/views/filterconfigcheckbox.h \
    ../../src/views/filterconfigradios.h \
    ../../src/views/gitlcolorpicker.h

SOURCES         = \
    tudisplayfilter.cpp \
    ../../src/views/filterconfigdialog.cpp \
    ../../src/views/filterconfigslider.cpp \
    ../../src/views/filterconfigcheckbox.cpp \
    ../../src/views/filterconfigradios.cpp \
    ../../src/views/gitlcolorpicker.cpp

FORMS += ../../src/views/filterconfigdialog.ui \
    ../../src/views/filterconfigslider.ui \
    ../../src/views/filterconfigcheckbox.ui \
    ../../src/views/filterconfigradios.ui \
    ../../src/views/gitlcolorpicker.ui
