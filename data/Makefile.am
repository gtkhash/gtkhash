SUBDIRS =

if ENABLE_NAUTILUS_DATA
SUBDIRS += nautilus
endif

if ENABLE_GTKHASH
pkgdata_DATA = gtkhash.xml.gz
EXTRA_DIST = gtkhash.xml gtkhash.xml.gz
CLEANFILES = $(pkgdata_DATA)
endif

gtkhash.xml.gz: gtkhash.xml
	gzip -c9 $< > $@
