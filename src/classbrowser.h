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

#ifndef CLASS_BROWSER_H
#define CLASS_BROWSER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "preferences.h"
#include "tab.h"
#include "classbrowser_parse.h"

extern guint classbrowser_id;


typedef struct
{
  gchar *filename;
  gboolean accessible;
  //time_t modified_time;
   GTimeVal modified_time;
}
ClassBrowserFile;

typedef struct
{
  gchar *functionname;
  gchar *paramlist;
  gchar *filename;
  guint line_number;
  guint class_id;
  gchar *classname;
  gboolean remove;
  guint identifierid;
}
ClassBrowserFunction;

typedef struct
{
  gchar *classname;
  gchar *filename;
  guint line_number;
  gboolean remove;
  guint identifierid;
}
ClassBrowserClass;

void classbrowser_set_sortable(GtkTreeStore *classtreestore);
void classbrowser_update(void);
void classbrowser_add_custom_autocompletion(GString *result, gchar *prefix);
void classbrowser_filelist_remove(ClassBrowserFile *file);
void classbrowser_functionlist_start_file(gchar *filename);
void classbrowser_functionlist_remove_dead_wood(void);
void classbrowser_classlist_add(gchar *classname, gchar *filename, gint line_number);
void classbrowser_functionlist_add(gchar *classname, gchar *funcname, gchar *filename, guint line_number, gchar *param_list);
void autocomplete_member_function(GtkWidget *scintilla, gint wordStart, gint wordEnd);
void classbrowser_update_selected_label(gchar *filename, gint line);
gint classbrowser_compare_function_names(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data);

#endif
