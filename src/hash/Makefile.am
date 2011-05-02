SUBDIRS = mutils

noinst_LTLIBRARIES = hash.la

hash_la_SOURCES = \
	hash-file.c hash-file.h \
	hash-func.c hash-func.h \
	hash-lib.c hash-lib.h \
	hash-string.c hash-string.h

if ENABLE_GCRYPT
hash_la_SOURCES += hash-lib-gcrypt.c hash-lib-gcrypt.h
endif

if ENABLE_NSS
hash_la_SOURCES += hash-lib-nss.c hash-lib-nss.h
endif

if ENABLE_GLIB_CHECKSUMS
hash_la_SOURCES += hash-lib-glib.c hash-lib-glib.h
endif

if ENABLE_LINUX_CRYPTO
hash_la_SOURCES += hash-lib-linux.c hash-lib-linux.h
endif

if ENABLE_MHASH
hash_la_SOURCES += hash-lib-mhash.c hash-lib-mhash.h
endif

hash_la_CPPFLAGS = -DIN_HASH_LIB $(GDK_CFLAGS) $(LIBGCRYPT_CFLAGS) $(NSS_CFLAGS)

hash_la_LDFLAGS = -avoid-version -module

hash_la_LIBADD = $(GDK_LIBS) $(LIBGCRYPT_LIBS) $(NSS_LIBS) $(MHASH_LIBS)
