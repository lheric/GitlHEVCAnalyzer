CONFIG += c++11

INCLUDEPATH  += ../../src \
                ../../libgitlmvc/libgitlevtbus/src

HEADERS += ../../src/views/filterconfigdialog.h \
    ../../src/views/filterconfigslider.h \
    ../../src/views/filterconfigcheckbox.h \
    ../../src/views/filterconfigradios.h \
    ../../src/views/filterconfigcombobox.h \
    ../../src/views/gitlcolorpicker.h

SOURCES += ../../src/views/filterconfigdialog.cpp \
    ../../src/views/filterconfigslider.cpp \
    ../../src/views/filterconfigcheckbox.cpp \
    ../../src/views/filterconfigradios.cpp \
    ../../src/views/filterconfigcombobox.cpp \
    ../../src/views/gitlcolorpicker.cpp

FORMS += ../../src/views/filterconfigdialog.ui \
    ../../src/views/filterconfigslider.ui \
    ../../src/views/filterconfigcheckbox.ui \
    ../../src/views/filterconfigradios.ui \
    ../../src/views/filterconfigcombobox.ui \
    ../../src/views/gitlcolorpicker.ui
