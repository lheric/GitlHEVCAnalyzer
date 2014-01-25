greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE        = lib
CONFIG         += plugin
INCLUDEPATH    += ../../src \
                  ../../libgitlmvc/libgitlevtbus/src
HEADERS         = intradisplayfilter.h \
                ../../src/views/filterconfigdialog.h \
                ../../src/views/filterconfigslider.h \
                ../../src/views/filterconfigcheckbox.h \
                ../../src/views/filterconfigradios.h \
                ../../src/views/gitlcolorpicker.h
SOURCES         = intradisplayfilter.cpp \
                ../../src/views/filterconfigdialog.cpp \
                ../../src/views/filterconfigslider.cpp \
                ../../src/views/filterconfigcheckbox.cpp \
                ../../src/views/filterconfigradios.cpp \
                ../../src/views/gitlcolorpicker.cpp
TARGET          = $$qtLibraryTarget(libintradisplayfilter)
DESTDIR         = $${OUT_PWD}/../../plugins


FORMS += \
    ../../src/views/filterconfigdialog.ui \
    ../../src/views/filterconfigslider.ui \
    ../../src/views/filterconfigcheckbox.ui \
    ../../src/views/filterconfigradios.ui \
    ../../src/views/gitlcolorpicker.ui
