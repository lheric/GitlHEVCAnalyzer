include(../hm.pri)

# HM: source/App/TAppDecoder/CMakeLists.txt
TEMPLATE = app
CONFIG  += console
TARGET   = HM_180
DESTDIR  = $$shadowed($$PWD/../../..)/decoders

SOURCES += $$files($$HM_SRC/App/TAppDecoder/*.cpp)
HEADERS += $$files($$HM_SRC/App/TAppDecoder/*.h)
INCLUDEPATH += $$HM_SRC/App/TAppDecoder

HM_LIBS = TLibDecoder TLibSysuAnalyzer Utilities TLibCommon
LIBS += -L$$HM_LIBDIR
for(lib, HM_LIBS) {
    name = $$hmLibName($$lib)
    LIBS += -l$$name
    PRE_TARGETDEPS += $$HM_LIBDIR/$${QMAKE_PREFIX_STATICLIB}$${name}.$${QMAKE_EXTENSION_STATICLIB}
}
