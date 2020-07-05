appdatadir = $(datadir)/metainfo
appdata_DATA =

if ENABLE_GTKHASH
appdata_DATA += org.gtkhash.gtkhash.appdata.xml
endif

if ENABLE_CAJA
appdata_DATA += org.gtkhash.caja.metainfo.xml
endif

if ENABLE_NAUTILUS
appdata_DATA += org.gtkhash.nautilus.metainfo.xml
endif

if ENABLE_NEMO
appdata_DATA += org.gtkhash.nemo.metainfo.xml
endif

if ENABLE_PEONY
appdata_DATA += org.gtkhash.peony.metainfo.xml
endif

if ENABLE_THUNAR
appdata_DATA += org.gtkhash.thunar.metainfo.xml
endif

CLEANFILES = $(appdata_DATA)

EXTRA_DIST = \
	org.gtkhash.gtkhash.appdata.xml.in \
	org.gtkhash.caja.metainfo.xml.in \
	org.gtkhash.nautilus.metainfo.xml.in \
	org.gtkhash.nemo.metainfo.xml.in \
	org.gtkhash.peony.metainfo.xml.in \
	org.gtkhash.thunar.metainfo.xml.in

@INTLTOOL_XML_RULE@
