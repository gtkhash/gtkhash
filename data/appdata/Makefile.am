appdatadir = $(datadir)/appdata
appdata_DATA =

if ENABLE_GTKHASH
appdata_DATA += gtkhash.appdata.xml
endif

if ENABLE_CAJA
appdata_DATA += caja-gtkhash.metainfo.xml
endif

if ENABLE_NAUTILUS
appdata_DATA += nautilus-gtkhash.metainfo.xml
endif

if ENABLE_NEMO
appdata_DATA += nemo-gtkhash.metainfo.xml
endif

if ENABLE_THUNAR
appdata_DATA += thunar-gtkhash.metainfo.xml
endif

CLEANFILES = $(appdata_DATA)

EXTRA_DIST = \
	gtkhash.appdata.xml.in \
	caja-gtkhash.metainfo.xml.in \
	nautilus-gtkhash.metainfo.xml.in \
	nemo-gtkhash.metainfo.xml.in \
	thunar-gtkhash.metainfo.xml

@INTLTOOL_XML_RULE@
