SUBDIRS = mutils

noinst_LTLIBRARIES = hash.la

hash_la_SOURCES = \
	hash-file.c hash-file.h \
	hash-func.c hash-func.h \
	hash-lib.c hash-lib.h \
	hash-lib-glib.c hash-lib-glib.h \
	hash-lib-mhash.c hash-lib-mhash.h \
	hash-string.c hash-string.h

hash_la_CPPFLAGS = -DIN_HASH_LIB $(GDK_CFLAGS)

hash_la_LDFLAGS = -avoid-version -module

hash_la_LIBADD = $(GDK_LIBS) $(MHASH_LIBS)
