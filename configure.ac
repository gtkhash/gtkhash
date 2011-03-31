AC_PREREQ(2.61)
AC_INIT(GtkHash, 0.4.0)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_SRCDIR(src/main.c)
AC_CONFIG_MACRO_DIR(m4)
AM_INIT_AUTOMAKE

AC_PROG_CC_C99
AM_PROG_CC_C_O
LT_INIT

AX_CFLAGS_GCC_OPTION(-pedantic)
AX_CFLAGS_GCC_OPTION(-Wall)
AX_CFLAGS_GCC_OPTION(-Wextra)

AX_CFLAGS_GCC_OPTION(-Waggregate-return)
AX_CFLAGS_GCC_OPTION(-Wbad-function-cast)
AX_CFLAGS_GCC_OPTION(-Wcast-align)
AX_CFLAGS_GCC_OPTION(-Wfloat-equal)
AX_CFLAGS_GCC_OPTION(-Wlogical-op)
AX_CFLAGS_GCC_OPTION(-Wmissing-declarations)
AX_CFLAGS_GCC_OPTION(-Wmissing-noreturn)
AX_CFLAGS_GCC_OPTION(-Wredundant-decls)
AX_CFLAGS_GCC_OPTION(-Wshadow)
AX_CFLAGS_GCC_OPTION(-Wswitch-default)
AX_CFLAGS_GCC_OPTION(-Wwrite-strings)

PKG_PROG_PKG_CONFIG

AC_CHECK_LIB(mhash, mhash, :, AC_MSG_ERROR(mhash library not found))
MHASH_LIBS="-lmhash"
AC_SUBST(MHASH_LIBS)

PKG_CHECK_MODULES(GTHREAD, gthread-2.0)
AC_SUBST(GTHREAD_CFLAGS)
AC_SUBST(GTHREAD_LIBS)

PKG_CHECK_MODULES(GDK, gdk-2.0)
AC_SUBST(GDK_CFLAGS)
AC_SUBST(GDK_LIBS)

PKG_CHECK_MODULES(GTK, gtk+-2.0)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

AC_ARG_ENABLE(nautilus, AS_HELP_STRING(--disable-nautilus, disable building the nautilus extension))
AM_CONDITIONAL(ENABLE_NAUTILUS, test "${enable_nautilus}" != "no")
AM_GCONF_SOURCE_2

if test "${enable_nautilus}" != "no" ; then
	# Check for nautilus
	PKG_CHECK_MODULES(NAUTILUS, libnautilus-extension)
	AC_SUBST(NAUTILUS_CFLAGS)
	AC_SUBST(NAUTILUS_LIBS)
	NAUTILUS_EXTENSION_DIR=`${PKG_CONFIG} --variable=extensiondir libnautilus-extension`
	AC_SUBST(NAUTILUS_EXTENSION_DIR)

	# Check for gconf
	PKG_CHECK_MODULES(GCONF, gconf-2.0)
	AC_SUBST(GCONF_CFLAGS)
	AC_SUBST(GCONF_LIBS)
	AC_PATH_PROG(GCONFTOOL, gconftool-2, no)
	if test "${GCONFTOOL}" = "no" ; then
		AC_MSG_ERROR(gconftool-2 not found)
	fi
fi

AM_CONDITIONAL(ENABLE_NLS, test "${enable_nls}" != "no")

if test "${enable_nls}" = "no" ; then
	AC_DEFINE_UNQUOTED(ENABLE_NLS, 0)
else
	GETTEXT_PACKAGE="${PACKAGE}"
	AC_SUBST(GETTEXT_PACKAGE)
	AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "${GETTEXT_PACKAGE}", Define the gettext package)
	IT_PROG_INTLTOOL(0.40.6)
	AM_GLIB_GNU_GETTEXT
fi

AC_DEFINE_UNQUOTED(FILE_BUFFER_SIZE, 131072, Define file buffer size)
AC_DEFINE_UNQUOTED(G_LOG_DOMAIN, "${PACKAGE_NAME}", Define the Glib log domain)

AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(data/Makefile)
AC_CONFIG_FILES(data/nautilus/Makefile)
AC_CONFIG_FILES(po/Makefile.in)
AC_CONFIG_FILES(src/Makefile)
AC_CONFIG_FILES(src/hash/Makefile)
AC_CONFIG_FILES(src/hash/mutils/Makefile)
AC_CONFIG_FILES(src/nautilus/Makefile)
AC_OUTPUT
