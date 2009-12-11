/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/
 
   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
 
   The GNU General Public License is contained in the file COPYING.
*/

#include "tab_util.h"
#include "preferences.h"

gchar *merge_font (gchar *modifier)
{
	gint i;
	gchar **df, **sf;
	gchar *merged;

	df = g_strsplit (preferences.default_font, "-", -1);
	sf = g_strsplit (modifier, "-", -1);

	for (i = 1; (df[i] != NULL) && (sf[i] != NULL); i++) {
		if (g_strcasecmp (sf[i], "*") != 0)
			g_stpcpy (df[i], sf[i]);
	}

	merged = g_strjoinv ("-", df);

	g_strfreev (df);
	g_strfreev (sf);

	return merged;
}



gint scintilla_color(gint red, gint green, gint blue)
{
	return red | (green << 8) | (blue << 16);
}
