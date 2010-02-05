dnl
dnl This file is part of gPHPEdit
dnl
dnl  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy@gphpedit.org>
dnl
dnl gPHPEdit is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl gPHPEdit is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License along
dnl with this program; if not, write to the Free Software Foundation, Inc.,
dnl 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
dnl
AC_PREREQ([2.64])
AC_INIT(gPHPEdit, 0.9.97, https://savannah.nongnu.org/support/?group=gphpedit)
AC_CONFIG_SRCDIR([config.h.in]) dnl maybe change to src/main.c
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_PROG_INTLTOOL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([isascii mblen memmove memset mkfifo strchr strncasecmp strrchr strstr strtol])

dnl  --------------
dnl | dependencies |-----------------------------------------------------------
dnl  --------------

PKG_CHECK_MODULES(GPHPEDIT,[
	gtk+-2.0
	webkit-1.0
	gconf-2.0
	])

AC_SUBST(GPHPEDIT_CFLAGS)
AC_SUBST(GPHPEDIT_LIBS)

dnl ---------------------------------------------------------------------------

dnl  ------------------
dnl | language support |-------------------------------------------------------
dnl  ------------------
dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

GETTEXT_PACKAGE=gphpedit
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT

dnl Please keep this in alphabetical order
ALL_LINGUAS="de es"

dnl AM_GLIB_GNU_GETTEXT
dnl AM_GNU_GETTEXT

dnl AM_GLIB_GNU_GETTEXT sets $DATADIRNAME
 AC_MSG_CHECKING(locale directory)
 if test "x$prefix" = "xNONE"; then
  PACKAGE_LOCALE_DIR=$ac_default_prefix/share/locale
 else
  PACKAGE_LOCALE_DIR=$prefix/share/locale
 fi
 AC_DEFINE_UNQUOTED(PACKAGE_LOCALE_DIR, "$PACKAGE_LOCALE_DIR",
	[The directory in which gphpedit's locale data will be stored])
 AC_MSG_RESULT("$PACKAGE_LOCALE_DIR")

dnl ---------------------------------------------------------------------------
dnl check for deprecated flags
   DISABLE_DEPRECATED_CFLAGS="\
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
-DGNOME_DISABLE_DEPRECATED"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
AC_CONFIG_FILES([
Makefile
gphpedit.desktop
gphpedit.spec
data/Makefile
doc/Makefile
pixmaps/Makefile
po/Makefile.in
src/Makefile
])
AC_OUTPUT

