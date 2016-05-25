SUBDIRS = hash

if ENABLE_NAUTILUS_DATA
SUBDIRS += nautilus
endif

if ENABLE_GTKHASH

bin_PROGRAMS = gtkhash

BUILT_SOURCES = resources.c resources.h

nodist_gtkhash_SOURCES = resources.c resources.h

gtkhash_SOURCES = \
	callbacks.c callbacks.h \
	gui.c gui.h \
	hash.c hash.h \
	list.c list.h \
	main.c main.h \
	prefs.c prefs.h

gtkhash_CPPFLAGS = \
	-DLOCALEDIR="\"$(localedir)\"" \
	$(GTK_CFLAGS)

gtkhash_CFLAGS = $(GTKHASH_CFLAGS)

gtkhash_LDADD = \
	hash/hash.la \
	$(GTK_LIBS)

CLEANFILES = resources.c resources.h

if WITH_GTK2
resources.c resources.h: \
	../data/gtkhash-gtk2.gresource.xml \
	../data/gtkhash-gtk2.xml
endif
if WITH_GTK3
resources.c resources.h: \
	../data/gtkhash-gtk3.gresource.xml \
	../data/gtkhash-gtk3.xml
endif
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) \
		--c-name resources \
		--internal \
		--manual-register \
		--generate \
		--sourcedir="$(srcdir)"/../data/ \
		--target=$@ \
		$<

endif # ENABLE_GTKHASH
