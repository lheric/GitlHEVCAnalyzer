# HM-18.0 reference decoder, built with qmake.
# HM itself is the git submodule in ./HM (tag HM-18.0); the .pro files here
# replace HM's CMake build for the decoder only.
#
# Output: <build>/decoders/HM_180 (same place/naming as the other HM-x.y).

TEMPLATE = subdirs
CONFIG  += ordered
SUBDIRS  = \
    TLibCommon \
    TLibDecoder \
    Utilities \
    TAppDecoder
