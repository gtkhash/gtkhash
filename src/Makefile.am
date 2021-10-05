SUBDIRS = hash util

if ENABLE_NAUTILUS_DATA
SUBDIRS += nautilus
endif

if ENABLE_GTKHASH

bin_PROGRAMS = gtkhash

BUILT_SOURCES = resources.c resources.h

nodist_gtkhash_SOURCES = $(BUILT_SOURCES)

gtkhash_SOURCES = \
	main.c main.h \
	callbacks.c callbacks.h \
	check.c check.h \
	gui.c gui.h \
	hash.c hash.h \
	list.c list.h \
	prefs.c prefs.h \
	opts.c opts.h \
	uri-digest.c uri-digest.h

check_PROGRAMS = gtkhash-test

check_DATA = 0.bytes 1.bytes 10M.bytes 5G.bytes

nodist_gtkhash_test_SOURCES = $(BUILT_SOURCES)

gtkhash_test_SOURCES = \
	test.c \
	callbacks.c \
	check.c \
	gui.c \
	hash.c \
	list.c \
	opts.c \
	uri-digest.c

AM_CPPFLAGS = \
	-DLOCALEDIR="\"$(localedir)\"" \
	$(GTK_CFLAGS)

AM_CFLAGS = $(GTKHASH_CFLAGS)

LDADD = \
	hash/hash.la \
	util/util.la \
	$(GTK_LIBS)

TESTS = $(check_PROGRAMS)

AM_TESTS_ENVIRONMENT = HASH_LIBS="$(HASH_LIBS)"; export HASH_LIBS;

LOG_COMPILER = xvfb-run --auto-servernum -s "-screen 0 800x600x24" $(srcdir)/gtkhash-test.sh

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(check_DATA) \
	*.gcda *.gcno

EXTRA_DIST = gtkhash-test.sh test.md5sum.in

resources.c resources.h: \
	../data/gtkhash-gtk3.gresource.xml \
	../data/gtkhash-gtk3.ui
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) \
		--c-name resources \
		--internal \
		--manual-register \
		--generate \
		--sourcedir="$(srcdir)"/../data/ \
		--target=$@ \
		$<

%.bytes:
	$(AM_V_GEN) truncate -s $* $@

endif # ENABLE_GTKHASH
