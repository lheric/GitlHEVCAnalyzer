include(../hm.pri)

# HM: source/App/TAppDecoder/CMakeLists.txt
TEMPLATE = app
CONFIG  += console
TARGET   = HM_180
# <build root>/decoders in a full build; when HM_18.0.pro is built on its own,
# shadowed() of a path outside the source root is empty, so use the HM build dir.
HM_DECODER_ROOT = $$shadowed($$PWD/../../..)
isEmpty(HM_DECODER_ROOT): HM_DECODER_ROOT = $$HM_LIBDIR
DESTDIR  = $$HM_DECODER_ROOT/decoders

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
