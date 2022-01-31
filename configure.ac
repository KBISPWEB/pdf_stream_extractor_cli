AC_PREREQ([2.71])
AC_INIT([psec], [0.3], [bugs@kara-online.net])

#host switch
AC_CANONICAL_HOST
AS_CASE(["$host_os"],
  [*linux*], [
    OS_VERSION=linux
    AC_DEFINE([OS_LINUX], [],
      [Defined if the target OS is Linux])
    AC_CHECK_HEADERS([unistd.h])
  ],
  [*mingw*|*cygwin*], [
    OS_VERSION=windows
    AC_DEFINE([OS_WINDOWS], [],
      [Defined if the target OS is Windows])
    AC_CHECK_HEADERS([fileapi.h], [], [
      AC_MSG_ERROR([fileapi.h not found but is required for $host_os. Is your build host set correctly?])
    ], [])
  ],
  [AC_MSG_WARN([unsupported system: $host_os])]
)
AM_CONDITIONAL([HOST_WINDOWS], [test "x$OS_VERSION" = "xwindows"])

#autoconf stuff
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SUBDIRS([src/filext src/buffer])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

#program config
AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB
AC_C_RESTRICT

#user and host info
# none

#headers
AC_CHECK_HEADER([regex.h], [], [
  AC_MSG_ERROR([regex.h not found but is required. Make sure it's installed for $host.])
], [])

#types
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T

#functions
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([regcomp])
AC_CHECK_FUNCS([strstr])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
