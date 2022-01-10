AC_PREREQ([2.71])
AC_INIT([psec], [0.1], [bugs@kara-online.net])

#host switch
AC_CANONICAL_HOST
AS_CASE(["$host_os"],
  [*linux*], [
    OS_VERSION=linux
    AC_DEFINE([PSEC_OS_LINUX], [],
      [Defined if the target OS is Linux])
  ],
  [*mingw*|*cygwin*], [
    OS_VERSION=windows
    AC_DEFINE([PSEC_OS_WINDOWS], [],
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
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([src/config.h])

#program config
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR
AC_C_INLINE

#user and host info
# none

#headers
AC_CHECK_HEADER_STDBOOL
AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([fcntl.h])
AC_CHECK_HEADER([zlib.h], [], [
  AC_MSG_ERROR([zlib.h not found but is required. Make sure it's installed for $host.])
], [])
AC_CHECK_HEADER([regex.h], [], [
  AC_MSG_ERROR([regex.h not found but is required. Make sure it's installed for $host.])
], [])

#types
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AC_TYPE_OFF_T
AC_CHECK_TYPES([ptrdiff_t])

#functions
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strerror])
AC_CHECK_FUNCS([memchr])
AC_CHECK_FUNCS([memset])
AC_CHECK_FUNCS([mkdir])
AC_CHECK_FUNCS([munmap])
AC_CHECK_FUNCS([strchr])
AC_CHECK_FUNCS([strdup])
AC_CHECK_FUNCS([strrchr])
AC_CHECK_FUNCS([utime])

AC_CONFIG_FILES([
  Makefile
  src/Makefile
  src/ext/Makefile
  src/ioutils/Makefile
])
AC_OUTPUT
