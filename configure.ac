#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cado], [0.9], [info@v2.cs.unibo.it])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AC_CONFIG_SRCDIR([pam_check.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([sys/capability.h],
		[],
		[AC_MSG_ERROR([missing libcap header])])
AC_CHECK_HEADERS([security/pam_appl.h security/pam_misc.h],
		[],
		[AC_MSG_ERROR([missing PAM headers])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup strtoull])

AC_OUTPUT([Makefile])
