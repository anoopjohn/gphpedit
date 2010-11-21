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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "main.h"
#include "document.h"
#include "document_manager.h"
#include "calltip.h"
#include "tab.h"
#include "menubar.h"
#include "toolbar.h"
#include "preferences_manager.h"
#include "templates.h"
#include "gtksourcestyleschememanager.h"


typedef struct
{
  /* preferences manager */
  PreferencesManager *prefmg;
  TemplatesManager *tempmg;

  GtkSourceStyleSchemeManager *stylemg;

  CalltipManager *clltipmg;

  GtkWidget *window;
  GtkWidget *prinbox;
  GtkWidget *prin_hbox;
  /* menu */
  GtkWidget *menu;
  /* toolbars */
  GtkWidget *toolbar_main;
  GtkWidget *toolbar_find;

  GtkWidget *main_vertical_pane;
  GtkWidget *main_horizontal_pane;
  /* status bar */
  GtkWidget *appbar;

  GtkWidget *notebook_manager;
  
  GtkWidget *notebook_editor;
  /* syntax check widget */
  GtkWidget *win;

  DocumentManager *docmg;

  //widget for close side bar button
  GtkWidget *close_sidebar_button;
  
  GtkWidget *classbrowser;

  /*filebrowser widget */
  GtkWidget *folder;
}
MainWindow;

extern MainWindow main_window;

void main_window_create(void);
void update_app_title(Document *document);
void main_window_pass_command_line_files(char **argv);
gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data );
extern GIOChannel* inter_gphpedit_io;
extern guint inter_gphpedit_event_id;
extern guint idle_id;
#endif
