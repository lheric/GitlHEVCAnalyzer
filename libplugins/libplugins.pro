TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = libhighlightlcufilter \       #highlight lcu filter
          libhighlightdifffilter \      #highlight difference filter
          libcudisplayfilter \          #cu stucture filter
          libmergedisplayfilter \       #merge display filter
          libmvdisplayfilter \          #MV display filter
          libmccdisplayfilter \         #Motion Collision Count display filter
          libnzmvdisplayfilter \        #Non-zero Fractional MV display filter
          librdgaindisplayfilter        #R-D gain display filter for block level


