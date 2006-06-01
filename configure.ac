dnl Process this file with autoconf to produce a configure script.
dnl
dnl NOTE: Each time a release is made, the version number below and the
dnl       libtool version number in src/Makefile.am have to be incremented.
dnl       The libtool version number has to be changed ALWAYS, no matter how
dnl       small the change to the source code was!
dnl
AC_INIT(libdiscid, 0.0.0)
AC_CONFIG_SRCDIR(src/disc.c)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

dnl Set the host_cpu, host_vendor, and host_os variables.
AC_CANONICAL_HOST

case "${host_cpu}-${host_os}" in
  *-beos*)    os=beos; LIBS='-lbe -lroot' ;;
  *-cygwin*)  os=cygwin; LIBS='-lwinmm' ;;
  *-freebsd*) os=freebsd ;;
  *-darwin*)  os=darwin ;;
  *-irix*)    os=irix; CXXFLAGS="$CFLAGS -fsquangle"; 
                       LIBS='-lcdaudio -lmediad -lds' ;; 
  *-linux*)   os=linux ;;
  *-netbsd*)  os=netbsd ;;
  *-openbsd*) os=openbsd ;;
  *-os2_emx*) os=os2 ;;
  *-solaris*) os=solaris; LIBS='-lsocket -lnsl' ;;
  *-qnx*)     os=qnx; LIBS='-lsocket' ;;
  *)          AC_MSG_RESULT([WARNING: unknown system]) ;;
esac
AM_CONDITIONAL(DARWIN, test x$os = xdarwin)

dnl TODO: print OS, only create ${os} variables if we have an implementation

dnl The file containing the operating system dependent disc access code.
DISC_OS_OBJ=disc_${os}.lo
AC_SUBST(DISC_OS_OBJ)

dnl Checks for programs.
AC_PROG_CC
AM_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)  
AC_PROG_INSTALL


dnl Test endianness and size of a long; required for the SHA1 implementation.
AC_C_BIGENDIAN
AC_CHECK_SIZEOF(long)


if test "$GCC" = yes; then
  CFLAGS="$CFLAGS -Wall -O2"
fi


AC_OUTPUT([
  Makefile src/Makefile include/Makefile examples/Makefile libdiscid.pc
])
