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
#include "calltip.h"
#include "tab.h"
#include "menu.h"
#include "toolbar.h"
#include "syntax_check_window.h"
#include "preferences_manager.h"

typedef struct
{
  /* preferences manager */
  Preferences_Manager *prefmg;

  GtkWidget *window;
  GtkWidget *prinbox;
  GtkWidget *prin_hbox;
  GtkWidget *infobar;
  GtkWidget *infolabel;
  /* menu */
  Mainmenu *menu;
  /* toolbars */
  Maintoolbar *toolbar_main;
  Findtoolbar *toolbar_find;

  GtkWidget *main_vertical_pane;
  GtkWidget *main_horizontal_pane;
  /* status bar */
  GtkWidget *appbar;

  GtkWidget *notebook_manager;
  
  GtkWidget *notebook_editor;
  /* syntax check widget */
  GtkSyntax_Check_Window *win;

  Editor *current_editor;

  //widget for close side bar button
  GtkWidget *close_sidebar_button;
  
  GtkWidget *classbrowser;

  GtkClipboard* clipboard;

  /*filebrowser widget */
  GtkWidget *folder;//filebrowser;
}
MainWindow;

extern MainWindow main_window;
extern gboolean DEBUG_MODE;
void force_config_folder(void);
void update_controls(void);
void main_window_create(void);
void main_window_open_command_line_files(char **argv, gint argc);
void update_app_title(void);
void main_window_pass_command_line_files(char **argv);
gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data );
void main_window_add_to_reopen_menu(gchar *full_filename);
void create_untitled_if_empty(void);
extern GIOChannel* inter_gphpedit_io;
extern guint inter_gphpedit_event_id;
extern guint idle_id;
#endif
