cajadir = $(CAJA_EXTENSION_DIR)
caja_LTLIBRARIES = libgtkhash-properties-caja.la

libgtkhash_properties_caja_la_SOURCES = \
	../nautilus/properties.c ../nautilus/properties.h \
	../nautilus/properties-hash.c ../nautilus/properties-hash.h \
	../nautilus/properties-list.c ../nautilus/properties-list.h \
	../nautilus/properties-prefs.c ../nautilus/properties-prefs.h

libgtkhash_properties_caja_la_CPPFLAGS = \
	$(CAJA_CFLAGS) \
	-DDATADIR="\"$(pkgdatadir)/nautilus\"" \
	-DLOCALEDIR="\"$(localedir)\"" \
	-DIN_CAJA_EXTENSION=1

libgtkhash_properties_caja_la_CFLAGS = $(GTKHASH_CFLAGS)

libgtkhash_properties_caja_la_LDFLAGS = \
	-avoid-version \
	-export-symbols-regex ^caja_module_ \
	-module \
	-shared

libgtkhash_properties_caja_la_LIBADD = \
	../hash/hash.la $(CAJA_LIBS)
