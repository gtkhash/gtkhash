SUBDIRS = hash

if ENABLE_NAUTILUS
SUBDIRS += nautilus
endif

bin_PROGRAMS = gtkhash

gtkhash_SOURCES = \
	callbacks.c callbacks.h \
	gui.c gui.h \
	hash.c hash.h \
	list.c list.h \
	main.c main.h \
	prefs.c prefs.h

gtkhash_CPPFLAGS = \
	$(GTHREAD_CFLAGS) $(GTK_CFLAGS) \
	-DDATADIR="\"$(pkgdatadir)\"" \
	-DLOCALEDIR="\"$(localedir)\""

gtkhash_LDADD = hash/hash.la $(GTHREAD_LIBS) $(GTK_LIBS)
