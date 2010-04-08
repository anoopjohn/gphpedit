## 
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy@gphpedit.org>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	data	\
	doc	\
	pixmaps	\
	src \
	po

applicationsdir = $(datadir)/applications
applications_in_files = gphpedit.desktop.in
applications_DATA = $(applications_in_files:.desktop.in=.desktop)

spec_in_files = gphpedit.spec.in

EXTRA_DIST = \
	AUTHORS ChangeLog NEWS README 	\
	intltool−extract.in 		\
	intltool−merge.in   		\
	intltool−update.in		\
	gphpedit.spec			\
	gphpedit.applications		\
	$(applications_in_files)	\
	$(spec_in_files)		\
	src/gtkscintilla2

CLEANFILES = 			\
	intltool-extract 	\
	intltool-merge 		\
	intltool-update		\
        aclocal.m4 		\
	build-aux/config.guess  \
	build-aux/config.sub    \
	build-aux/depcomp 	\
	build-aux/install-sh 	\
	build-aux/missing 	\
	build-aux/ltmain.sh 	\
	src/stamp-h1	 	\
	src/config.h		\
	src/Makefile.in	 	\
	src/Makefile	 	\
	gphpedit.desktop	\
	gphpedit.spec		\
	config.log		\
	Makefile.in	 	\
	Makefile	 	\
	config.status		\
	data/Makefile.in	\
	data/Makefile	 	\
	doc/Makefile.in		\
	doc/Makefile	 	\
	pixmaps/Makefile.in	\
	pixmaps/Makefile
