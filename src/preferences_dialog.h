/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003-2005 Andy Jeffries
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


#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H

#include "main.h"
#include "tab_php.h"
#include "tab_css.h"
#include "tab_sql.h"

typedef struct
{
  GtkWidget *window;
  GtkWidget *vbox5;
  GtkWidget *notebook1;
  GtkWidget *vbox6;
  GtkWidget *hbox14;
  GtkWidget *hbox26;
  GtkWidget *label32;
  GtkWidget *tab_size;
  GtkWidget *tabs_not_spaces;
  GtkWidget *hbox15;
  GtkWidget *label33;
  GtkWidget *edge_colour;
  GtkWidget *hbox16;
  GtkWidget *label34;
  GtkWidget *edge_column;
  GtkWidget *save_session;
  GtkWidget *label29;
  GtkWidget *vbox10;
  GtkWidget *hbox22;
  GtkWidget *frame2;
  GtkWidget *vbox11;
  GtkWidget *bold_button;
  GtkWidget *italic_button;
  GtkWidget *label40;
  GtkWidget *frame3;
  GtkWidget *vbox12;
  GtkWidget *hbox23;
  GtkWidget *label42;
  GtkWidget *foreground_colour;
  GtkWidget *hbox24;
  GtkWidget *label43;
  GtkWidget *background_colour;
  GtkWidget *label41;
  GtkWidget *code_sample;
  GtkWidget *textview2;
  GtkWidget *frame4;
  GtkWidget *hbox25;
  GtkWidget *font_combo;
  GtkWidget *combo_entry2;
  GtkWidget *size_combo;
  GtkWidget *combo_entry3;
  GtkWidget *label44;
  GtkWidget *label45;
  GtkWidget *frame1;
  GtkWidget *element_combo;
  GtkWidget *combo_entry4;
  GtkWidget *label39;
  GtkWidget *label30;
  GtkWidget *vbox7;
  GtkWidget *hbox17;
  GtkWidget *label35;
  GtkWidget *php_file_entry;
  GtkWidget *combo_entry1;
  GtkWidget *hbox18;
  GtkWidget *label36;
  GtkWidget *file_extensions;
  GtkWidget *shared_source;
  GtkWidget *hbox19;
  GtkWidget *label37;
  GtkWidget *delay;
  GtkWidget *hbox20;
  GtkWidget *label38;
  GtkWidget *vbox8;
  GtkWidget *hbox21;
  GtkWidget *scrolledwindow1;
  GtkWidget *Templates;
  GtkListStore *template_store;
  GtkWidget *vbox9;
  GtkWidget *add_template_button;
  GtkWidget *edit_template_button;
  GtkWidget *delete_template_button;
  GtkWidget *template_sample_scrolled;
  GtkWidget *template_sample;
  GtkWidget *label31;
  GtkWidget *hbox13;
  GtkWidget *ok_button;
  GtkWidget *apply_button;
  GtkWidget *cancel_button;
  GtkWidget *show_indentation_guides;
  GtkWidget *edge_mode;
  GtkWidget *line_wrapping;
  GtkWidget *use_tabs_instead_spaces;
  GtkWidget *single_instance_only;
  GtkTreeSelection *template_selection;
  GList *highlighting_elements;

  gboolean changing_highlight_element;
  Editor *highlighting_editor;
}
PreferencesDialog;

extern PreferencesDialog preferences_dialog;

void preferences_dialog_create (void);
void apply_preferences(GtkButton *button, gpointer data);

#endif
