dnl Process this file with autoconf to produce a configure script.

# AC_INIT would complain with "not a literal" when using MAJOR/MINOR/PATCH vars
# currently the sole purpose of the project is the library,
# so we use the library version also as project version
# don't forget to update MAJOR/MINOR/PATCH below!
AC_INIT(libdiscid, 0.5.0)
AC_CONFIG_SRCDIR(src/disc.c)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

# VERSION NUMBERING:
# see http://semver.org, we use that schema starting with 0.x.x
# in short:
# MAJOR is increased when backwards incompatible changes are made to the API
# MINOR is increased when features are added to the API
# PATCH is increase for all other releases
libdiscid_MAJOR=0
libdiscid_MINOR=5
libdiscid_PATCH=0
# don't forget to update AC_INIT above!

libdiscid_VERSION=${libdiscid_MAJOR}.${libdiscid_MINOR}.${libdiscid_PATCH}
libdiscid_VERSION_LT=${libdiscid_MAJOR}:${libdiscid_MINOR}:${libdiscid_PATCH}
libdiscid_VERSION_RC=${libdiscid_MAJOR},${libdiscid_MINOR},${libdiscid_PATCH},0
if test $libdiscid_PATCH -lt 10; then
  libdiscid_VERSION_NUM=${libdiscid_MINOR}0${libdiscid_PATCH}
else
  libdiscid_VERSION_NUM=${libdiscid_MINOR}${libdiscid_PATCH}
fi
AC_SUBST(libdiscid_VERSION)
AC_SUBST(libdiscid_VERSION_LT)
AC_SUBST(libdiscid_VERSION_RC)
AC_SUBST(libdiscid_VERSION_NUM)

AC_SUBST(libdiscid_MAJOR)
AC_SUBST(libdiscid_MINOR)
AC_SUBST(libdiscid_PATCH)

dnl Set the host_cpu, host_vendor, and host_os variables.
AC_CANONICAL_HOST

# compatibility with cmake
EXTRA_DIST=config-cmake.h.in
AC_DEFINE(libdiscid_EXPORTS, [], "the header is used to create the library")


case "${host_cpu}-${host_os}" in
  #*-beos*)    os=beos; LIBS='-lbe -lroot' ;;
  *-mingw*)   os=win32; LIBS='-lwinmm' ;;
  *-cygwin*)  os=win32; LIBS='-lwinmm' ;;
  *-freebsd*) os=freebsd ;;
  *-kfreebsd*) os=freebsd ;;
  *-darwin*)  os=darwin; LIBS='-framework CoreFoundation -framework IOKit';;
  *-linux*)   os=linux ;;
  *-netbsd*)  os=openbsd ;;
  *-openbsd*) os=openbsd ;;
  #*-os2_emx*) os=os2 ;;
  *-solaris*) os=solaris ;;
  #*-qnx*)     os=qnx; LIBS='-lsocket' ;;
  *)          os=generic;
	  AC_MSG_WARN([unsupported operating system, no disc reading available!]) ;;
esac
dnl AM_CONDITIONAL(DARWIN, test x$os = xdarwin)


dnl The file containing the operating system dependent disc access code.
if test "${os}" = "win32"; then
  DISC_OS_OBJ="disc_${os}.lo disc_${os}_new.lo"
  AC_MSG_NOTICE([using discid implementation disc_${os}.c disc_${os}_new.c])
  DISC_OS_OBJ="${DISC_OS_OBJ} ../versioninfo.lo"
else
  DISC_OS_OBJ="disc_${os}.lo"
  AC_MSG_NOTICE([using discid implementation disc_${os}.c])
fi
AC_SUBST(DISC_OS_OBJ)

LT_INIT([win32-dll])
LT_LANG([Windows Resource])

dnl Checks for programs.
AC_PROG_CC
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)  
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
  CFLAGS="$CFLAGS -Wall -O2"
fi


AC_OUTPUT([
  Makefile src/Makefile include/Makefile include/discid/Makefile
  examples/Makefile test/Makefile libdiscid.pc versioninfo.rc Doxyfile
  include/discid/discid.h
])
