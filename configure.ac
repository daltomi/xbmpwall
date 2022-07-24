AC_PREREQ([2.69])

AC_INIT([xbmpwall],
		[1.16],
		[https://github.com/daltomi/xbmpwall/issues],
		[],
		[https://github.com/daltomi/xbmpwall])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_CONFIG_SRCDIR([src/xbmpwall.c])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_FILES([Makefile])

AC_CANONICAL_HOST

AC_PROG_CC

AS_IF([test "x$ac_cv_prog_cc_c11" = "xno"],
	  [AC_MSG_ERROR([Your compiler "$CC" doesn't support the C11 standard.])])

AC_PREPROC_IFELSE([AC_LANG_SOURCE(
				   [[#define _POSIX_C_SOURCE 200809L
				   #include <unistd.h>
				   #ifdef _POSIX_VERSION
				   #if _POSIX_VERSION != 200809L
				   #error "_POSIX_VERSION != 200809L"
				   #endif
				   #endif]])],
				   [AC_DEFINE([_POSIX_C_SOURCE], [200809L], [Define POSIX version])],
				   [AC_MSG_FAILURE([Implementation must conform to the POSIX.1-2008 standard])])


AC_CHECK_FUNCS([strndup strncpy chmod execl], [],
			   [AC_MSG_FAILURE([Implementation must conform to the POSIX.1-2008 standard])])

AC_CHECK_FUNCS([vsnprintf vsnprintf vfprintf], [],
			   [AC_MSG_ERROR([The functions are supported from C99. Note: glibc >= 2.19'])])

AC_CHECK_HEADERS([stdlib.h string.h unistd.h stdarg.h limits.h])

AC_TYPE_SIZE_T

AC_FUNC_FORK

AC_FUNC_MALLOC

AC_SEARCH_LIBS([XOpenDisplay], [X11], [],
			   [AC_MSG_ERROR([libX11 not found - install X11 devel package.])])

AC_SEARCH_LIBS([XtVaAppInitialize], [Xt], [],
			   [AC_MSG_ERROR([libXt not found - install X Toolkit devel package.])])

AC_SEARCH_LIBS([XawOpenApplication], [Xaw], [],
			   [AC_MSG_ERROR([libXaw not found - install Athenas Widget devel package.])])

AC_PATH_PROGS([xsetroot], [xsetroot], [no])

AS_IF([test x$xsetroot = xno],
	  [AC_MSG_ERROR([Not found external program required: xsetroot])])

AC_DEFINE_UNQUOTED([XSETROOT],
				   ["$xsetroot"],
				   [Define full path for xsetroot])

dnl TODO: other OS's
AS_CASE([$host_os],
	[linux*], [AC_PATH_PROGS([sh], [sh], [no])],
	[AC_PATH_PROGS([sh], [sh], [no])]
)

AS_IF([test x$sh = xno],
	  [AC_MSG_ERROR([Not found external program required: sh])])

AC_DEFINE_UNQUOTED([SH],
				   ["$sh"],
				   [Define full path for sh shell])
AC_OUTPUT

