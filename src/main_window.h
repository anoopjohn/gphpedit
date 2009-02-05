/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries
      andy@gphpedit.org
	  
   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/
 
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
 
   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.
 
   The GNU General Public License is contained in the file COPYING.*/


#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "main.h"
#include "calltip.h"
#include "tab.h"


typedef struct
{
	GtkWidget *window;

	GtkWidget *toolbar_main;
	GtkWidget *toolbar_main_button_new;
	GtkWidget *toolbar_main_button_open;
	GtkWidget *toolbar_main_button_save;
	GtkWidget *toolbar_main_button_save_as;
	GtkWidget *toolbar_main_button_close;
	GtkWidget *toolbar_main_button_undo;
	GtkWidget *toolbar_main_button_redo;
	GtkWidget *toolbar_main_button_cut;
	GtkWidget *toolbar_main_button_copy;
	GtkWidget *toolbar_main_button_paste;
	GtkWidget *toolbar_main_button_find;
	GtkWidget *toolbar_main_button_replace;

	GtkWidget *toolbar_find;
	GtkWidget *toolbar_find_search_label;
	GtkWidget *toolbar_find_search_entry;
	GtkWidget *toolbar_find_goto_label;
	GtkWidget *toolbar_find_goto_entry;

	GtkWidget *main_vertical_pane;
	GtkWidget *main_horizontal_pane;

	GtkWidget *appbar;

	GtkWidget *notebook_manager;
	GtkWidget *notebook_editor;

	GtkWidget *scrolledwindow1;
	// My new best friend: http://developer.gnome.org/doc/API/2.0/gtk/TreeWidget.html
	GtkListStore *lint_store;
	GtkCellRenderer *lint_renderer;
	GtkWidget *lint_view;
	GtkTreeViewColumn *lint_column;
	GtkTreeSelection *lint_select;

	Editor *current_editor;

	GtkWidget *scrolledwindow3;
	GtkTreeStore *classtreestore;
	GtkWidget *classtreeview;
	GtkTreeSelection *classtreeselect;
	GtkWidget *treeviewlabel;
	GtkWidget *label1;
	/*
	GtkWidget *notebook_manager_functions_page;
	GtkWidget *label2;
	GtkWidget *notebook_manager_files_page;
	GtkWidget *label3;*/
	
	GtkClipboard* clipboard;
}
MainWindow;


#define CB_ITEM_TYPE_CLASS 1
#define CB_ITEM_TYPE_CLASS_METHOD 2
#define CB_ITEM_TYPE_FUNCTION 3

enum {
    NAME_COLUMN,
    LINE_NUMBER_COLUMN,
    FILENAME_COLUMN,
    TYPE_COLUMN,
    ID_COLUMN,
    N_COLUMNS
};


extern MainWindow main_window;
extern gboolean DEBUG_MODE;
void force_config_folder(void);

void main_window_create(void);
void main_window_open_command_line_files(char **argv, gint argc);
void update_app_title(void);
void main_window_create_unix_socket(void);
void main_window_pass_command_line_files(char **argv);
gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data );
GString *get_differing_part(GSList *filenames, gchar *file_requested);
void main_window_add_to_reopen_menu(gchar *full_filename);
void plugin_exec(gint plugin_num);
void create_untitled_if_empty(void);

extern GIOChannel* inter_gphpedit_io;
extern guint inter_gphpedit_event_id;

extern guint idle_id;

#endif
