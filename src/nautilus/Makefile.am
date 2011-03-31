nautilusdir = $(NAUTILUS_EXTENSION_DIR)
nautilus_LTLIBRARIES = libgtkhash-properties.la

libgtkhash_properties_la_SOURCES = \
	properties.c properties.h \
	properties-hash.c properties-hash.h \
	properties-list.c properties-list.h \
	properties-prefs.c properties-prefs.h

libgtkhash_properties_la_CPPFLAGS = \
	$(NAUTILUS_CFLAGS) $(GTK_CFLAGS) $(GCONF_CFLAGS) \
	-DBUILDER_XML="\"$(pkgdatadir)/nautilus/gtkhash-properties.xml\"" \
	-DLOCALEDIR="\"$(localedir)\""

libgtkhash_properties_la_LDFLAGS = \
	-avoid-version \
	-export-symbols-regex ^nautilus_module_ \
	-module \
	-shared

libgtkhash_properties_la_LIBADD = \
	../hash/hash.la $(NAUTILUS_LIBS) $(GTK_LIBS) $(GCONF_LIBS)
