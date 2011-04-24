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

#include <string.h>
#include "document_types.h"
#include "preferences_manager.h"
#include "main_window.h"

gboolean is_cobol_file(const gchar *filename)
{
  return (g_str_has_suffix(filename,".cbl") || g_str_has_suffix(filename,".CBL"));
}

gboolean is_css_file(const gchar *filename)
{
  return (g_str_has_suffix(filename,".css"));
}

gboolean is_cxx_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".cxx") || g_str_has_suffix(filename,".c") || g_str_has_suffix(filename,".h")
  || g_str_has_suffix(filename,".cpp") || g_str_has_suffix(filename,".cc") || g_str_has_suffix(filename,".c++"))
      return TRUE;
  return FALSE;
}

gboolean is_perl_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".pl") || g_str_has_suffix(filename,".pm") || g_str_has_suffix(filename,".al") || g_str_has_suffix(filename,".perl"))
      return TRUE;
  return FALSE;
}

gboolean is_python_file(const gchar *filename)
{
  return (g_str_has_suffix(filename,".py"));
}

gboolean is_sql_file(const gchar *filename)
{
  return (g_str_has_suffix(filename,".sql"));
}

gboolean is_php_file_from_filename(const gchar *filename)
{
  // New style function for configuration of what constitutes a PHP file
  g_return_val_if_fail(filename, FALSE);
  gchar *file_extension;
  gchar **php_file_extensions;
  gboolean is_php = FALSE;
  gint i;

  file_extension = strrchr(filename, '.');
  if (file_extension) {
    file_extension++;
    
    const gchar *php_extensions;
    g_object_get(main_window.prefmg, "php_file_extensions", &php_extensions, NULL);

    php_file_extensions = g_strsplit(php_extensions, ",", -1);
    
    for (i = 0; php_file_extensions[i] != NULL; i++) {
      if (g_str_has_suffix(filename,php_file_extensions[i])){
        is_php = TRUE;
        break;
      }
    }
        
    g_strfreev(php_file_extensions);
  }
  
  return is_php;
}

gboolean is_php_file_from_content(const gchar *content)
{
  if (!content) return FALSE;
  // New style function for configuration of what constitutes a PHP file
  gboolean is_php = FALSE;
  gint i;
  gchar **lines;
    
  if (!is_php) { // If it's not recognised as a PHP file, examine the contents for <?php and #!.*php
    lines = g_strsplit(content, "\n", 10);
    if (!lines[0]) {
      g_strfreev(lines);
      return is_php;
    }
    if (lines[0][0] == '#' && lines[0][1] == '!' && strstr(lines[0], "php") != NULL) {
      is_php = TRUE;
    } else {
      for (i = 0; lines[i+1] != NULL; i++) {
        if (strstr (lines[i], "<?php") != NULL) {
          is_php = TRUE;
          break;
        }
      }
    }
    g_strfreev(lines);
  }
  return is_php;
}
