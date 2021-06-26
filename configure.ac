AC_PREREQ([2.61])
AC_INIT([GtkHash], [1.4])
AC_SUBST([RELEASE_DATE], [2020-07-11])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 check-news dist-xz foreign subdir-objects -Wall])
AM_SILENT_RULES([yes])

AC_PROG_CC_C99
AM_PROG_CC_C_O
AM_PROG_AR
LT_INIT

GETTEXT_PACKAGE="${PACKAGE}"
AC_SUBST([GETTEXT_PACKAGE])
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["${GETTEXT_PACKAGE}"], [Define the gettext package])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.7])
AM_CONDITIONAL([ENABLE_NLS], [test "${enable_nls}" != "no"])

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
	-Wformat \
	-Wformat-nonliteral \
	-Wformat-overflow \
	-Wformat-security \
	-Wformat-signedness \
	-Wformat-truncation \
	-Wlogical-op \
	-Wmissing-declarations \
	-Wmissing-noreturn \
	-Wredundant-decls \
	-Wshadow \
	-Wswitch-default \
	-Wundef \
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
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.48])
AC_SUBST([GLIB_CFLAGS])
AC_SUBST([GLIB_LIBS])

PKG_CHECK_MODULES([GIO], [gio-2.0 >= 2.48])
AC_SUBST([GIO_CFLAGS])
AC_SUBST([GIO_LIBS])

m4_ifdef([PKG_CHECK_VAR],
	[PKG_CHECK_VAR([GLIB_COMPILE_RESOURCES], [gio-2.0], [glib_compile_resources])],
	[AC_CHECK_PROGS([GLIB_COMPILE_RESOURCES], [glib-compile-resources])])
AC_SUBST([GLIB_COMPILE_RESOURCES])

GLIB_GSETTINGS
# }}}

# check for gtk {{{
AC_MSG_CHECKING([which GTK version to use])
AC_ARG_WITH([gtk],
	[AS_HELP_STRING([--with-gtk=3.0], [GTK version to use [default=3.0]])],
	[with_gtk="${withval}"], [with_gtk="3.0"])
AC_MSG_RESULT([${with_gtk}])

PKG_CHECK_MODULES([GTK], ["gtk+-${with_gtk}" >= 3.18])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])
# }}}

# native file chooser option {{{
AC_MSG_CHECKING([whether to enable native file chooser dialogs])
AC_ARG_ENABLE([native-file-chooser],
	[AS_HELP_STRING([--disable-native-file-chooser], [disable native file chooser dialogs])],
	[enable_native_file_chooser="${enableval}"], [enable_native_file_chooser="yes"])
AC_MSG_RESULT([${enable_native_file_chooser}])

if test "${enable_native_file_chooser}" = "yes" ; then
	AC_DEFINE([ENABLE_NATIVE_FILE_CHOOSER], [1], [Native file chooser])
else
	AC_DEFINE([ENABLE_NATIVE_FILE_CHOOSER], [0], [Native file chooser])
fi
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
	AC_DEFINE([GTK_DISABLE_DEPRECATED], [1], [disable gtk deprecated])
else
	AC_DEFINE([G_DISABLE_ASSERT], [1], [disable glib assert])
	AC_DEFINE([G_DISABLE_CAST_CHECKS], [1], [disable glib cast checks])
fi
# }}}

# blake2 option {{{
AC_MSG_CHECKING([whether to use libb2])
AC_ARG_ENABLE([blake2],
	[AS_HELP_STRING([--disable-blake2], [do not use libb2])],
	[enable_blake2="${enableval}"], [enable_blake2="yes"])
AC_MSG_RESULT([${enable_blake2}])
AM_CONDITIONAL([ENABLE_BLAKE2], [test "${enable_blake2}" = "yes"])

if test "${enable_blake2}" = "yes" ; then
	AC_CHECK_HEADER([blake2.h], [:], [AC_MSG_ERROR([blake2 header not found])])
	AC_CHECK_LIB([b2], [blake2sp_init], [:], [AC_MSG_ERROR([blake2 library not found])])
	BLAKE2_LIBS="-lb2"
	AC_SUBST([BLAKE2_LIBS])
	AC_DEFINE([ENABLE_BLAKE2], [1], [libb2])
else
	AC_DEFINE([ENABLE_BLAKE2], [0], [libb2])
fi
# }}}

# gcrypt option {{{
AC_MSG_CHECKING([whether to use Libgcrypt])
AC_ARG_ENABLE([gcrypt],
	[AS_HELP_STRING([--disable-gcrypt], [do not use Libgcrypt])],
	[enable_gcrypt="${enableval}"], [enable_gcrypt="yes"])
AC_MSG_RESULT([${enable_gcrypt}])
AM_CONDITIONAL([ENABLE_GCRYPT], [test "${enable_gcrypt}" = "yes"])

gcrypt_funcs=

HASH_LIB_GCRYPT_MIN_VER="1.6.0"
AC_DEFINE_UNQUOTED([HASH_LIB_GCRYPT_MIN_VER], ["${HASH_LIB_GCRYPT_MIN_VER}"], [required gcrypt version])

if test "${enable_gcrypt}" = "yes" ; then
	AM_PATH_LIBGCRYPT(["${HASH_LIB_GCRYPT_MIN_VER}"], [:], [AC_MSG_ERROR([Libgcrypt not found])])
	AC_DEFINE([ENABLE_GCRYPT], [1], [Libgcrypt])

	ac_save_CFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} ${LIBGCRYPT_CFLAGS} ${LIBGCRYPT_LIBS}"

	AC_CHECK_DECL([GCRY_MD_BLAKE2B_512],
		[gcrypt_funcs="${gcrypt_funcs} BLAKE2b BLAKE2s"], [:],
		[#include <gcrypt.h>])
	AC_CHECK_DECL([GCRY_MD_SHA3_512],
		[gcrypt_funcs="${gcrypt_funcs} SHA3-224 SHA3-256 SHA3-384 SHA3-512"], [:],
		[#include <gcrypt.h>])

	CFLAGS="${ac_save_CFLAGS}"
else
	AC_DEFINE([ENABLE_GCRYPT], [0], [Libgcrypt])
fi
# }}}

# glib checksums option {{{
AC_MSG_CHECKING([whether to use the Glib Data Checksums API])
AC_ARG_ENABLE([glib-checksums],
	[AS_HELP_STRING([--enable-glib-checksums], [use the Glib Data Checksums API])],
	[enable_glib_checksums="${enableval}"], [enable_glib_checksums="no"])
AC_MSG_RESULT([${enable_glib_checksums}])
AM_CONDITIONAL([ENABLE_GLIB_CHECKSUMS], [test "${enable_glib_checksums}" = "yes"])

if test "${enable_glib_checksums}" = "yes" ; then
	AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [1], [Glib Checksums])
else
	AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [0], [Glib Checksums])
fi
# }}}


# internal md6 option {{{
AC_MSG_CHECKING([whether to use the internal MD6 library])
AC_ARG_ENABLE([internal-md6],
	[AS_HELP_STRING([--disable-internal-md6], [do not use the internal MD6 library])],
	[enable_internal_md6="${enableval}"], [enable_internal_md6="yes"])
AC_MSG_RESULT([${enable_internal_md6}])
AM_CONDITIONAL([ENABLE_INTERNAL_MD6], [test "${enable_internal_md6}" = "yes"])

if test "${enable_internal_md6}" = "yes" ; then
	AC_DEFINE([ENABLE_INTERNAL_MD6], [1], [MD6])
else
	AC_DEFINE([ENABLE_INTERNAL_MD6], [0], [MD6])
fi
# }}}

# libcrypto (openssl) option {{{
AC_MSG_CHECKING([whether to use OpenSSL libcrypto])
AC_ARG_ENABLE([libcrypto],
	[AS_HELP_STRING([--enable-libcrypto], [use libcrypto])],
	[enable_libcrypto="${enableval}"], [enable_libcrypto="no"])
AC_MSG_RESULT([${enable_libcrypto}])
AM_CONDITIONAL([ENABLE_LIBCRYPTO], [test "${enable_libcrypto}" = "yes"])

if test "${enable_libcrypto}" = "yes" ; then
	PKG_CHECK_MODULES([LIBCRYPTO], [libcrypto >= 1.1])
	AC_SUBST([LIBCRYPTO_CFLAGS])
	AC_SUBST([LIBCRYPTO_LIBS])
	AC_DEFINE([ENABLE_LIBCRYPTO], [1], [libcrypto])
else
	AC_DEFINE([ENABLE_LIBCRYPTO], [0], [libcrypto])
fi
# }}}

# linux crypto option {{{
AC_MSG_CHECKING([whether to use Linux kernel AF_ALG])
AC_ARG_ENABLE([linux-crypto],
	[AS_HELP_STRING([--enable-linux-crypto], [use Linux kernel AF_ALG])],
	[enable_linux_crypto="${enableval}"], [enable_linux_crypto="no"])
AC_MSG_RESULT([${enable_linux_crypto}])
AM_CONDITIONAL([ENABLE_LINUX_CRYPTO], [test "${enable_linux_crypto}" = "yes"])

if test "${enable_linux_crypto}" = "yes" ; then
	AC_CHECK_HEADER([linux/if_alg.h], [:], [AC_MSG_ERROR([linux/if_alg.h not found])])
	AC_DEFINE([ENABLE_LINUX_CRYPTO], [1], [Linux kernel AF_ALG])
else
	AC_DEFINE([ENABLE_LINUX_CRYPTO], [0], [Linux kernel AF_ALG])
fi
# }}}

# mbedtls option {{{
AC_MSG_CHECKING([whether to use Mbed TLS])
AC_ARG_ENABLE([mbedtls],
	[AS_HELP_STRING([--enable-mbedtls], [use Mbed TLS])],
	[enable_mbedtls="${enableval}"], [enable_mbedtls="no"])
AC_MSG_RESULT([${enable_mbedtls}])
AM_CONDITIONAL([ENABLE_MBEDTLS], [test "${enable_mbedtls}" = "yes"])

if test "${enable_mbedtls}" = "yes" ; then
	AC_CHECK_HEADER([mbedtls/md.h], [:], [AC_MSG_ERROR([mbedtls headers not found])])
	AC_CHECK_LIB([mbedcrypto], [mbedtls_md_update], [:], [AC_MSG_ERROR([mbedtls library not found])])
	MBEDTLS_LIBS="-lmbedcrypto"
	AC_SUBST([MBEDTLS_LIBS])
	AC_DEFINE([ENABLE_MBEDTLS], [1], [Mbed TLS])
else
	AC_DEFINE([ENABLE_MBEDTLS], [0], [Mbed TLS])
fi
# }}}

# nettle option {{{
AC_MSG_CHECKING([whether to use Nettle])
AC_ARG_ENABLE([nettle],
	[AS_HELP_STRING([--enable-nettle], [use Nettle])],
	[enable_nettle="${enableval}"], [enable_nettle="no"])
AC_MSG_RESULT([${enable_nettle}])
AM_CONDITIONAL([ENABLE_NETTLE], [test "${enable_nettle}" = "yes"])

if test "${enable_nettle}" = "yes" ; then
	PKG_CHECK_MODULES([NETTLE], [nettle])
	AC_SUBST([NETTLE_CFLAGS])
	AC_SUBST([NETTLE_LIBS])
	AC_DEFINE([ENABLE_NETTLE], [1], [Nettle])
else
	AC_DEFINE([ENABLE_NETTLE], [0], [Nettle])
fi
# }}}

# zlib option {{{
AC_MSG_CHECKING([whether to use zlib])
AC_ARG_ENABLE([zlib],
	[AS_HELP_STRING([--disable-zlib], [do not use zlib])],
	[enable_zlib="${enableval}"], [enable_zlib="yes"])
AC_MSG_RESULT([${enable_zlib}])
AM_CONDITIONAL([ENABLE_ZLIB], [test "${enable_zlib}" = "yes"])

if test "${enable_zlib}" = "yes" ; then
	PKG_CHECK_MODULES([ZLIB], [zlib])
	AC_SUBST([ZLIB_CFLAGS])
	AC_SUBST([ZLIB_LIBS])
	AC_DEFINE([ENABLE_ZLIB], [1], [zlib])
else
	AC_DEFINE([ENABLE_ZLIB], [0], [zlib])
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

	# nautilus-3.29.90+ moved to a different header file
	ac_save_CPPFLAGS="${CPPFLAGS}"
	CPPFLAGS="${CPPFLAGS} ${NAUTILUS_CFLAGS}"
	AC_CHECK_HEADERS([nautilus-extension.h])
	CPPFLAGS="${ac_save_CPPFLAGS}"
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

AC_MSG_CHECKING([which Thunarx version to use])
AC_ARG_WITH([thunarx],
	[AS_HELP_STRING([--with-thunarx=3], [Thunarx version to use [default=3]])],
	[with_thunar="${withval}"], [with_thunarx="3"])
AC_MSG_RESULT([${with_thunarx}])

if test "${enable_thunar}" = "yes" ; then
	# Check for thunar
	PKG_CHECK_MODULES([THUNAR], ["thunarx-${with_thunarx}" >= 1.7.0])
	AC_SUBST([THUNAR_CFLAGS])
	AC_SUBST([THUNAR_LIBS])
	THUNAR_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensionsdir "thunarx-${with_thunarx}"`
	AC_SUBST([THUNAR_EXTENSION_DIR])
fi
# }}}

AM_CONDITIONAL([ENABLE_NAUTILUS_DATA], [
	test "${enable_caja}" = "yes" ||
	test "${enable_nautilus}" = "yes" ||
	test "${enable_nemo}" = "yes" ||
	test "${enable_thunar}" = "yes"])

# Matches preferred order from src/hash/hash-lib.c
HASH_LIBS="blake2 gcrypt libcrypto mbedtls nettle linux_crypto zlib glib_checksums internal_md6"
AC_SUBST([HASH_LIBS])

AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], ["${PACKAGE_NAME}"], [Define the Glib log domain])
AC_DEFINE([HASH_FILE_BUFFER_SIZE], [131072], [Define file read buffer size (bytes)])
AC_DEFINE([HASH_FILE_REPORT_INTERVAL], [166], [Define progress report interval (milliseconds)])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([data/appdata/Makefile])
AC_CONFIG_FILES([data/appdata/org.gtkhash.gtkhash.appdata.xml.in])
AC_CONFIG_FILES([data/appdata/org.gtkhash.caja.metainfo.xml.in])
AC_CONFIG_FILES([data/appdata/org.gtkhash.nautilus.metainfo.xml.in])
AC_CONFIG_FILES([data/appdata/org.gtkhash.nemo.metainfo.xml.in])
AC_CONFIG_FILES([data/appdata/org.gtkhash.thunar.metainfo.xml.in])
AC_CONFIG_FILES([data/caja/Makefile])
AC_CONFIG_FILES([data/caja/libgtkhash-properties-caja.caja-extension.in])
AC_CONFIG_FILES([data/icons/Makefile])
AC_CONFIG_FILES([data/nautilus/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/hash/Makefile])
AC_CONFIG_FILES([src/test.md5sum])
AC_CONFIG_FILES([src/nautilus/Makefile])
AC_CONFIG_FILES([src/util/Makefile])
AC_OUTPUT

echo
echo "Interfaces:"
echo "  gtkhash:            ${enable_gtkhash} (gtk+-${with_gtk})"
echo "  gtkhash-caja:       ${enable_caja}"
echo "  gtkhash-nautilus:   ${enable_nautilus}"
echo "  gtkhash-nemo:       ${enable_nemo}"
echo "  gtkhash-thunar:     ${enable_thunar} (thunarx-${with_thunarx})"

blake2_funcs="
	BLAKE2b BLAKE2s BLAKE2bp BLAKE2sp"
gcrypt_funcs="${gcrypt_funcs}
	CRC32
	GOST
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512
	TIGER192
	WHIRLPOOL"
glib_checksums_funcs="
	MD5
	SHA1
	SHA256 SHA384 SHA512"
internal_md6_funcs="
	MD6-224 MD6-256 MD6-384 MD6-512"
libcrypto_funcs="
	BLAKE2b BLAKE2s
	MD2
	MD4
	MD5
	MDC2
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512
	SM3
	WHIRLPOOL"
linux_crypto_funcs="
	BLAKE2b BLAKE2s
	CRC32C
	MD4
	MD5
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512
	TIGER192
	WHIRLPOOL
	XXH64"
mbedtls_funcs="
	MD2
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512"
nettle_funcs="
	GOST
	MD2
	MD4
	MD5
	RIPEMD160
	SHA1
	SHA224 SHA256 SHA384 SHA512
	SHA3-224 SHA3-256 SHA3-384 SHA3-512"
zlib_funcs="
	ADLER32
	CRC32"

echo
printf "%-20s%s\n" "Hash Function" "Preferred Implementation"

have_libs=""

for func in \
	ADLER32 \
	BLAKE2b BLAKE2s BLAKE2bp BLAKE2sp \
	CRC32 CRC32C \
	GOST \
	MD2 \
	MD4 \
	MD5 \
	MD6-224 MD6-256 MD6-384 MD6-512 \
	MDC2 \
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320 \
	SHA1 \
	SHA224 SHA256 SHA384 SHA512 \
	SHA3-224 SHA3-256 SHA3-384 SHA3-512 \
	SM3 \
	TIGER192 \
	WHIRLPOOL \
	XXH64
do
	printf "  %-20s" "${func}"
	enabled=""
	for lib in ${HASH_LIBS} ; do
		eval test \${enable_${lib}} = "yes" || continue

		if eval echo \${${lib}_funcs} | tr ' ' '\n' \
			| grep -q "^${func}$"
		then
			test -n "${enabled}" && printf ","
			printf "${lib}"
			enabled="yes"
			have_libs="yes"

			test "${lib}" = "linux_crypto" && printf "*"
		fi
	done
	test -n "${enabled}" && echo || echo "_"
done

if test "${enable_linux_crypto}" = "yes" ; then
	echo
	echo "* linux_crypto support depends on kernel module availability - check /proc/crypto"
fi

echo

test -z "${have_libs}" && AC_MSG_ERROR([No hash libraries enabled])

# Exit status
true
