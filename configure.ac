#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cado], [0.9.1], [info@v2.cs.unibo.it])
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AC_CONFIG_SRCDIR([pam_check.h])
AC_CONFIG_HEADERS([config.h])
CFLAGS="$CFLAGS -Wall"

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([s2argv], [s2argv], [],
		[
		AC_MSG_ERROR([Could  not find S2ARGV library (https://github.com/rd235/s2argv-execs)])
		])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([sys/capability.h],
		[],
		[AC_MSG_ERROR([missing libcap header])])
AC_CHECK_HEADERS([security/pam_appl.h security/pam_misc.h],
		[],
		[AC_MSG_ERROR([missing PAM headers])])
AC_CHECK_HEADERS([s2argv.h],
		[],
		[AC_MSG_ERROR([missing S2ARGV headers])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup strtoull])

AC_DEFUN([CADO_CONF_VAR],
[AC_ARG_VAR([$1], [$2 @<:@$3@:>@])
if test "$$1" = ""; then
	$1='$3'
fi
])

AC_ARG_WITH([editor],
	[AC_HELP_STRING([--with-editor=EDITOR], [path to default editor])],
	[editor_defined="$with-editor"],
	[editor_defined="no"])

AS_IF([test "x$editor_defined" = "xno"], [
	AC_PATH_PROG([editor_defined], [vi], [/usr/bin/vi])
])

AC_DEFINE_UNQUOTED([EDITOR], ["$editor_defined"], [default editor])

# Set the paths.
CADO_CONF_VAR([SPOOL_DIR], [the directory where all the user scado files reside],[${localstatedir}/spool/cado])
CADO_CONF_VAR([CADO_EXE_DIR], [the directory where all the temporary executable files reside],[/tmp])

AC_OUTPUT([Makefile])
