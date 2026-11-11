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

# --- TLibSysuAnalyzer (analyzer output for the GUI) -------------------------
# HM is an unmodified upstream submodule; the analyzer hooks live in
# sysu-analyzer.patch (same hook points as the modified HM-12.0 sources) and
# are applied to the submodule's working tree here, once. To move HM to
# another tag: git -C HM checkout -- . , switch the tag, re-run qmake, and
# refresh the patch if it no longer applies.
HM_ANALYZER_DIR = $$clean_path($$PWD/../TLibSysuAnalyzer)
HM_PATCH        = $$PWD/sysu-analyzer.patch
HM_GIT_APPLY    = git -C $$shell_quote($$PWD/HM) apply --ignore-whitespace
!system("$$HM_GIT_APPLY --reverse --check $$shell_quote($$HM_PATCH) 2> $$QMAKE_SYSTEM_NULL_DEVICE") {
    !system("$$HM_GIT_APPLY $$shell_quote($$HM_PATCH)") {
        error("Could not apply $$HM_PATCH to $$PWD/HM. Reset it with: git -C $$PWD/HM checkout -- .")
    }
    message("Applied TLibSysuAnalyzer hooks (sysu-analyzer.patch) to HM-18.0")
}
DEFINES     += ENABLE_ANAYSIS_OUTPUT=1 HM_VERSION=180
INCLUDEPATH += $$clean_path($$PWD/..)    # for "TLibSysuAnalyzer/TSysuAnalyzerOutput.h"

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
