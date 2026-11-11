include(../hm.pri)

# Analyzer output (decoder_*.txt, pred_yuv.yuv, resi_yuv.yuv) for the GUI,
# called from the hooks in ../sysu-analyzer.patch.
TEMPLATE = lib
CONFIG  += staticlib
TARGET   = $$hmLibName(TLibSysuAnalyzer)
DESTDIR  = $$HM_LIBDIR

SOURCES += TSysuAnalyzerOutput.cpp
HEADERS += TSysuAnalyzerOutput.h
