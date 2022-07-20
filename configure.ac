#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(ubackup, 1.0)
AC_CONFIG_SRCDIR([cui/main.cpp])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PREFIX_DEFAULT(/usr)
# Checks for libraries.

AC_PATH_PROG([RESTICBIN], [restic], [/usr/bin/restic])
AC_DEFINE_UNQUOTED([RESTICBIN], ["$RESTICBIN"], [Path of restic program.])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.

AC_OUTPUT(
	Makefile
	lib/Makefile
	cui/Makefile
	lib/internal/Makefile
	lib/utils/Makefile
)