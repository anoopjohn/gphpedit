/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#ifndef DOCUMENT_TYPES_H
#define DOCUMENT_TYPES_H

#include <glib.h>

gboolean is_cobol_file(const gchar *filename);
gboolean is_css_file(const gchar *filename);
gboolean is_cxx_file(const gchar *filename);
gboolean is_perl_file(const gchar *filename);
gboolean is_python_file(const gchar *filename);
gboolean is_sql_file(const gchar *filename);
gboolean is_php_file_from_filename(const gchar *filename);
gboolean is_php_file_from_content(const gchar *content);

#endif
