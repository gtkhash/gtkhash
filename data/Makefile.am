SUBDIRS =
pkgdata_DATA =

if ENABLE_NAUTILUS_DATA
SUBDIRS += nautilus
endif

if ENABLE_GTKHASH

if WITH_GTK2
pkgdata_DATA += gtkhash-gtk2.xml.gz
endif

if WITH_GTK3
pkgdata_DATA += gtkhash-gtk3.xml.gz
endif

applicationsdir = $(datadir)/applications
applications_DATA = gtkhash.desktop

gsettings_SCHEMAS = org.gtkhash.gschema.xml

EXTRA_DIST = \
	gtkhash-gtk2.xml gtkhash-gtk3.xml \
	$(applications_DATA).in \
	$(gsettings_SCHEMAS)

CLEANFILES = \
	$(pkgdata_DATA) \
	$(applications_DATA)

endif

gtkhash-gtk2.xml.gz: gtkhash-gtk2.xml
	$(AM_V_GEN) gzip -nc9 $< > $@

gtkhash-gtk3.xml.gz: gtkhash-gtk3.xml
	$(AM_V_GEN) gzip -nc9 $< > $@

@INTLTOOL_DESKTOP_RULE@
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@
