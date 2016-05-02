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

gsettings_SCHEMAS = org.gtkhash.gschema.xml
EXTRA_DIST = gtkhash-gtk2.xml gtkhash-gtk3.xml $(gsettings_SCHEMAS)
CLEANFILES = $(pkgdata_DATA)

endif

gtkhash-gtk2.xml.gz: gtkhash-gtk2.xml
	$(AM_V_GEN) gzip -c9 $< > $@

gtkhash-gtk3.xml.gz: gtkhash-gtk3.xml
	$(AM_V_GEN) gzip -c9 $< > $@

@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@
