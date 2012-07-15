/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010, 2011 José Rostagno (for vijona.com.ar) 

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


#ifndef MAIN_WINDOW_CALLBACKS_H
#define MAIN_WINDOW_CALLBACKS_H
#include "main.h"
#include "main_window.h"

void on_new1_activate(GtkWidget *widget, gpointer user_data);
void reopen_recent(GtkRecentChooser *chooser, gpointer data);
void on_open1_activate(GtkWidget *widget, gpointer user_data);
void on_openselected1_activate(GtkWidget *widget, gpointer user_data);
void on_save1_activate(GtkWidget *widget, gpointer user_data);
void on_save_as1_activate(GtkWidget *widget, gpointer user_data);
void on_saveall1_activate(GtkWidget *widget, gpointer user_data);
void on_close1_activate(GtkWidget *widget, gpointer user_data);
void on_quit1_activate(GtkWidget *widget, gpointer user_data);
void on_cut1_activate(GtkWidget *widget, gpointer user_data);
void on_copy1_activate(GtkWidget *widget, gpointer user_data);
void on_paste1_activate(GtkWidget *widget, gpointer user_data);
void on_selectall1_activate(GtkWidget *widget, gpointer user_data);
void on_find1_activate(GtkWidget *widget, gpointer user_data);
void on_replace1_activate(GtkWidget *widget, gpointer user_data);
void on_properties1_activate(GtkWidget *widget, gpointer user_data);
void on_undo1_activate(GtkWidget *widget, gpointer user_data);
void on_redo1_activate(GtkWidget *widget, gpointer user_data);
void on_preferences1_activate(GtkWidget *widget, gpointer user_data);
void context_help(GtkWidget *widget, gpointer user_data);
void on_about1_activate(GtkWidget *widget, gpointer user_data);
void force_php(GtkWidget *widget, gpointer user_data);
void force_css(GtkWidget *widget, gpointer user_data);
void force_sql(GtkWidget *widget, gpointer user_data);
void force_cxx(GtkWidget *widget, gpointer user_data);
void force_perl(GtkWidget *widget, gpointer user_data);
void force_cobol(GtkWidget *widget, gpointer user_data);
void force_python(GtkWidget *widget, gpointer user_data);
void zoom_in(GtkWidget *widget, gpointer user_data);
void zoom_out(GtkWidget *widget, gpointer user_data);
void zoom_100(GtkWidget *widget, gpointer user_data);
void block_indent(GtkWidget *widget, gpointer user_data);
void block_unindent(GtkWidget *widget, gpointer user_data);
void selectiontoupper(GtkWidget *widget, gpointer user_data);
void selectiontolower(GtkWidget *widget, gpointer user_data);
void on_reload1_activate(GtkWidget *widget, gpointer user_data);
void keyboard_macro_startstop(GtkWidget *widget, gpointer user_data);
void keyboard_macro_playback(GtkWidget *widget, gpointer user_data);
#endif
