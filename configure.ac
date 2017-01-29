dnl Process this file with autoconf to produce a configure script.

# VERSION NUMBERING:
# see http://semver.org, we use that schema starting with 0.x.x
# in short:
# MAJOR is increased when backwards incompatible changes are made to the API
# MINOR is increased when features are added to the API
# PATCH is increase for all other releases
define(MAJOR, 0)
define(MINOR, 6)
define(PATCH, 2)
# currently the sole purpose of the project is the library,
# so we use the library version also as project version
AC_INIT(libdiscid, MAJOR.MINOR.PATCH)

AC_CONFIG_SRCDIR([src/disc.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])	# where to put config.guess etc.
AM_INIT_AUTOMAKE([foreign subdir-objects])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

libdiscid_MAJOR=MAJOR
libdiscid_MINOR=MINOR
libdiscid_PATCH=PATCH

libdiscid_VERSION_LT=${libdiscid_MAJOR}:${libdiscid_MINOR}:${libdiscid_PATCH}
libdiscid_VERSION_RC=${libdiscid_MAJOR},${libdiscid_MINOR},${libdiscid_PATCH},0
if test $libdiscid_PATCH -lt 10; then
  libdiscid_VERSION_NUM=${libdiscid_MINOR}0${libdiscid_PATCH}
else
  libdiscid_VERSION_NUM=${libdiscid_MINOR}${libdiscid_PATCH}
fi
AC_SUBST(libdiscid_VERSION_LT)
AC_SUBST(libdiscid_VERSION_RC)
AC_SUBST(libdiscid_VERSION_NUM)

AC_SUBST(libdiscid_MAJOR)
AC_SUBST(libdiscid_MINOR)
AC_SUBST(libdiscid_PATCH)

dnl Set the host_cpu, host_vendor, and host_os variables.
AC_CANONICAL_HOST

AC_DEFINE(libdiscid_EXPORTS, [], "the header is used to create the library")


case "${host_cpu}-${host_os}" in
  #*-beos*)    os=beos;;
  *-mingw*)   os=win32;;
  *-cygwin*)  os=win32;;
  *-freebsd*) os=freebsd ;;
  *-kfreebsd*) os=freebsd ;;
  *-dragonfly*) os=freebsd ;;
  *-darwin*)  os=darwin;;
  *-linux*)   os=linux ;;
  *-netbsd*)  os=netbsd ;;
  *-openbsd*) os=netbsd ;;
  #*-os2_emx*) os=os2 ;;
  *-solaris*) os=solaris ;;
  #*-qnx*)     os=qnx;;
  *)          os=generic;
	  AC_MSG_WARN([unsupported operating system, no disc reading available!]) ;;
esac

AC_MSG_NOTICE([Using disc read implementation for: $os])
#AM_CONDITIONAL([OS_BEOS], [test x$os = xbeos])
AM_CONDITIONAL([OS_DARWIN], [test x$os = xdarwin])
AM_CONDITIONAL([OS_FREEBSD], [test x$os = xfreebsd])
AM_CONDITIONAL([OS_GENERIC], [test x$os = xgeneric])
AM_CONDITIONAL([OS_LINUX], [test x$os = xlinux])
AM_CONDITIONAL([OS_NETBSD], [test x$os = xnetbsd])
#AM_CONDITIONAL([OS_OS2], [test x$os = xos2])
#AM_CONDITIONAL([OS_QNX], [test x$os = xqnx])
AM_CONDITIONAL([OS_SOLARIS], [test x$os = xsolaris])
AM_CONDITIONAL([OS_WIN32], [test x$os = xwin32])

LT_INIT([win32-dll])
LT_LANG([Windows Resource])

dnl Checks for programs.
AC_PROG_CC
AM_PROG_LIBTOOL
AC_PROG_INSTALL

dnl Test endianness and size of a long; required for the SHA1 implementation.
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(long)

if test "$cross_compiling" = "yes" && test "$os" = "win32"; then
  AC_MSG_WARN([detected cross compilation: disabling tests!!!])
  #AC_SUBST([TESTS_ENVIRONMENT], [wine]) doesn't work for libtool builds
  tests_enabled=no
else
  tests_enabled=yes
fi
AM_CONDITIONAL([RUN_TESTS], [test x${tests_enabled} = xyes])

if test "$GCC" = yes; then
  WARN_CFLAGS="-Wall"
fi

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# for libmusicbrainz5 example:
#AC_SEARCH_LIBS([mb5_query_query], [musicbrainz5], [have_mb5=yes], [have_mb5=no])
AC_CHECK_HEADER([musicbrainz5/mb5_c.h], [have_mb5=yes])
AM_CONDITIONAL([HAVE_MUSICBRAINZ5], [test x${have_mb5} = xyes])

AC_OUTPUT([
  Makefile libdiscid.pc versioninfo.rc Doxyfile include/discid/discid.h
])
