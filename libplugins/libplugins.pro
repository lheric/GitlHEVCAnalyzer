TEMPLATE = subdirs

#these are public filters
SUBDIRS = libtudisplayfilter \          #tu structure filter
          libcudisplayfilter \          #cu structure filter
          libmergedisplayfilter \       #merge display filter
          libmvdisplayfilter \          #MV display filter
          libintradisplayfilter \       #intra mode display
          libpreddisplayfilter \        #pred mode display
          libbitdisplayfilter   	#bit heatmap display

#private filters for internal usage
EXTRA_PRIVATE_FILTER {
    message("private filters included")
    SUBDIRS += \
              GitlHEVCAnalyzerPrivateFilter/libfinerbitdisplayfilter \
              GitlHEVCAnalyzerPrivateFilter/libmccdisplayfilter \         #mcc display
              GitlHEVCAnalyzerPrivateFilter/libeventdetectionfilter \     #event detection
              GitlHEVCAnalyzerPrivateFilter/libobjecttrackingfilter       #object tracking filter
} else {
    message("private filters excluded")
}
