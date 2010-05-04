/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno (for vijona.com.ar) 

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
#ifndef TOOLBAR_H
#define TOOLBAR_H
#include <gtk/gtk.h>

typedef struct
{
        GtkWidget *toolbar;
	GtkWidget *button_new;
	GtkWidget *button_open;
	GtkWidget *button_save;
	GtkWidget *button_save_as;
	GtkWidget *button_close;
	GtkWidget *button_undo;
	GtkWidget *button_redo;
	GtkWidget *button_cut;
	GtkWidget *button_copy;
	GtkWidget *button_paste;
	GtkWidget *button_find;
	GtkWidget *button_replace;
	GtkToolItem *toolbar_separator;
	GtkWidget *button_indent;
	GtkWidget *button_unindent;
	GtkWidget *button_zoom_in;
	GtkWidget *button_zoom_out;
        GtkWidget *button_zoom_100;
}
Maintoolbar;
typedef struct
{
        GtkWidget *toolbar;
 	GtkToolItem *toolbar_separator;
 	GtkWidget *toolbar_find;
	GtkWidget *search_label;
	GtkWidget *search_entry;
	GtkWidget *goto_label;
	GtkWidget *goto_entry;
        GtkWidget *cleanimg;
	GtkEntryCompletion *completion;
	GtkTreeModel *completion_model;
}
Findtoolbar;

void main_window_create_maintoolbar(void);
void main_window_create_findtoolbar(void);
GtkTreeModel *create_completion_model (void);
#endif /*TOOLBAR_H*/
