common_cppflags = -DLOCALEDIR="\"$(localedir)\""

common_ldflags = \
	-avoid-version \
	-module \
	-shared

common_libadd = ../hash/hash.la

BUILT_SOURCES = properties-resources.c properties-resources.h

CLEANFILES = properties-resources.c properties-resources.h

if ENABLE_CAJA

cajadir = $(CAJA_EXTENSION_DIR)
caja_LTLIBRARIES = libgtkhash-properties-caja.la

nodist_libgtkhash_properties_caja_la_SOURCES = \
	properties-resources.c properties-resources.h

libgtkhash_properties_caja_la_SOURCES = \
	properties.c properties.h \
	properties-hash.c properties-hash.h \
	properties-list.c properties-list.h \
	properties-prefs.c properties-prefs.h

libgtkhash_properties_caja_la_CPPFLAGS = \
	$(common_cppflags) \
	$(CAJA_CFLAGS) \
	-DIN_CAJA_EXTENSION=1

libgtkhash_properties_caja_la_CFLAGS = $(GTKHASH_CFLAGS)

libgtkhash_properties_caja_la_LDFLAGS = \
	$(common_ldflags) \
	-export-symbols-regex ^caja_module_

libgtkhash_properties_caja_la_LIBADD = \
	$(common_libadd) \
	$(CAJA_LIBS)

endif

if ENABLE_NAUTILUS

nautilusdir = $(NAUTILUS_EXTENSION_DIR)
nautilus_LTLIBRARIES = libgtkhash-properties-nautilus.la

nodist_libgtkhash_properties_nautilus_la_SOURCES = \
	properties-resources.c properties-resources.h

libgtkhash_properties_nautilus_la_SOURCES = \
	properties.c properties.h \
	properties-hash.c properties-hash.h \
	properties-list.c properties-list.h \
	properties-prefs.c properties-prefs.h

libgtkhash_properties_nautilus_la_CPPFLAGS = \
	$(common_cppcflags) \
	$(NAUTILUS_CFLAGS) \
	-DIN_NAUTILUS_EXTENSION=1

libgtkhash_properties_nautilus_la_CFLAGS = $(GTKHASH_CFLAGS)

libgtkhash_properties_nautilus_la_LDFLAGS = \
	$(common_ldflags) \
	-export-symbols-regex ^nautilus_module_

libgtkhash_properties_nautilus_la_LIBADD = \
	$(common_libadd) \
	$(NAUTILUS_LIBS)

endif

if ENABLE_NEMO

nemodir = $(NEMO_EXTENSION_DIR)
nemo_LTLIBRARIES = libgtkhash-properties-nemo.la

nodist_libgtkhash_properties_nemo_la_SOURCES = \
	properties-resources.c properties-resources.h

libgtkhash_properties_nemo_la_SOURCES = \
	properties.c properties.h \
	properties-hash.c properties-hash.h \
	properties-list.c properties-list.h \
	properties-prefs.c properties-prefs.h

libgtkhash_properties_nemo_la_CPPFLAGS = \
	$(common_cppflags) \
	$(NEMO_CFLAGS) \
	-DIN_NEMO_EXTENSION=1

libgtkhash_properties_nemo_la_CFLAGS = $(GTKHASH_CFLAGS)

libgtkhash_properties_nemo_la_LDFLAGS = \
	$(common_ldflags) \
	-export-symbols-regex ^nemo_module_

libgtkhash_properties_nemo_la_LIBADD = \
	$(common_libadd) \
	$(NEMO_LIBS)

endif

if ENABLE_THUNAR

thunardir = $(THUNAR_EXTENSION_DIR)
thunar_LTLIBRARIES = libgtkhash-properties-thunar.la

nodist_libgtkhash_properties_thunar_la_SOURCES = \
	properties-resources.c properties-resources.h

libgtkhash_properties_thunar_la_SOURCES = \
	properties.c properties.h \
	properties-hash.c properties-hash.h \
	properties-list.c properties-list.h \
	properties-prefs.c properties-prefs.h

libgtkhash_properties_thunar_la_CPPFLAGS = \
	$(common_cppflags) \
	$(THUNAR_CFLAGS) \
	-DIN_THUNAR_EXTENSION=1

libgtkhash_properties_thunar_la_CFLAGS = $(GTKHASH_CFLAGS)

libgtkhash_properties_thunar_la_LDFLAGS = \
	$(common_ldflags) \
	-export-symbols-regex ^thunar_extension_

libgtkhash_properties_thunar_la_LIBADD = \
	$(common_libadd) \
	$(THUNAR_LIBS)

endif

properties-resources.c properties-resources.h: \
	../../data/nautilus/gtkhash-properties.gresource.xml \
	../../data/nautilus/gtkhash-properties.xml
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) \
		--c-name gtkhash_properties_resources \
		--internal \
		--manual-register \
		--generate \
		--sourcedir=../../data/nautilus/ \
		--target=$@ \
		$<
