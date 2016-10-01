#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([mutex], [1.0], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([test/mutexTest.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE([subdir-objects])
# Checks for programs.
AC_PROG_RANLIB
AC_PROG_CXX
AC_PROG_CC
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_OUTPUT
