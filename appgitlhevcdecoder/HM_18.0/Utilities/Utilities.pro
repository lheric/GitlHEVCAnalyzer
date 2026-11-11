include(../hm.pri)

# HM: source/Lib/Utilities/CMakeLists.txt
# (replaces TLibVideoIO + TAppCommon from older HM versions)
HM_LIB_SOURCES = Lib/Utilities/*
hmStaticLib(Utilities)
