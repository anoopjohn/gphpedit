## 
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2003-2005 Andy Jeffries <andy@gphpedit.org>
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

pixmaps_DATA = \
	gphpedit.png\
	classbrowser.png\
	class.png\
	function.png\
	gedit-plugin.png
pixmapsdir = \
	$(datadir)/pixmaps

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = gphpedit.png
## don't have svg icon we need one
## svgicondir = $(datadir)/icons/hicolor/scalable/apps
## svgicon_DATA = gphpedit.svg
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor
install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		$(gtk_update_icon_cache); \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		echo "***   $(gtk_update_icon_cache)"; \
	fi
EXTRA_DIST = \
        $(icon_DATA) \
	$(pixmaps_DATA)
##	$(svgicon_DATA)
