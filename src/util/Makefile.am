noinst_LTLIBRARIES = util.la

util_la_SOURCES = util.c util.h

util_la_CPPFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS)

util_la_CFLAGS = $(GTKHASH_CFLAGS)

util_la_LDFLAGS = -avoid-version -module

util_la_LIBADD = \
	$(GLIB_LIBS) \
	$(GIO_LIBS)

CLEANFILES = *.gcda *.gcno
