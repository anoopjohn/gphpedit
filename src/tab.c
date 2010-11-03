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
#include "tab.h"
#include "debug.h"
#include "main_window.h"
#include "gvfs_utils.h"

/**
* str_replace
* replaces tpRp with withc in the string
*/
void str_replace(char *Str, char ToRp, char WithC)
{
  int i = 0;

  while(i < strlen(Str)) {
    if(Str[i] == ToRp) {
      Str[i] = WithC;
    }
    i++;
  }
}

void info_dialog (gchar *title, gchar *message)
{
  GtkWidget *dialog;
  gint button;
  dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"%s", message);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_window_set_transient_for (GTK_WINDOW(dialog),GTK_WINDOW(main_window.window));
  button = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
  /*
   * Run the dialog and wait for the user to select yes or no.
   * If the user closes the window with the window manager, we
   * will get a -4 return value
   */
}

gint yes_no_dialog (gchar *title, gchar *message)
{
  GtkWidget *dialog;
  gint button;
  dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_YES_NO,"%s", message);
  gtk_window_set_title(GTK_WINDOW(dialog), title);
  gtk_window_set_transient_for (GTK_WINDOW(dialog),GTK_WINDOW(main_window.window));
  button = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
  /*
   * Run the dialog and wait for the user to select yes or no.
   * If the user closes the window with the window manager, we
   * will get a -4 return value
   */
         
  return button;
}

void register_file_opened(gchar *filename)
{
  gphpedit_debug_message(DEBUG_DOC_MANAGER,"filename: %s\n", filename);
  gchar *folder = filename_parent_uri(filename);
  g_object_set (main_window.prefmg, "last_opened_folder", folder, NULL);
  g_free(folder);
}

/************************/
/**
 * trunc_on_char:
 * @string: a #gchar * to truncate
 * @which_char: a #gchar with the char to truncate on
 *
 * Returns a pointer to the same string which is truncated at the first
 * occurence of which_char
 *
 * Return value: the same gchar * as passed to the function
 **/
gchar *trunc_on_char(gchar * string, gchar which_char)
{
  gchar *tmpchar = string;
  while(*tmpchar) {
    if (*tmpchar == which_char) {
      *tmpchar = '\0';
      return string;
    }
    tmpchar++;
  }
  return string;
}
