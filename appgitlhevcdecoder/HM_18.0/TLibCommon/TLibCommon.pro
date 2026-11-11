include(../hm.pri)

# HM: source/Lib/TLibCommon/CMakeLists.txt (also builds libmd5)
HM_LIB_SOURCES = Lib/TLibCommon/* Lib/libmd5/*
hmStaticLib(TLibCommon)
