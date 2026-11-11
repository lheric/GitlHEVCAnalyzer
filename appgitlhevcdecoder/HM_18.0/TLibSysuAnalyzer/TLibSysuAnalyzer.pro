include(../hm.pri)

# Analyzer output library shared by all HM-x.y decoders
# (appgitlhevcdecoder/TLibSysuAnalyzer). TComSysuCuMDTools is an encoder-side
# helper that the decoder never calls, so only the output class is built.
TEMPLATE = lib
CONFIG  += staticlib
TARGET   = $$hmLibName(TLibSysuAnalyzer)
DESTDIR  = $$HM_LIBDIR

SOURCES += $$HM_ANALYZER_DIR/TSysuAnalyzerOutput.cpp
HEADERS += $$HM_ANALYZER_DIR/TSysuAnalyzerOutput.h
