dnl
dnl This file is part of gPHPEdit
dnl
dnl  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy@gphpedit.org>
dnl  Copyright (C) 2010  Jose Rostagno <joserostagno@hotmail.com>
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
AC_PREREQ([2.59])
AC_INIT([gPHPEdit],[0.9.98],[https://savannah.nongnu.org/support/?group=gphpedit],[gphpedit],[http://www.gphpedit.org])

dnl autoconf versions before 2.64 don't have AC_PACKAGE_URL
m4_ifdef([AC_PACKAGE_URL],,
  [AC_DEFINE([PACKAGE_URL], ["http://www.gphpedit.org"], [Define to the home page for this package.])
   AC_SUBST([PACKAGE_URL], ["http://www.gphpedit.org"])])

# give autoconf the name of a file whose existence is a hint that
# autoconf has been started from the right directory.
AC_CONFIG_SRCDIR([src/main.c])

# tell autoconf where to put the temporary files it generates
AC_CONFIG_AUX_DIR([build-aux])

# Tell autoconf where to put the output file that contains all
# definitions our C program will need
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_MACRO_DIR([m4])

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

# Initialize automake, and tell it to tell the C compiler to output as
# many warnings as it can
AM_INIT_AUTOMAKE([-Wall foreign])

AM_MAINTAINER_MODE([enable])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Initialize libtool
LT_PREREQ([2.2.6])
LT_INIT(disable-static)

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stddef.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_C_CONST
# Checks for library functions.
AC_CHECK_FUNCS([isascii mblen memmove memset mkfifo strchr strncasecmp strrchr strstr strtol])

dnl  --------------
dnl | dependencies |-----------------------------------------------------------
dnl  --------------

# Checks for libraries.
dnl Checking for the required libraries.
GTK_REQUIRED_VERSION=2.20.0
GLIB_REQUIRED_VERSION=2.26.0
LIBXML_REQUIRED_VERSION=2.6.0

PKG_CHECK_MODULES(GTK, gtk+-2.0 >= $GTK_REQUIRED_VERSION)
PKG_CHECK_MODULES(GLIB, 	glib-2.0 >= $GLIB_REQUIRED_VERSION
                        	gio-2.0 >= $GLIB_REQUIRED_VERSION)
PKG_CHECK_MODULES(WEBKIT,[webkit-1.0])
PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= $LIBXML_REQUIRED_VERSION)

AC_PATH_PROG([CTAGS_EXUBERANT], [ctags-exuberant], [/usr/bin/ctags-exuberant],
                   [$PATH$PATH_SEPARATOR/usr/bin$PATH_SEPARATOR]dnl
     [/usr/sbin$PATH_SEPARATOR/usr/etc$PATH_SEPARATOR/etc])
if test "$CTAGS_EXUBERANT" != ""; then
 AC_DEFINE_UNQUOTED(HAVE_CTAGS_EXUBERANT, $ctags-exuberant,	[ctags-exuberant])
fi

dnl **********************
dnl  function declaration
dnl **********************

dnl $1: var
dnl $2: file
dnl $3: path
AC_DEFUN([AC_CHECK_DIR],
[path="$$1 \
       $3"
$1=""
for dir in $path ;
do
  if test -f $dir/$2; then
    $1=$dir
    break
  fi
done])

dnl $1: var
dnl $2: file
AC_DEFUN([AC_ASK_DIR],
[if test "$$1" == ""; then
  dnl AC_MSG_RESULT([Checking for php-doc])
  dnl echo -n "\$$1="
  read $1
  AC_MSG_CHECKING([for php-doc])
  if test -f $$1/$2; then
     AC_MSG_RESULT([$$1])
  else
    AC_MSG_RESULT([no])
    dnl AC_MSG_ERROR([still can not find $2 in $$1])
  fi
fi ])

dnl check for php-doc directory
AC_CHECK_DIR([php_doc_dir],[index.html],
        [/usr/doc/php-docs-200403/html \
 	/usr/share/doc/php-docs-20050822/html \
	 /usr/share/doc/php-docs-20050822/html \
	 /usr/doc/php-docs-4.2.3/html \
	/usr/share/doc/php-manual/en/html \
  /usr/share/doc/php/html \
	/usr/share/doc/php-doc/html ])
 AC_ASK_DIR([php_doc_dir],[index.html])
if test "$php_doc_dir" != ""; then
 AC_DEFINE_UNQUOTED(PHP_DOC_DIR, "$php_doc_dir",
	[php-doc directory])
fi

dnl define default translate url
AC_DEFINE_UNQUOTED(TRANSLATE_URL, "https://www.transifex.net/projects/p/gphpedit/c/main/",[default translate url])

dnl  ------------------
dnl | language support |-------------------------------------------------------
dnl  ------------------
dnl ================================================================
dnl Gettext stuff.
dnl ================================================================

# Generate po/LINGUAS on the fly rather than relying on translators
# to maintain it manually. This also overcome the problem that Transifex
# cannot add a language to po/LINGUAS if a new po file is submitted.
rm -f $srcdir/po/LINGUAS
for po_file in `ls $srcdir/po/*.po | sort`;
do
  lang=`echo "$po_file" | sed "s|.*/po/\(.*\)\.po|\1|g"`
  echo $lang >> $srcdir/po/LINGUAS
done

IT_PROG_INTLTOOL([0.40.0])
AC_SUBST([GETTEXT_PACKAGE], [gphpedit])
AM_GNU_GETTEXT_VERSION([0.17])
AM_GLIB_GNU_GETTEXT
AC_DEFINE(GETTEXT_PACKAGE,[PACKAGE_TARNAME], [Gettext package])

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

dnl ************************
dnl   Debug
dnl ************************

AC_ARG_ENABLE(debug,
	      AS_HELP_STRING([--enable-debug], [enable compilation of debugging messages]),
	      [case "${enableval}" in
	      yes) ENABLE_DEBUG=yes ;;
	      no) ENABLE_DEBUG=no ;;
	      *) AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
	      esac],
	      [ENABLE_DEBUG=no])
if test x$ENABLE_DEBUG = xyes; then
	AC_DEFINE(DEBUG, 1, [enable compilation of debugging messages])
	CFLAGS+=" -g -O0"
fi


dnl ************************
dnl   Locations
dnl ************************

if test "$prefix" == "NONE"; then
	AC_SUBST([prefix],[${ac_default_prefix}])
fi

if test "$exec_prefix" == "NONE"; then
	AC_SUBST([exec_prefix],[${prefix}])
fi

if test -z "$localedir"; then
	AC_SUBST([localedir],[${datadir}/locale])
fi

dnl define api data dir

API_DIR=$prefix/share/gphpedit
AC_DEFINE_UNQUOTED(API_DIR, "$API_DIR",
	[The directory in which gphpedit's api data will be stored])

dnl ================================================================
dnl GSettings stuff
dnl ================================================================

GLIB_GSETTINGS

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
data/org.gphpedit.gschema.xml.in
doc/Makefile
pixmaps/Makefile
themes/Makefile
po/Makefile.in
src/Makefile
])
AC_OUTPUT


dnl *******************************
dnl   Feedback
dnl *******************************
echo
echo "-------------------------------------------------"
echo "gPHPEDIT version    : $PACKAGE_VERSION"
echo "gPHPEDIT prefix     : $prefix"
echo "Debug               : $ENABLE_DEBUG"
if test "$php_doc_dir" != ""; then
echo "PHP-DOC help support: yes"
else
echo "PHP-DOC help support: no"
fi
echo "-------------------------------------------------"
echo "Now type Make to compile gPHPEdit"
echo "-------------------------------------------------"
