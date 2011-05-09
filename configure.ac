AC_PREREQ([2.61])
AC_INIT([GtkHash], [0.4.0])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 check-news dist-xz])
AM_SILENT_RULES([yes])

AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT

AX_CFLAGS_GCC_OPTION([-pedantic])
AX_CFLAGS_GCC_OPTION([-Wall])
AX_CFLAGS_GCC_OPTION([-Wextra])

AX_CFLAGS_GCC_OPTION([-Waggregate-return])
AX_CFLAGS_GCC_OPTION([-Wbad-function-cast])
AX_CFLAGS_GCC_OPTION([-Wcast-align])
AX_CFLAGS_GCC_OPTION([-Wfloat-equal])
AX_CFLAGS_GCC_OPTION([-Wlogical-op])
AX_CFLAGS_GCC_OPTION([-Wmissing-declarations])
AX_CFLAGS_GCC_OPTION([-Wmissing-noreturn])
AX_CFLAGS_GCC_OPTION([-Wredundant-decls])
AX_CFLAGS_GCC_OPTION([-Wshadow])
AX_CFLAGS_GCC_OPTION([-Wswitch-default])
AX_CFLAGS_GCC_OPTION([-Wwrite-strings])

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.28])

PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
AC_SUBST([GTHREAD_CFLAGS])
AC_SUBST([GTHREAD_LIBS])

AC_MSG_CHECKING([which GTK+ version to use])
AC_ARG_WITH([gtk],
	[AS_HELP_STRING([--with-gtk=2.0|3.0], [GTK+ version to use [2.0]])],
	[with_gtk="${withval}"], [with_gtk="2.0"])
AC_MSG_RESULT(["${with_gtk}"])

PKG_CHECK_MODULES([GDK], ["gdk-${with_gtk}"])
AC_SUBST([GDK_CFLAGS])
AC_SUBST([GDK_LIBS])

PKG_CHECK_MODULES([GTK], ["gtk+-${with_gtk}"])
AC_SUBST([GTK_CFLAGS])
AC_SUBST([GTK_LIBS])

hash_libs=""

AC_MSG_CHECKING([whether to use the Linux-2.6.38+ hash algorithm interface])
AC_ARG_ENABLE([linux-crypto],
	[AS_HELP_STRING([--enable-linux-crypto], [use the Linux-2.6.38+ hash algorithm interface])],
	[enable_linux_crypto="${enableval}"], [enable_linux_crypto="no"])
AC_MSG_RESULT(["${enable_linux_crypto}"])
AM_CONDITIONAL([ENABLE_LINUX_CRYPTO], [test "${enable_linux_crypto}" = "yes"])
AC_DEFINE([ENABLE_LINUX_CRYPTO], [0], [Use Linux Crypto])

if test "${enable_linux_crypto}" = "yes" ; then
	AC_CHECK_HEADER([linux/if_alg.h], [:], [AC_MSG_ERROR([linux/if_alg.h not found])])
	hash_libs="${hash_libs} linux_crypto"
	AC_DEFINE([ENABLE_LINUX_CRYPTO], [1])
fi

AC_MSG_CHECKING([whether to use Libgcrypt])
AC_ARG_ENABLE([gcrypt],
	[AS_HELP_STRING([--enable-gcrypt], [use Libgcrypt])],
	[enable_gcrypt="${enableval}"], [enable_gcrypt="no"])
AC_MSG_RESULT(["${enable_gcrypt}"])
AM_CONDITIONAL([ENABLE_GCRYPT], [test "${enable_gcrypt}" = "yes"])
AC_DEFINE([ENABLE_GCRYPT], [0], [Use Libgcrypt])

if test "${enable_gcrypt}" = "yes" ; then
	AM_PATH_LIBGCRYPT([1.4.0], [:], [AC_MSG_ERROR([Libgcrypt not found])])
	hash_libs="${hash_libs} gcrypt"
	AC_DEFINE([ENABLE_GCRYPT], [1])
fi

AC_MSG_CHECKING([whether to use NSS])
AC_ARG_ENABLE([nss],
	[AS_HELP_STRING([--enable-nss], [use NSS])],
	[enable_nss="${enableval}"], [enable_nss="no"])
AC_MSG_RESULT(["${enable_nss}"])
AM_CONDITIONAL([ENABLE_NSS], [test "${enable_nss}" = "yes"])
AC_DEFINE([ENABLE_NSS], [0], [Use NSS])

if test "${enable_nss}" = "yes" ; then
	PKG_CHECK_MODULES([NSS], [nss])
	AC_SUBST([NSS_CFLAGS])
	AC_SUBST([NSS_LIBS])
	hash_libs="${hash_libs} nss"
	AC_DEFINE([ENABLE_NSS], [1])
fi

AC_MSG_CHECKING([whether to use the Glib Data Checksums API])
AC_ARG_ENABLE([glib-checksums],
	[AS_HELP_STRING([--disable-glib-checksums], [do not use the Glib Data Checksums API])],
	[enable_glib_checksums="${enableval}"], [enable_glib_checksums="yes"])
AC_MSG_RESULT(["${enable_glib_checksums}"])
AM_CONDITIONAL([ENABLE_GLIB_CHECKSUMS], [test "${enable_glib_checksums}" = "yes"])
AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [0], [Use Glib Checksums])

if test "${enable_glib_checksums}" = "yes" ; then
	hash_libs="${hash_libs} glib_checksums"
	AC_DEFINE([ENABLE_GLIB_CHECKSUMS], [1])
fi

AC_MSG_CHECKING([whether to use the mhash library])
AC_ARG_ENABLE([mhash],
	[AS_HELP_STRING([--enable-mhash], [use the mhash library])],
	[enable_mhash="${enableval}"], [enable_mhash="no"])
AC_MSG_RESULT(["${enable_mhash}"])
AM_CONDITIONAL([ENABLE_MHASH], [test "${enable_mhash}" = "yes"])
AC_DEFINE([ENABLE_MHASH], [0], [Use mhash])

if test "${enable_mhash}" = "yes" ; then
	AC_CHECK_HEADER([mhash.h], [:], [AC_MSG_ERROR([mhash.h not found])])
	AC_CHECK_LIB([mhash], [mhash], [:], [AC_MSG_ERROR([mhash library not found])])
	MHASH_LIBS="-lmhash"
	AC_SUBST([MHASH_LIBS])
	hash_libs="${hash_libs} mhash"
	AC_DEFINE([ENABLE_MHASH], [1])
fi

test -z "${hash_libs}" && AC_MSG_ERROR([No hash libraries enabled])

AC_MSG_CHECKING([whether to build the Nautilus extension])
AC_ARG_ENABLE([nautilus],
	[AS_HELP_STRING([--enable-nautilus], [build the Nautilus extension])],
	[enable_nautilus="${enableval}"], [enable_nautilus="no"])
AC_MSG_RESULT(["${enable_nautilus}"])
AM_CONDITIONAL([ENABLE_NAUTILUS], [test "${enable_nautilus}" = "yes"])

AM_GCONF_SOURCE_2

if test "${enable_nautilus}" = "yes" ; then
	# Check for nautilus
	PKG_CHECK_MODULES([NAUTILUS], [libnautilus-extension])
	AC_SUBST([NAUTILUS_CFLAGS])
	AC_SUBST([NAUTILUS_LIBS])
	NAUTILUS_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensiondir libnautilus-extension`
	AC_SUBST([NAUTILUS_EXTENSION_DIR])
	if test "`basename \"${NAUTILUS_EXTENSION_DIR}\"`" != "extensions-${with_gtk}" ; then
		AC_MSG_ERROR([Nautilus version must match GTK+ version or bad things will happen])
	fi

	# Check for gconf
	PKG_CHECK_MODULES([GCONF], [gconf-2.0])
	AC_SUBST([GCONF_CFLAGS])
	AC_SUBST([GCONF_LIBS])
	AC_PATH_PROG([GCONFTOOL], [gconftool-2], [no])
	if test "${GCONFTOOL}" = "no" ; then
		AC_MSG_ERROR([gconftool-2 not found])
	fi
fi

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

AC_DEFINE_UNQUOTED([G_LOG_DOMAIN], ["${PACKAGE_NAME}"], [Define the Glib log domain])

AC_DEFINE([HASH_FILE_BUFFER_SIZE], [131072], [Define file read buffer size (bytes)])
AC_DEFINE([HASH_FILE_REPORT_INTERVAL], [166], [Define progress report interval (milliseconds)])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([data/Makefile])
AC_CONFIG_FILES([data/nautilus/Makefile])
AC_CONFIG_FILES([po/Makefile.in])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([src/hash/Makefile])
AC_CONFIG_FILES([src/nautilus/Makefile])
AC_OUTPUT

gcrypt_funcs="
	MD4 MD5
	SHA1 SHA224 SHA256 SHA384 SHA512
	RIPEMD160
	TIGER192
	WHIRLPOOL"
glib_checksums_funcs="
	MD5
	SHA1 SHA256"
linux_crypto_funcs="
	MD4 MD5
	SHA1 SHA224 SHA256 SHA384 SHA512
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320
	TIGER128 TIGER160 TIGER192
	WHIRLPOOL"
mhash_funcs="
	MD2 MD4 MD5
	SHA1 SHA224 SHA256 SHA384 SHA512
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320
	TIGER128 TIGER160 TIGER192
	WHIRLPOOL
	HAVAL128-3 HAVAL160-3 HAVAL192-3 HAVAL224-3 HAVAL256-3
	GOST
	SNEFRU128 SNEFRU256
	CRC32 CRC32B
	ADLER32"
nss_funcs="
	MD2 MD5
	SHA1 SHA256 SHA384 SHA512"
echo
echo "Supported Hash Functions:"
for func in \
	MD2 MD4 MD5 \
	SHA1 SHA224 SHA256 SHA384 SHA512 \
	RIPEMD128 RIPEMD160 RIPEMD256 RIPEMD320 \
	TIGER128 TIGER160 TIGER192 \
	WHIRLPOOL \
	HAVAL128-3 HAVAL160-3 HAVAL192-3 HAVAL224-3 HAVAL256-3 \
	GOST \
	SNEFRU128 SNEFRU256 \
	CRC32 CRC32B \
	ADLER32
do
	printf "  %-14s" "${func}"
	enabled=""
	for lib in ${hash_libs} ; do
		if eval echo \${${lib}_funcs} | tr ' ' '\n' \
			| grep -q "^${func}$"
		then
			test -n "${enabled}" && printf ","
			printf "${lib}"
			enabled="yes"
		fi
	done
	test -n "${enabled}" && echo || echo "<disabled>"
done
