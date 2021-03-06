dnl
dnl $Id: configure.in,v 1.3 2004/10/07 01:23:25 jnathan Exp $
dnl
dnl configure.in
dnl
dnl Copyright (C) 2002, 2003 Jeff Nathan <jeff@snort.org>
dnl
dnl Suggested reading:  "GNU Autoconf, Automake and Libtool" New Riders Press

AC_PREREQ(2.61)
AC_INIT(nemesis, 1.5-beta1, https://github.com/troglobit/nemesis/issues)
AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([src/nemesis.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])

AH_TOP(
#undef PACKAGE
#undef VERSION
#undef FREEBSD
#undef LINUX
#undef MACOS
#undef OPENBSD
#undef NETBSD
#undef SOLARIS
#undef u_int8_t
#undef u_int16_t
#undef u_int32_t
)

dnl Determine host type before performing further tests.
AC_CANONICAL_HOST

dnl Set the default prefix.
if test "$prefix" = "NONE"; then
    prefix="/usr/local"
fi

AM_PROG_CC_STDC

AC_PROG_CC

AC_PROG_GCC_TRADITIONAL
if test -n "$GCC"; then
    CFLAGS="${CFLAGS} -Wall"
fi

AC_ARG_ENABLE(debug,
    [  --enable-debug          enable debugging options (bugreports and developers)],
        [ if test -n "$GCC"; then
              CFLAGS="-g -O0 -Wall"
          fi
          CPPFLAGS="${CPPFLAGS} -DDEBUG" ],)

AC_ARG_ENABLE(profile,
    [  --enable-profile        enable profiling options (developers only)],
        [ if test -n "$GCC"; then
              CFLAGS="${CFLAGS} -pg -static"
          fi ],)

case "$host" in
  *-linux*)
    AC_DEFINE(LINUX)	
    ;;

  *-freebsd*)
    AC_DEFINE(FREEBSD)
    ;;

  *-openbsd*)
    AC_DEFINE(OPENBSD)
    ;;

  *-netbsd*)
    AC_DEFINE(NETBSD)
    ;;

  *-solaris*)
    AC_DEFINE(SOLARIS)
    ;;

  *-apple*)
    AC_DEFINE(MACOS)
    LIBS="${LIBS} -L/sw/lib"
    CPPFLAGS="${CPPFLAGS} -I/sw/include"
    ;;

  *)
    AC_MSG_WARN(Your OS is not officially supported yet)
    AC_MSG_WARN(Nemesis may not work properly)
    ;;
esac

dnl AC_ARG_ENABLE(pcapoutput,
dnl     [  --enable-pcapoutput     enable writing packets to binary pcap files],
dnl     CPPFLAGS="${CPPFLAGS} -DENABLE_PCAPOUTPUT",)

AC_ARG_WITH(libnet_includes,
    [  --with-libnet-includes=DIR    libnet include directory],
    [with_libnet_includes="$withval"],
    [with_libnet_includes=no])

AC_ARG_WITH(libnet_libraries,
    [  --with-libnet-libraries=DIR	libnet library directory],
    [with_libnet_libraries="$withval"],
    [with_libnet_libraries=no])

if test "$with_libnet_includes" != "no"; then
   CPPFLAGS="${CPPFLAGS} -I${with_libnet_includes}"
fi   

if test "$with_libnet_libraries" != "no"; then
   LIBS="${LIBS} -L${with_libnet_libraries}"
fi

dnl AC_ARG_WITH(libpcap_includes,
dnl     [  --with-libpcap-includes=DIR   libcap include directory],
dnl     [with_libpcap_includes="$withval"],
dnl     [with_libpcap_includes=no])

dnl AC_ARG_WITH(libpcap_libraries,
dnl     [  --with-libpcap-libraries=DIR  libcap library directory],
dnl     [with_libpcap_libraries="$withval"],
dnl     [with_libpcap_libraries=no])

dnl if test "$with_libpcap_includes" != "no"; then
dnl     CPPFLAGS="${CPPFLAGS} -I${with_libpcap_includes}"
dnl fi
  
dnl if test "$with_libpcap_libraries" != "no"; then
dnl     LIBS="${LIBS}  -L${with_libpcap_libraries}"
dnl fi

CPPFLAGS="${CPPFLAGS} `libnet-config --defines`"
CFLAGS="${CFLAGS} -funroll-loops -pipe"

if test `libnet-config --cflags | wc -c` = "1"; then
    CPPFLAGS="${CPPFLAGS} -I/usr/local/include -I/sw/include"
    LIBNET_CONFIG_BROKEN_CFLAGS=yes
else
    CPPFLAGS="${CPPFLAGS} `libnet-config --cflags`"
fi

if test `libnet-config --libs | wc -c` = "1"; then
    AC_MSG_WARN(libnet-config --libs is broken on your system.  If you)
    AC_MSG_WARN(are using a precompiled package please notify the maintainer.)
    LIBS="${LIBS} -L/usr/local/lib -L/sw/lib"
else
    LIBS="${LIBS} `libnet-config --libs | sed s/\-lnet//`"
fi

AC_DEFUN([HEADER_FAIL_MESSAGE],[
   echo
   echo
   echo "**********************************************"
   echo "  ERROR: unable to find" $1
   echo "  checked in the following places"
   for i in `echo $2`; do
       echo "        $i"
   done  
   echo "**********************************************"
   echo
   exit 1
])

AC_DEFUN([FUNCTION_FAIL_MESSAGE],[
   echo
   echo
   echo "**********************************************"
   echo "  ERROR: Nemesis cannot compile without"
   echo "         the function $1"
   echo "**********************************************"
   echo
   exit 1
])

AC_C_CONST

dnl Program tests
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S

dnl Library tests
AC_CHECK_LIB(m, fabs)
AC_CHECK_LIB(nsl, inet_ntoa)
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(resolv, hstrerror)

LNET=""
AC_CHECK_LIB(net, libnet_build_ipv4,, LNET="no")

if test "$LNET" = "no"; then
    echo
    echo "   ERROR!  Libnet library not found, go get it from"
    echo "   http://www.packetfactory.net/projects/libnet/"
    echo "   or use the --with-libnet-* options, if you have it installed"
    echo "   in unusual place"
    exit
fi

dnl if test "$enable_pcapoutput" != "no" -a "$enable_pcapoutput" = "yes"; then
dnl     LPCAP=""
dnl     AC_CHECK_LIB(pcap, pcap_datalink,, LPCAP="no")
dnl     if test "$LPCAP" = "no"; then
dnl         echo
dnl         echo "   ERROR!  Libpcap library not found, go get it from"
dnl         echo "   http://www.tcpdump.org"
dnl         echo "   or use the --with-libpcap-* options, if you have it installed"
dnl         echo "   in unusual place"
dnl         exit
dnl     fi
dnl fi

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
dnl AC_CHECK_HEADERS(unistd.h errno.h strings.h netdb.h limits.h machine/endian.h sys/time.h sys/param.h arpa/inet.h netinet/in.h netinet/ip_var.h)
AC_CHECK_HEADERS(unistd.h errno.h strings.h netdb.h limits.h machine/endian.h sys/time.h sys/param.h arpa/inet.h netinet/in.h)

LNET=""
AC_CHECK_HEADERS(libnet.h,, LNET="no")

if test "$LNET" = "no"; then
   echo
   echo "   ERROR!  Libnet header not found, go get it from"
   echo "   http://www.packetfactory.net/projects/libnet/"
   echo "   or use the --with-libnet-* options, if you have it installed"
   echo "   in unusual place"
   exit
fi

AC_MSG_CHECKING(for libnet version 1.1.6)
if test "${LIBNET_CONFIG_BROKEN_CFLAGS}" = "yes"; then
    if test -n "$with_libnet_includes" -a "$with_libnet_includes" != "no"; then
        libnet_dir="${with_libnet_includes}"
    else
        libnet_dir="/usr/include /usr/local/include /sw/include"
    fi
else
    libnet_dir=`libnet-config --cflags | cut -dI -f2`
fi

LIBNET_INC_DIR=""
for i in $libnet_dir; do
    if test -r $i/libnet.h; then
        LIBNET_INC_DIR=$i
    fi
done

if test "$LIBNET_INC_DIR" != ""; then
    if eval "grep LIBNET_VERSION $LIBNET_INC_DIR/libnet.h | grep -v 1.1.6 >/dev/null"; then
        AC_MSG_RESULT(no)
        echo
        echo "   ERROR!  Nemesis will *only* work with libnet version 1.1.6,"
        echo "   go get it from"
        echo "   http://www.packetfactory.net/projects/libnet/"
        HEADER_FAIL_MESSAGE("libnet 1.1.6 (libnet.h)", $LIBNET_INC_DIR)
    fi
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
    HEADER_FAIL_MESSAGE("libnet 1.1.6 (libnet.h)", $libnet_dir)
fi

dnl if test "$enable_pcapoutput" != "no" -a "$enable_pcapoutput" = "yes"; then
dnl     LPCAP=""
dnl     AC_CHECK_HEADERS(pcap.h,, LPCAP="no")
dnl     if test "$LPCAP" = "no"; then
dnl         echo
dnl         echo "   ERROR!  Libpcap header not found, go get it from"
dnl         echo "   http://www.tcpdump.org"
dnl         echo "   or use the --with-libpcap-* options, if you have it installed"
dnl         echo "   in unusual place"
dnl         exit
dnl     fi
dnl fi

dnl Checks for u_intx typedefs
AC_CHECK_TYPE(u_int8_t, uint8_t)
AC_CHECK_TYPE(u_int16_t, uint16_t)
AC_CHECK_TYPE(u_int32_t, uint32_t)

AC_TYPE_SIZE_T
AC_STRUCT_TM

dnl Checks for library functions.
AC_CHECK_FUNCS(gethostbyname,,FUNCTION_FAIL_MESSAGE("gethostbyname"))
AC_CHECK_FUNCS(gettimeofday,,FUNCTION_FAIL_MESSAGE("gettimeofday"))
AC_CHECK_FUNCS(inet_ntoa,,FUNCTION_FAIL_MESSAGE("inet_ntoa"))
AC_CHECK_FUNCS(isascii,,FUNCTION_FAIL_MESSAGE("isascii"))
AC_FUNC_MEMCMP
AC_CHECK_FUNCS(memset,,FUNCTION_FAIL_MESSAGE("memset"))
AC_CHECK_FUNCS(strchr,,FUNCTION_FAIL_MESSAGE("strchr"))
AC_CHECK_FUNCS(strdup,,FUNCTION_FAIL_MESSAGE("strdup"))
AC_CHECK_FUNCS(strerror,,FUNCTION_FAIL_MESSAGE("strerror"))
AC_CHECK_FUNCS(strrchr,,FUNCTION_FAIL_MESSAGE("strrchr"))
AC_CHECK_FUNCS(strtoul,,FUNCTION_FAIL_MESSAGE("strtoul"))
AC_REPLACE_FUNCS(getopt inet_aton strlcat strlcpy strsep)

CFLAGS=`echo $CFLAGS | sed -e 's/-I\/usr\/include //g'`
CPPFLAGS=`echo $CPPFLAGS | sed -e 's/-I\/usr\/include //g'`

AC_OUTPUT
