AC_PREREQ([2.71])
AC_INIT([ioutils], [0.2], [bugs@kara-online.net])

#host switch
AC_CANONICAL_HOST
AS_CASE(["$host_os"],
  [*linux*], [
    OS_VERSION=linux:w

    AC_DEFINE([OS_LINUX], [],
      [Defined if the target OS is Linux])
  ],
  [*mingw*|*cygwin*], [
    OS_VERSION=windows
    AC_DEFINE([OS_WINDOWS], [],
      [Defined if the target OS is Windows])
  ],
  [AC_MSG_WARN([unsupported system: $host_os])]
)
AM_CONDITIONAL([HOST_WINDOWS], [test "x$OS_VERSION" = "xwindows"])

#autoconf stuff
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

#program config
AC_PROG_CC
AM_PROG_AR
AC_C_RESTRICT
AC_PROG_RANLIB

#user and host info
# none

#headers
AC_CHECK_HEADER([regex.h], [], [
  AC_MSG_ERROR([regex.h not found but is required. Make sure it's installed for $host.])
], [])

#types
AC_TYPE_SIZE_T

#functions
AC_FUNC_REALLOC
AC_CHECK_FUNCS([regcomp])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
])
AC_OUTPUT
