TEMPLATE = subdirs
SUBDIRS = libtudisplayfilter \          #tu structure filter
          libcudisplayfilter \          #cu structure filter
          libmergedisplayfilter \       #merge display filter
          libmvdisplayfilter \          #MV display filter
          libintradisplayfilter \       #intra mode display
          libpreddisplayfilter \        #pred mode display
          libbitdisplayfilter \ 	#bit heatmap display
          GitlHEVCAnalyzerPrivateFilter/libmccdisplayfilter \         #mcc display
          GitlHEVCAnalyzerPrivateFilter/libeventdetectionfilter \     #event detection
          GitlHEVCAnalyzerPrivateFilter/libobjecttrackingfilter       #object tracking filter
