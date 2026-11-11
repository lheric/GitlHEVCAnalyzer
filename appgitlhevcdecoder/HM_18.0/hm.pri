# Common qmake settings for the HM-18.0 decoder sub-projects.
# Mirrors HM's own CMake build (HM/CMakeLists.txt and the per-directory
# CMakeLists.txt): source lists are directory globs, exactly like HM's
# file(GLOB ...), so moving the HM submodule to a newer tag normally needs
# no changes here.

HM_SRC    = $$PWD/HM/source
HM_LIBDIR = $$shadowed($$PWD)

!exists($$HM_SRC/Lib/TLibCommon/CommonDef.h) {
    error("HM sources missing in $$PWD/HM - run: git submodule update --init --recursive")
}

CONFIG += c++11 thread
CONFIG -= qt app_bundle

INCLUDEPATH += \
    $$HM_SRC/Lib \
    $$HM_SRC/Lib/TLibCommon \
    $$HM_SRC/Lib/libmd5 \
    $$HM_SRC/Lib/TLibDecoder \
    $$HM_SRC/Lib/Utilities

msvc {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    QMAKE_CXXFLAGS += /wd4996
    exists($$PWD/HM/compat/msvc): INCLUDEPATH += $$PWD/HM/compat/msvc
} else {
    # Same as HM's CMake: -Wno-sign-compare, and -msse4.1 on x86 only.
    QMAKE_CXXFLAGS_WARN_ON += -Wno-sign-compare
    contains(QT_ARCH, x86_64)|contains(QT_ARCH, i386): QMAKE_CXXFLAGS += -msse4.1
}

# Library name with HM-12.0's "d" suffix for debug builds.
defineReplace(hmLibName) {
    CONFIG(debug, debug|release): return($${1}d)
    return($$1)
}

# Static library sub-project: HM_LIB_SOURCES are globs relative to HM_SRC.
defineTest(hmStaticLib) {
    TEMPLATE = lib
    CONFIG  += staticlib
    TARGET   = $$hmLibName($$1)
    DESTDIR  = $$HM_LIBDIR
    for(pattern, HM_LIB_SOURCES) {
        SOURCES += $$files($$HM_SRC/$${pattern}.cpp)
        HEADERS += $$files($$HM_SRC/$${pattern}.h)
    }
    isEmpty(SOURCES): error("No sources found for $$1 in $$HM_SRC")
    export(TEMPLATE)
    export(CONFIG)
    export(TARGET)
    export(DESTDIR)
    export(SOURCES)
    export(HEADERS)
    return(true)
}
