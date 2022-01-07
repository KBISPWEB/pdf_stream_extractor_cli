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
  ],
  [AC_MSG_WARN([unsupported system: $host_os])]
)

#autoconf stuff
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_HEADERS([config.h])

#program config
AC_PROG_CC

#user and host info
# none

#headers
AC_CHECK_HEADERS([inttypes.h])
AC_CHECK_HEADERS([unistd.h])

#types
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T

#functions
# none

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
