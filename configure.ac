dnl Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])

AC_INIT([EBView],[0.3.6],[http://ebview.sourceforge.net/])
AC_CONFIG_SRCDIR(src/ebview.c)
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

GETTEXT_PACKAGE=ebview
AC_SUBST(GETTEXT_PACKAGE)
ALL_LINGUAS="ja"
AM_GLIB_GNU_GETTEXT

EBVIEW_MAINTAINER_MODE

dnl AC_FUNC_SETVBUF_REVERSED

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
LT_INIT

dnl Checks for libraries.
AM_PATH_GTK_2_0(2.0.0, , AC_MSG_ERROR(Cannot find GTK+), gthread)

PKG_CHECK_MODULES(PANGOXFT, pangoxft)
AC_SUBST(PANGOXFT_CFLAGS)
AC_SUBST(PANGOXFT_LIBS)

eb_LIB_EB4

dnl Checks for header files.
AC_PATH_X
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(fcntl.h malloc.h sys/ioctl.h sys/time.h unistd.h eb/eb.h iconv.h libintl.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_PROG_GCC_TRADITIONAL
AC_C_CONST
AC_TYPE_PID_T
AC_HEADER_TIME
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_SYS_LARGEFILE

dnl Checks for library functions.
AC_CHECK_FUNCS(mkdir select strdup strtol)

AC_DEFINE_UNQUOTED(LOCALEDIR, "${PREFIX}/share/locale", Where .mo file is.)
AC_DEFINE_UNQUOTED(PACKAGEDIR, "${PREFIX}/share/${PACKAGE}", Where EBView data goes.)

AC_CONFIG_FILES([po/Makefile.in src/Makefile Makefile data/Makefile doc/Makefile])
AC_CONFIG_FILES([data/about.jp data/about.en])
AC_OUTPUT
