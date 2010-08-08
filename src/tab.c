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
#include "main_window_callbacks.h"
#include "gvfs_utils.h"
#include "gphpedit-close-button.h"
#include "gphpedit-statusbar.h"
#include "classbrowser_ui.h"
#include "classbrowser_parse.h"

GSList *editors;

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
    
    php_file_extensions = g_strsplit(get_preferences_manager_php_file_extensions(main_window.prefmg),",",-1);
    
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

gboolean is_css_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".css"))
      return TRUE;
  return FALSE;
}

gboolean is_perl_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".pl") || g_str_has_suffix(filename,".pm"))
      return TRUE;
  return FALSE;
}

gboolean is_cobol_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".cbl") || g_str_has_suffix(filename,".CBL"))
      return TRUE;
  return FALSE;
}

gboolean is_python_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".py"))
      return TRUE;
  return FALSE;
}

gboolean is_cxx_file(const gchar *filename)
{
  if (g_str_has_suffix(filename,".cxx") || g_str_has_suffix(filename,".c") || g_str_has_suffix(filename,".h"))
      return TRUE;
  return FALSE;
}

gboolean is_sql_file(const gchar *filename)
{
if (g_str_has_suffix(filename,".sql"))
      return TRUE;
  return FALSE;
}

void register_file_opened(gchar *filename)
{
  gphpedit_debug_message(DOC_MANAGER,"filename: %s\n" filename);
  gchar *full_filename=filename_get_uri(filename);
  main_window_add_to_reopen_menu(full_filename);
  g_free(full_filename);
  gchar *folder = filename_parent_uri(filename);
  set_preferences_manager_last_opened_folder(main_window.prefmg, folder);
  g_free(folder);
}

gboolean switch_to_file_or_open(gchar *filename, gint line_number)
{
  gphpedit_debug(DOC_MANAGER);
  Document *document;
  GSList *walk;
  GString *tmp_filename;
  // need to check if filename is local before adding to the listen
  filename = g_strdup(filename);
  for (walk = editors; walk!=NULL; walk = g_slist_next(walk)) {
    document = walk->data;
    gchar *docfilename = document_get_filename(document);
    if (g_strcmp0(docfilename, filename)==0) {
      gtk_notebook_set_current_page( GTK_NOTEBOOK(main_window.notebook_editor), gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),document_get_editor_widget(document)));
      main_window.current_document = document;
      document_goto_line(main_window.current_document, line_number);
      g_free(docfilename);
      return TRUE;
    }
    g_free(docfilename);
  }
  tmp_filename = g_string_new(filename);
  add_new_document(TAB_FILE, filename, line_number);
  g_string_free(tmp_filename, TRUE);
  register_file_opened(filename);
  g_free(filename);
  return TRUE;
}

void document_load_complete_cb (Document *doc, gboolean result, gpointer user_data){
  if (result) {
    editors = g_slist_append(editors, doc);
    GtkWidget *document_tab;
    document_tab = get_close_tab_widget(doc);
    GtkWidget *document_widget = document_get_editor_widget(doc);
    gtk_widget_show(document_widget);
    gtk_notebook_append_page (GTK_NOTEBOOK (main_window.notebook_editor), document_widget, document_tab);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (main_window.notebook_editor), -1);
    main_window.current_document = doc;
    document_grab_focus(main_window.current_document);
    update_app_title(); 
    session_save();
   classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  }
}

void document_save_update_cb (Document *doc, gpointer user_data){
  update_app_title();
}

void add_new_document(gint type, const gchar *filename, gint goto_line){
  gphpedit_debug(DOC_MANAGER);
  Document *doc = document_new (type, filename, goto_line);
  g_signal_connect(G_OBJECT(doc), "load_complete", G_CALLBACK(document_load_complete_cb), NULL);
  g_signal_connect(G_OBJECT(doc), "save_update", G_CALLBACK(document_save_update_cb), NULL);
  document_load(doc);
}

GtkWidget *get_close_tab_widget(Document *document) {
  GtkWidget *hbox;
  GtkWidget *close_button;
  hbox = gtk_hbox_new(FALSE, 0);
  close_button = gphpedit_close_button_new ();
  gtk_widget_set_tooltip_text(close_button, _("Close Tab"));

  g_signal_connect(G_OBJECT(close_button), "clicked", G_CALLBACK(on_tab_close_activate), document);
  /* load file icon */
  GtkWidget *icon= gtk_image_new_from_pixbuf (document_get_document_icon(document));
  gtk_widget_show (icon);
  gtk_box_pack_start(GTK_BOX(hbox), icon, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), document_get_editor_label(document), FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(hbox), close_button, FALSE, FALSE, 0);
  gtk_widget_show(close_button);
  gtk_widget_show(hbox);
  return hbox;
}

Document *document_manager_find_document (void *widget){
  gphpedit_debug(DOC_MANAGER);
  GSList *walk;
  Document *document;

  for (walk = editors; walk != NULL; walk = g_slist_next (walk)) {
    document = walk->data;
    if (document_get_editor_widget(document) == GTK_WIDGET(widget)) {
      return walk->data;
    }
  }
  return NULL;
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
