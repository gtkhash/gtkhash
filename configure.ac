AC_PREREQ([2.61])
AC_INIT([GtkHash], [1.0.1])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 check-news dist-xz foreign subdir-objects])
AM_SILENT_RULES([yes])

AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT

GTKHASH_CFLAGS=""

for opt in \
	-Werror=unknown-warning-option \
	-Werror=implicit-function-declaration \
	-fno-common \
	-fvisibility=hidden \
	-pedantic \
	-Wall \
	-Wextra \
	-Waggregate-return \
	-Wbad-function-cast \
	-Wcast-align \
	-Winit-self \
	-Wfloat-equal \
	-Wlogical-op \
	-Wmissing-declarations \
	-Wmissing-noreturn \
	-Wredundant-decls \
	-Wshadow \
	-Wswitch-default \
	-Wwrite-strings \
	-Wno-parentheses-equality
do
	AC_MSG_CHECKING([whether ${CC} accepts option ${opt}])
	ac_save_CFLAGS="${CFLAGS}"
	CFLAGS="${GTKHASH_CFLAGS} ${opt} ${CFLAGS}"
	AC_COMPILE_IFELSE(
		[AC_LANG_PROGRAM([], [])],
		[AC_MSG_RESULT([yes]) ; GTKHASH_CFLAGS="${GTKHASH_CFLAGS} ${opt}"],
		[AC_MSG_RESULT([no])])
	CFLAGS="${ac_save_CFLAGS}"
done

AC_SUBST([GTKHASH_CFLAGS])

# check for pkg-config
PKG_PROG_PKG_CONFIG

# check for glib/gio {{{
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.36])
AC_SUBST([GLIB_CFLAGS])
AC_SUBST([GLIB_LIBS])

PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.36])
AC_SUBST([GIO_CFLAGS])
AC_SUBST([GIO_LIBS])

m4_ifdef([PKG_CHECK_VAR],
	[PKG_CHECK_VAR([GLIB_COMPILE_RESOURCES], [gio-2.0], [glib_compile_resources])],
	[AC_CHECK_PROGS([GLIB_COMPILE_RESOURCES], [glib-compile-resources])])
AC_SUBST([GLIB_COMPILE_RESOURCES])

GLIB_GSETTINGS
# }}}

# check for gtk+ {{{
AC_MSG_CHECKING([which GTK+ version to use])
AC_ARG_WITH([gtk],
	[AS_HELP_STRING([--with-gtk=2.0|3.0], [GTK+ version to use [3.0]])],
	[with_gtk="${withval}"], [with_gtk="3.0"])
AC_MSG_RESULT([${with_gtk}])

PKG_CHECK_MODULES([GTK], ["gtk+-${with_gtk}" >= 2.24])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

AM_CONDITIONAL([WITH_GTK2], [test "${with_gtk}" = "2.0"])
AM_CONDITIONAL([WITH_GTK3], [test "${with_gtk}" = "3.0"])
# }}}

# debug option {{{
AC_MSG_CHECKING([whether to enable debugging])
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [enable debugging])],
	[enable_debug="${enableval}"], [enable_debug="no"])
AC_MSG_RESULT([${enable_debug}])

if test "${enable_debug}" = "yes" ; then
	AC_DEFINE([G_DISABLE_DEPRECATED], [1], [disable glib deprecated])
	AC_DEFINE([GDK_DISABLE_DEPRECATED], [1], [disable gdk deprecated])
	AC_DEFINE([GTK_DISABLE_DEPRECATED], [1], [disable gtk+ deprecated])
else
	AC_DEFINE([G_DISABLE_ASSERT], [1], [disable glib assert])
	AC_DEFINE([G_DISABLE_CAST_CHECKS], [1], [disable glib cast checks])
fi
# }}}

# blake2 option {{{
AC_MSG_CHECKING([whether to use libb2])
AC_ARG_ENABLE([blake2],
	[AS_HELP_STRING([--enable-blake2], [use libb2])],
	[enable_blake2="${enableval}"], [enable_blake2="no"])
AC_MSG_RESULT([${enable_blake2}])
AM_CONDITIONAL([ENABLE_BLAKE2], [test "${enable_blake2}" = "yes"])
AC_DEFINE([ENABLE_BLAKE2], [0], [Use libb2])

if test "${enable_blake2}" = "yes" ; then
	AC_CHECK_HEADER([blake2.h], [:], [AC_MSG_ERROR([blake2 header not found])])
	AC_CHECK_LIB([b2], [blake2sp_init], [:], [AC_MSG_ERROR([blake2 library not found])])
	BLAKE2_LIBS="-lb2"
	AC_SUBST([BLAKE2_LIBS])
	AC_DEFINE([ENABLE_BLAKE2], [1])
fi
# }}}

# gcrypt option {{{
AC_MSG_CHECKING([whether to use Libgcrypt])
AC_ARG_ENABLE([gcrypt],
	[AS_HELP_STRING([--enable-gcrypt], [use Libgcrypt])],
	[enable_gcrypt="${enableval}"], [enable_gcrypt="no"])
AC_MSG_RESULT([${enable_gcrypt}])
AM_CONDITIONAL([ENABLE_GCRYPT], [test "${enable_gcrypt}" = "yes"])
AC_DEFINE([ENABLE_GCRYPT], [0], [Use Libgcrypt])

if test "${enable_gcrypt}" = "yes" ; then
	AM_PATH_LIBGCRYPT([1.7.0], [:], [AC_MSG_ERROR([Libgcrypt not found])])
	AC_DEFINE([ENABLE_GCRYPT], [1])
fi
# }}}

# glib checksums option {{{
AC_MSG_CHECKING([whether to use the Glib Data Checksums API])
AC_ARG_ENABLE([glib-checksums],
	[AS_HELP_STRING([--disable-glib-checksums], [do not use the Glib Data Checksums API])],
	[enable_glib_checksums="${enableval}"], [enable_glib_checksums="yes"])
AC_MSG_RESULT([${enable_glib_checksums}])
AM_CONDITIONAL([ENABLE_GLIB_CHECKSUMS], [test "${enable_glib_checksums}" = "yes"])
AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [0], [Use Glib Checksums])

if test "${enable_glib_checksums}" = "yes" ; then
	AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [1])
fi
# }}}


# internal md6 option {{{
AC_MSG_CHECKING([whether to use the internal MD6 library])
AC_ARG_ENABLE([internal-md6],
	[AS_HELP_STRING([--disable-internal-md6], [do not use the internal MD6 library])],
	[enable_internal_md6="${enableval}"], [enable_internal_md6="yes"])
AC_MSG_RESULT([${enable_internal_md6}])
AM_CONDITIONAL([ENABLE_MD6], [test "${enable_internal_md6}" = "yes"])
AC_DEFINE([ENABLE_MD6], [0], [Use MD6])

if test "${enable_internal_md6}" = "yes" ; then
	AC_DEFINE([ENABLE_MD6], [1])
fi
# }}}

# libcrypto (openssl) option {{{
#
# Disabled until a thread-safe API is released with openssl-1.1
enable_libcrypto=no
#
#AC_MSG_CHECKING([whether to use libcrypto])
#AC_ARG_ENABLE([libcrypto],
#	[AS_HELP_STRING([--enable-libcrypto], [use libcrypto])],
#	[enable_libcrypto="${enableval}"], [enable_libcrypto="no"])
#AC_MSG_RESULT([${enable_libcrypto}])
AM_CONDITIONAL([ENABLE_LIBCRYPTO], [test "${enable_libcrypto}" = "yes"])
AC_DEFINE([ENABLE_LIBCRYPTO], [0], [Use libcrypto])
#
#if test "${enable_libcrypto}" = "yes" ; then
#	PKG_CHECK_MODULES([LIBCRYPTO], [libcrypto])
#	AC_SUBST([LIBCRYPTO_CFLAGS])
#	AC_SUBST([LIBCRYPTO_LIBS])
#	AC_DEFINE([ENABLE_LIBCRYPTO], [1])
#fi
# }}}

# linux crypto option {{{
AC_MSG_CHECKING([whether to use the Linux-2.6.38+ hash algorithm interface])
AC_ARG_ENABLE([linux-crypto],
	[AS_HELP_STRING([--enable-linux-crypto], [use the Linux-2.6.38+ hash algorithm interface])],
	[enable_linux_crypto="${enableval}"], [enable_linux_crypto="no"])
AC_MSG_RESULT([${enable_linux_crypto}])
AM_CONDITIONAL([ENABLE_LINUX_CRYPTO], [test "${enable_linux_crypto}" = "yes"])
AC_DEFINE([ENABLE_LINUX_CRYPTO], [0], [Use Linux Crypto])

if test "${enable_linux_crypto}" = "yes" ; then
	AC_CHECK_HEADER([linux/if_alg.h], [:], [AC_MSG_ERROR([linux/if_alg.h not found])])
	AC_DEFINE([ENABLE_LINUX_CRYPTO], [1])
fi
# }}}

# mbedtls option {{{
AC_MSG_CHECKING([whether to use mbed TLS])
AC_ARG_ENABLE([mbedtls],
	[AS_HELP_STRING([--enable-mbedtls], [use mbed TLS])],
	[enable_mbedtls="${enableval}"], [enable_mbedtls="no"])
AC_MSG_RESULT([${enable_mbedtls}])
AM_CONDITIONAL([ENABLE_MBEDTLS], [test "${enable_mbedtls}" = "yes"])
AC_DEFINE([ENABLE_MBEDTLS], [0], [Use mbed TLS])

if test "${enable_mbedtls}" = "yes" ; then
	AC_CHECK_HEADER([mbedtls/md.h], [:], [AC_MSG_ERROR([mbedtls headers not found])])
	AC_CHECK_LIB([mbedcrypto], [mbedtls_md_update], [:], [AC_MSG_ERROR([mbedtls library not found])])
	MBEDTLS_LIBS="-lmbedcrypto"
	AC_SUBST([MBEDTLS_LIBS])
	AC_DEFINE([ENABLE_MBEDTLS], [1])
fi
# }}}

# mhash option {{{
AC_MSG_CHECKING([whether to use the mhash library])
AC_ARG_ENABLE([mhash],
	[AS_HELP_STRING([--enable-mhash], [use the mhash library])],
	[enable_mhash="${enableval}"], [enable_mhash="no"])
AC_MSG_RESULT([${enable_mhash}])
AM_CONDITIONAL([ENABLE_MHASH], [test "${enable_mhash}" = "yes"])
AC_DEFINE([ENABLE_MHASH], [0], [Use mhash])

if test "${enable_mhash}" = "yes" ; then
	AC_CHECK_HEADER([mhash.h], [:], [AC_MSG_ERROR([mhash.h not found])])
	AC_CHECK_LIB([mhash], [mhash], [:], [AC_MSG_ERROR([mhash library not found])])
	MHASH_LIBS="-lmhash"
	AC_SUBST([MHASH_LIBS])
	AC_DEFINE([ENABLE_MHASH], [1])
fi
# }}}

# nettle option {{{
AC_MSG_CHECKING([whether to use Nettle])
AC_ARG_ENABLE([nettle],
	[AS_HELP_STRING([--enable-nettle], [use Nettle])],
	[enable_nettle="${enableval}"], [enable_nettle="no"])
AC_MSG_RESULT([${enable_nettle}])
AM_CONDITIONAL([ENABLE_NETTLE], [test "${enable_nettle}" = "yes"])
AC_DEFINE([ENABLE_NETTLE], [0], [Use Nettle])

if test "${enable_nettle}" = "yes" ; then
	PKG_CHECK_MODULES([NETTLE], [nettle])
	AC_SUBST([NETTLE_CFLAGS])
	AC_SUBST([NETTLE_LIBS])
	AC_DEFINE([ENABLE_NETTLE], [1])
fi
# }}}

# nss option {{{
AC_MSG_CHECKING([whether to use NSS])
AC_ARG_ENABLE([nss],
	[AS_HELP_STRING([--enable-nss], [use NSS])],
	[enable_nss="${enableval}"], [enable_nss="no"])
AC_MSG_RESULT([${enable_nss}])
AM_CONDITIONAL([ENABLE_NSS], [test "${enable_nss}" = "yes"])
AC_DEFINE([ENABLE_NSS], [0], [Use NSS])

if test "${enable_nss}" = "yes" ; then
	PKG_CHECK_MODULES([NSS], [nss])
	AC_SUBST([NSS_CFLAGS])
	AC_SUBST([NSS_LIBS])
	AC_DEFINE([ENABLE_NSS], [1])
fi
# }}}

# polarssl option {{{
AC_MSG_CHECKING([whether to use PolarSSL])
AC_ARG_ENABLE([polarssl],
	[AS_HELP_STRING([--enable-polarssl], [use PolarSSL])],
	[enable_polarssl="${enableval}"], [enable_polarssl="no"])
AC_MSG_RESULT([${enable_polarssl}])
AM_CONDITIONAL([ENABLE_POLARSSL], [test "${enable_polarssl}" = "yes"])
AC_DEFINE([ENABLE_POLARSSL], [0], [Use PolarSSL])

if test "${enable_polarssl}" = "yes" ; then
	AC_CHECK_HEADER([polarssl/md.h], [:], [AC_MSG_ERROR([polarssl headers not found])])
	AC_CHECK_LIB([polarssl], [md_update], [:], [AC_MSG_ERROR([polarssl library not found])])
	POLARSSL_LIBS="-lpolarssl"
	AC_SUBST([POLARSSL_LIBS])
	AC_DEFINE([ENABLE_POLARSSL], [1])
fi
# }}}

# zlib option {{{
AC_MSG_CHECKING([whether to use zlib])
AC_ARG_ENABLE([zlib],
	[AS_HELP_STRING([--disable-zlib], [do not use zlib])],
	[enable_zlib="${enableval}"], [enable_zlib="yes"])
AC_MSG_RESULT([${enable_zlib}])
AM_CONDITIONAL([ENABLE_ZLIB], [test "${enable_zlib}" = "yes"])
AC_DEFINE([ENABLE_ZLIB], [0], [Use zlib])

if test "${enable_zlib}" = "yes" ; then
	PKG_CHECK_MODULES([ZLIB], [zlib])
	AC_SUBST([ZLIB_CFLAGS])
	AC_SUBST([ZLIB_LIBS])
	AC_DEFINE([ENABLE_ZLIB], [1])
fi
# }}}

AC_MSG_CHECKING([whether to build gtkhash binary])
AC_ARG_ENABLE([gtkhash],
	[AS_HELP_STRING([--disable-gtkhash], [do not build gtkhash binary])],
	[enable_gtkhash="${enableval}"], [enable_gtkhash="yes"])
AC_MSG_RESULT([${enable_gtkhash}])
AM_CONDITIONAL([ENABLE_GTKHASH], [test "${enable_gtkhash}" = "yes"])

# appstream option {{{
AC_MSG_CHECKING([whether to install AppStream metadata files])
AC_ARG_ENABLE([appstream],
	[AS_HELP_STRING([--disable-appstream], [do not install AppStream metadata files])],
	[enable_appstream="${enableval}"], [enable_appstream="yes"])
AC_MSG_RESULT([${enable_appstream}])
AM_CONDITIONAL([ENABLE_APPSTREAM], [test "${enable_appstream}" = "yes"])
# }}}

# caja option {{{
AC_MSG_CHECKING([whether to build the Caja extension])
AC_ARG_ENABLE([caja],
	[AS_HELP_STRING([--enable-caja], [build the Caja extension])],
	[enable_caja="${enableval}"], [enable_caja="no"])
AC_MSG_RESULT([${enable_caja}])
AM_CONDITIONAL([ENABLE_CAJA], [test "${enable_caja}" = "yes"])

if test "${enable_caja}" = "yes" ; then
	# Check for caja
	PKG_CHECK_MODULES([CAJA], [libcaja-extension])
	AC_SUBST([CAJA_CFLAGS])
	AC_SUBST([CAJA_LIBS])
	CAJA_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensiondir libcaja-extension`
	AC_SUBST([CAJA_EXTENSION_DIR])
fi
# }}}

# nautilus option {{{
AC_MSG_CHECKING([whether to build the Nautilus extension])
AC_ARG_ENABLE([nautilus],
	[AS_HELP_STRING([--enable-nautilus], [build the Nautilus extension])],
	[enable_nautilus="${enableval}"], [enable_nautilus="no"])
AC_MSG_RESULT([${enable_nautilus}])
AM_CONDITIONAL([ENABLE_NAUTILUS], [test "${enable_nautilus}" = "yes"])

if test "${enable_nautilus}" = "yes" ; then
	# Check for nautilus
	PKG_CHECK_MODULES([NAUTILUS], [libnautilus-extension])
	AC_SUBST([NAUTILUS_CFLAGS])
	AC_SUBST([NAUTILUS_LIBS])
	NAUTILUS_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensiondir libnautilus-extension`
	AC_SUBST([NAUTILUS_EXTENSION_DIR])
fi
# }}}

# nemo option {{{
AC_MSG_CHECKING([whether to build the Nemo extension])
AC_ARG_ENABLE([nemo],
	[AS_HELP_STRING([--enable-nemo], [build the Nemo extension])],
	[enable_nemo="${enableval}"], [enable_nemo="no"])
AC_MSG_RESULT([${enable_nemo}])
AM_CONDITIONAL([ENABLE_NEMO], [test "${enable_nemo}" = "yes"])

if test "${enable_nemo}" = "yes" ; then
	# Check for nemo
	PKG_CHECK_MODULES([NEMO], [libnemo-extension])
	AC_SUBST([NEMO_CFLAGS])
	AC_SUBST([NEMO_LIBS])
	NEMO_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensiondir libnemo-extension`
	AC_SUBST([NEMO_EXTENSION_DIR])
fi
# }}}

# thunar option {{{
AC_MSG_CHECKING([whether to build the Thunar extension])
AC_ARG_ENABLE([thunar],
	[AS_HELP_STRING([--enable-thunar], [build the Thunar extension])],
	[enable_thunar="${enableval}"], [enable_thunar="no"])
AC_MSG_RESULT([${enable_thunar}])
AM_CONDITIONAL([ENABLE_THUNAR], [test "${enable_thunar}" = "yes"])

if test "${enable_thunar}" = "yes" ; then
	# Check for thunar
	PKG_CHECK_MODULES([THUNAR], [thunarx-2])
	AC_SUBST([THUNAR_CFLAGS])
	AC_SUBST([THUNAR_LIBS])
	THUNAR_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensionsdir thunarx-2`
	AC_SUBST([THUNAR_EXTENSION_DIR])
fi
# }}}

AM_CONDITIONAL([ENABLE_NAUTILUS_DATA], [
	test "${enable_caja}" = "yes" ||
	test "${enable_nautilus}" = "yes" ||
	test "${enable_nemo}" = "yes" ||
	test "${enable_thunar}" = "yes"])

# nls option {{{
AM_CONDITIONAL([ENABLE_NLS], [test "${enable_nls}" != "no"])

if test "${enable_nls}" = "no" ; then
	AC_DEFINE([ENABLE_NLS], [0])
else
	GETTEXT_PACKAGE="${PACKAGE}"
	AC_SUBST([GETTEXT_PACKAGE])
	AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [Define the gettext package])
	IT_PROG_INTLTOOL([0.40.6])
	AM_GLIB_GNU_GETTEXT
fi
# }}}

AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], ["${PACKAGE_NAME}"], [Define the Glib log domain])
AC_DEFINE([HASH_FILE_BUFFER_SIZE], [131072], [Define file read buffer size (bytes)])
AC_DEFINE([HASH_FILE_REPORT_INTERVAL], [166], [Define progress report interval (milliseconds)])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([data/appdata/Makefile])
AC_CONFIG_FILES([data/icons/Makefile])
AC_CONFIG_FILES([data/nautilus/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/hash/Makefile])
AC_CONFIG_FILES([src/nautilus/Makefile])
AC_CONFIG_FILES([src/util/Makefile])
AC_OUTPUT

echo
echo "Interfaces:"
echo "  gtkhash: ${enable_gtkhash} (gtk+-${with_gtk})"
echo "  gtkhash-caja: ${enable_caja}"
echo "  gtkhash-nautilus: ${enable_nautilus}"
echo "  gtkhash-nemo: ${enable_nemo}"
echo "  gtkhash-thunar: ${enable_thunar}"

blake2_funcs="
	BLAKE2b BLAKE2s BLAKE2bp BLAKE2sp"
gcrypt_funcs="
	GOST
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512
	TIGER192
	WHIRLPOOL"
glib_checksums_funcs="
	MD5
	SHA1
	SHA256 SHA512"
internal_md6_funcs="
	MD6-224 MD6-256 MD6-384 MD6-512"
libcrypto_funcs="
	MD4
	MD5
	MDC2
	RIPEMD160
	SHA0
	SHA1"
linux_crypto_funcs="
	MD4
	MD5
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512
	TIGER192
	WHIRLPOOL"
mbedtls_funcs="
	MD2
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512"
mhash_funcs="
	GOST
	HAVAL128-3 HAVAL160-3 HAVAL192-3 HAVAL224-3 HAVAL256-3
	MD2
	MD4
	MD5
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SNEFRU128 SNEFRU256
	TIGER192
	WHIRLPOOL"
nettle_funcs="
	GOST
	MD2
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512"
nss_funcs="
	MD2
	MD5
	SHA1
	SHA224 SHA256 SHA384 SHA512"
polarssl_funcs="
	MD2
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512"
zlib_funcs="
	ADLER32
	CRC32"

echo
echo "Supported Hash Functions:"

have_libs=""

for func in \
	ADLER32 \
	BLAKE2b BLAKE2s BLAKE2bp BLAKE2sp \
	CRC32 \
	GOST \
	HAVAL128-3 HAVAL160-3 HAVAL192-3 HAVAL224-3 HAVAL256-3 \
	MD2 \
	MD4 \
	MD5 \
	MD6-224 MD6-256 MD6-384 MD6-512 \
	MDC2 \
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320 \
	SHA0 \
	SHA1 \
	SHA224 SHA256 SHA384 SHA512 \
	SHA3-224 SHA3-256 SHA3-384 SHA3-512 \
	SNEFRU128 SNEFRU256 \
	TIGER192 \
	WHIRLPOOL
do
	printf "  %-14s" "${func}"
	enabled=""
	# Matches preferred order from src/hash/hash-lib.c
	for lib in \
		zlib \
		linux_crypto \
		gcrypt \
		libcrypto \
		mbedtls \
		polarssl \
		nettle \
		nss \
		glib_checksums \
		mhash \
		blake2 \
		internal_md6
	do
		eval test \${enable_${lib}} = "yes" || continue
		if eval echo \${${lib}_funcs} | tr ' ' '\n' \
			| grep -q "^${func}$"
		then
			test -n "${enabled}" && printf ","
			printf "${lib}"
			enabled="yes"
			have_libs="yes"
		fi
	done
	test -n "${enabled}" && echo || echo "<disabled>"
done

echo
test -z "${have_libs}" && AC_MSG_ERROR([No hash libraries enabled])

# Exit status
true
