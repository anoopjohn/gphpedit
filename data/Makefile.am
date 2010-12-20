## 
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003-2005 Andy Jeffries <andy@gphpedit.org>
##  Copyright (C) 2010  Jose Rostagno <joserostagno@hotmail.com>
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

api_DATA = \
	php-gphpedit.api\
	c.api\
	python.api\
	perl.api\
	css.api
apidir = \
	$(datadir)/gphpedit

gsettingsschema_in_files = org.gphpedit.gschema.xml
gsettings_SCHEMAS = $(gsettingsschema_in_files:.xml.in=.xml)

@GSETTINGS_RULES@

@INTLTOOL_XML_NOMERGE_RULE@

EXTRA_DIST = \
	$(api_DATA) \
  $(gsettingsschema_in_files)
