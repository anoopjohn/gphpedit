#! /bin/sh
#
# This file is part of gPHPEdit
#
#  Copyright (C) 2010  Jose Rostagno <joserostagno@hotmail.com>
#
# gPHPEdit is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# gPHPEdit is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#
set -e

# Refresh GNU autotools toolchain.
echo Cleaning autotools files...
find -type d -name autom4te.cache -print0 | xargs -0 rm -rf \;
find -type f \( -name missing -o -name install-sh -o -name mkinstalldirs \
	-o -name depcomp -o -name ltmain.sh -o -name configure \
	-o -name config.sub -o -name config.guess \
	-o -name Makefile.in \) -print0 | xargs -0 rm -f

echo Running autoreconf...
AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
        echo "*** No autoreconf found, please install it ***"
        exit 1
else
        autoreconf --force --install --verbose || exit $?
fi
echo Running intltoolize
intltoolize --copy --force --automake

# run ./autogen.sh --rebuild-po-files
# to update po files with lastest strings from source code
if [ $1 = "--rebuild-po-files" ]
then
echo Running configure
./configure
cd "po"
make check
make update-po
else
echo Running configure with arguments "$@"
./configure $@
fi
