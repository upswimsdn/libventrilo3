#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
LT_PREREQ([2.2.6])
AC_INIT([libventrilo3], [1.0.0], [eric@ypass.net])
LT_INIT
AM_INIT_AUTOMAKE

AM_MAINTAINER_MODE([disable])
AC_SUBST([LIBTOOL_DEPS])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_LIBTOOL

AX_PTHREAD([
LIBS="$PTHREAD_LIBS $LIBS"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CC="$PTHREAD_CC"
], [echo "error: cannot determine how to build threaded applications"; exit 1])
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

AC_CHECK_LIB([m], [tan], [], [
	echo "error: cannot find a working math library"
	exit -1
	])

AC_ARG_ENABLE(gsm,      [  --disable-gsm           disable GSM codec support],       [disable_gsm=yes])
AC_ARG_ENABLE(speex,    [  --disable-speex         disable Speex codec support],     [disable_speex=yes])
AC_ARG_ENABLE(celt,     [  --disable-celt          disable CELT codec support],      [disable_celt=yes])
AC_ARG_ENABLE(speexdsp, [  --disable-speexdsp      disable Speex resampler support], [disable_speexdsp=yes])

if test "$enable_gsm" != "no" ; then
	AC_CHECK_LIB([gsm], [gsm_create], [], [
		echo "error: cannot find libgsm: Is it installed? Also make sure you have the"
		echo "       libgsm development packages installed"
		exit -1
		])
	AC_CHECK_HEADER([gsm.h], [AC_DEFINE(HAVE_GSM_H, 1, [location of gsm.h])], [
		AC_CHECK_HEADER([gsm/gsm.h], AC_DEFINE(HAVE_GSM_GSM_H, 1, [location of gsm.h]), [
			echo "error: found gsm libraries, but headers are missing or corrupt. are you"
			echo "       missing a libgsm development package?"
			exit -1
			])
		])
	AC_DEFINE(HAVE_GSM, 1, [have gsm])
else
	AC_DEFINE(HAVE_GSM, 0, [have gsm])
fi

# Checks for header files.
if test "$enable_speex" != "no" ; then
	PKG_CHECK_MODULES(speex, speex >= 1.2)
	AC_SUBST(speex_CFLAGS)
	AC_SUBST(speex_LIBS)
	AC_DEFINE(HAVE_SPEEX, 1, [have speex])
else
	AC_DEFINE(HAVE_SPEEX, 0, [have speex])
fi

if test "$enable_celt" != "no" ; then
	PKG_CHECK_MODULES(celt, celt = 0.7.1)
	AC_SUBST(celt_CFLAGS)
	AC_SUBST(celt_LIBS)
	AC_DEFINE(HAVE_CELT, 1, [have celt])
else
	AC_DEFINE(HAVE_CELT, 0, [have celt])
fi

if test "$enable_speexdsp" != "no" ; then
	PKG_CHECK_MODULES(speexdsp, speexdsp >= 1.2)
	AC_SUBST(speexdsp_CFLAGS)
	AC_SUBST(speexdsp_LIBS)
	AC_DEFINE(HAVE_SPEEXDSP, 1, [have speexdsp])
else
	AC_DEFINE(HAVE_SPEEXDSP, 0, [have speexdsp])
fi

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_INT16_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([dup2 gettimeofday inet_ntoa memset select socket strchr strdup strerror gethostbyname_r])

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 examples/Makefile])

AC_OUTPUT
