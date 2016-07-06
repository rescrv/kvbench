#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([kvbench], [0.0.dev], [robert@rescrv.net])
m4_define([serial_tests], [
    m4_esyscmd([case `automake --version | head -n 1` in
                *1.11*);;
                *) echo serial-tests;;
                esac])
])
AM_INIT_AUTOMAKE(foreign serial_tests subdir-objects dist-bzip2)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
LT_PREREQ([2.2])
LT_INIT
AC_CONFIG_SRCDIR([kvbench.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

AS_IF([make -h 2>/dev/null | grep no-print-directory > /dev/null], [make_no_print=yes])
AM_CONDITIONAL([MAKE_NO_PRINT_DIRECTORY], [test x"${make_no_print}" != x])

ANAL_WARNINGS

# Checks for libraries.
PKG_CHECK_MODULES([YGOR], [ygor >= 0.0])

AC_CHECK_LIB([popt],[poptGetContext],,[AC_MSG_ERROR([
-------------------------------------------------
kvbench relies upon the popt library.
Please install popt to continue.
-------------------------------------------------])])
AC_ARG_VAR(POPT_LIBS, [linker flags for popt])
AS_IF([test "x$POPT_LIBS" = x], [POPT_LIBS="-lpopt"])

# Checks for header files.
AC_CHECK_HEADER([popt.h],,[AC_MSG_ERROR([
-------------------------------------------------
kvbench relies upon the popt library.
Please install popt to continue.
-------------------------------------------------])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.

# Optional components

AC_ARG_VAR([LEVELDB_REPO],[The path to the LevelDB repo, where "make" has been run])
AC_SUBST([LEVELDB_REPO], [${LEVELDB_REPO}])
AM_CONDITIONAL([ENABLE_LEVELDB], [test x"${LEVELDB_REPO}" != x])

AC_ARG_VAR([ROCKSDB_REPO],[The path to the RocksDB repo, where "make" has been run])
AC_SUBST([ROCKSDB_REPO], [${ROCKSDB_REPO}])
AM_CONDITIONAL([ENABLE_ROCKSDB], [test x"${ROCKSDB_REPO}" != x])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
