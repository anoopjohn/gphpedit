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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>

#include "main_window_callbacks.h"
#include "preferences_dialog.h"
#include "tab.h"
#include "templates.h"
#include "pluginmenu.h"
#include "gvfs_utils.h"
#include "gphpedit-statusbar.h"
#include "filebrowser_ui.h"
#include "classbrowser_ui.h"
#include "find_dialog.h"
#include "replace_dialog.h"

gboolean is_app_closing = FALSE;

// session_save relies on the fact that all tabs can be closed without 
// prompting, they should already be saved.  Also, the title won't be set
// afterwards.
void session_save(void)
{
  GSList *walk;
  Document *document;
  Document *current_focus_editor;
  GString *session_file;
  GString *session_file_contents;

  session_file = g_string_new(g_get_home_dir());
  session_file = g_string_append(session_file, "/.gphpedit/session");
  
  GFile *file=get_gfile_from_filename(session_file->str);
  g_string_free(session_file,TRUE);
  GError *error=NULL;


  if (get_preferences_manager_saved_session(main_window.prefmg) && (g_slist_length(editors) > 0)) {
    current_focus_editor = main_window.current_document;
    session_file_contents=g_string_new(NULL);
    for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
      document = walk->data;
      if (document) {
        if (!document_get_untitled(document)) {
          if (document == current_focus_editor) {
            session_file_contents = g_string_append(session_file_contents,"*");
          }
          if (document_is_scintilla_based(document)) {
            gchar *docfilename = document_get_filename(document);
            g_string_append_printf (session_file_contents,"%s\n",docfilename);
            g_free(docfilename);
          } else {
            if (document_get_document_type(document)==TAB_HELP){
              /* it's a help page */
              g_string_append_printf (session_file_contents,"phphelp:%s\n",document_get_help_function(document));
            } else if (document_get_document_type(document)==TAB_PREVIEW){
              /* it's a preview page */
              gchar *prevfilename = document_get_filename(document);
              gchar *temp= prevfilename;
              temp+=9;
              g_string_append_printf (session_file_contents,"preview:%s\n",temp);
              g_free(prevfilename);
            } else {
              g_print("error\n");
            }
          }
        }
      }
    }
    if(!g_file_replace_contents (file,session_file_contents->str,session_file_contents->len,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
      g_print(_("Error Saving session file: %s\n"),error->message);
      g_error_free (error);
    }
  if (session_file_contents) g_string_free(session_file_contents,TRUE);
  }
   g_object_unref(file);
}

void session_reopen(void)
{
  GString *session_file;
  char *filename;
  int focus_tab=-1;
  gboolean focus_this_one = FALSE;

  session_file = g_string_new( g_get_home_dir());
  session_file = g_string_append(session_file, "/.gphpedit/session");

  if (filename_file_exist(session_file->str)){
    gchar *content=read_text_file_sync(session_file->str);
    gchar **strings;
    strings = g_strsplit (content,"\n",0);
    int i=0;
    while (strings[i]){
      /* strings[i] contains possibly:
        file:///blah\n
        *file:///blah\n
        phphelp:function\n
        *phphelp:function\n
        preview:function\n
        *preview:function\n

      */

      if (strings[i][0]==0) break;
      filename = strings[i];
      str_replace(filename, 10, 0);
      if (strings[i][0]=='*') {
        filename++;
        focus_this_one = TRUE;
      }
        
      if (g_str_has_prefix(filename, "phphelp:")){
        filename += 8;
        add_new_document(TAB_HELP, filename, 0);
          
      } else if (g_str_has_prefix(filename, "preview:")){
        filename += 8;
        add_new_document(TAB_PREVIEW, filename, 0);
      } else {
        if (filename){
        switch_to_file_or_open(filename,0);
        }
      }
      if (focus_this_one && (main_window.current_document)) {
          focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor), document_get_editor_widget(main_window.current_document));
      }
      focus_this_one=FALSE;
      i++;    
    }
    g_strfreev (strings);
    g_free(content);
    gtk_notebook_set_current_page( GTK_NOTEBOOK(main_window.notebook_editor), focus_tab);
  }
  GFile *file=get_gfile_from_filename(session_file->str);
  GError *error=NULL;
  if (!g_file_delete (file,NULL,&error)){
      if (error->code!=G_FILE_ERROR_NOENT && error->code!=1){
        g_print(_("GIO Error deleting file: %s, code %d\n"), error->message,error->code);
      }
      g_error_free (error);
  }
  g_string_free(session_file,TRUE);
  g_object_unref (file);
}


/* Actual action functions */

void quit_application()
{
  preferences_manager_save_data(main_window.prefmg);
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:quit_application:Saved preferences\n"); }
  template_db_close();
  session_save();
  close_all_tabs();
}


void main_window_destroy_event(GtkWidget *widget, gpointer data)
{
  quit_application();
  cleanup_calltip();
  gtk_main_quit();
}


// This procedure relies on the fact that all tabs will be closed without prompting
// for whether they need saving beforehand.  If in doubt, call can_all_tabs_be_saved
// and pay attention to the return value.
void close_all_tabs(void)
{
  GSList *walk;
  Document *document;

  is_app_closing = TRUE;
  
  for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
    document = walk->data;
    if (document) {
      close_page(document);
      editors = g_slist_remove(editors, document);
    }
  }
  editors = NULL;
  main_window.current_document=NULL;
  is_app_closing = FALSE;
}

// Returns true if all tabs are either saved or closed
gboolean can_all_tabs_be_saved(void)
{
  GSList *walk;
  Document *document;
  gboolean saved;

  is_app_closing = TRUE;
  for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
    document = walk->data;
    if (document_get_editor_widget(document)) {
      if (!document_get_saved_status(document) && document_get_can_save(document)) {
        saved = try_save_page(document, FALSE);
        if (saved==FALSE) {
          is_app_closing = FALSE;
          return FALSE;
        }
      }
    }
  }
  is_app_closing = FALSE;
  return TRUE;
}


gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data){
  gboolean cancel_quit = FALSE;

  cancel_quit = !can_all_tabs_be_saved();

  if (cancel_quit) {
    update_app_title();
  }
  return cancel_quit;
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
  if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)) != 0) {
    set_preferences_manager_classbrowser_size(main_window.prefmg, gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)));
  }
  return TRUE;
}

gint main_window_key_press_event(GtkWidget   *widget, GdkEventKey *event,gpointer user_data)
{

  if (main_window.notebook_editor != NULL) {
    check_externally_modified();
    if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_ISO_Left_Tab)) {
      // Hack, for some reason when shift is held down keyval comes through as GDK_ISO_Left_Tab not GDK_Tab
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor)) == 0) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), gtk_notebook_get_n_pages(GTK_NOTEBOOK(main_window.notebook_editor))-1);
      }
      else {
        gtk_notebook_prev_page(GTK_NOTEBOOK(main_window.notebook_editor));
      }
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_Tab)) {
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor)) == gtk_notebook_get_n_pages(GTK_NOTEBOOK(main_window.notebook_editor))-1) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor),0);
      }
      else {
        gtk_notebook_next_page(GTK_NOTEBOOK(main_window.notebook_editor));
      }
      return TRUE;
    }
    else if ((event->state & GDK_MOD1_MASK)==GDK_MOD1_MASK && ((event->keyval >= GDK_0) && (event->keyval <= GDK_9))) {
      gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor),event->keyval - ((event->keyval == GDK_0) ? (GDK_0 - 9) : (GDK_0 + 1)));
      return TRUE;
    }
    else if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_space)) {
      document_show_calltip_at_current_pos(main_window.current_document);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_j) || (event->keyval == GDK_J)))  {
      template_find_and_insert();
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_F2)))  {
         document_modify_current_line_marker(main_window.current_document);
      return TRUE;
    }
    else if ((event->keyval == GDK_F2))  {
        document_find_next_marker(main_window.current_document);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_space)) { 
      document_force_autocomplete(main_window.current_document);
      return TRUE;
    }
  }
  return FALSE;
}

void on_new1_activate(GtkWidget *widget)
{
  // Create a new untitled tab
  add_new_document(TAB_FILE, NULL, 0);
}

void open_file_ok(GtkFileChooser *file_selection)
{
  GSList *filenames; 
  GSList *walk;

  // Extract filename from the file selection dialog
  filenames = gtk_file_chooser_get_uris(file_selection);
  
  for(walk = filenames; walk!= NULL; walk = g_slist_next(walk)) {
    switch_to_file_or_open(walk->data, 0);
  }
  g_slist_free(filenames);
}

void reopen_recent(GtkRecentChooser *chooser, gpointer data) {
  gchar *filename = gtk_recent_chooser_get_current_uri  (chooser);
  if (!filename) return;
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:reopen_recent:filename: %s\n", filename); }
  switch_to_file_or_open(filename, 0);
  g_free(filename);
}

void on_openselected1_activate(GtkWidget *widget)
{
  GSList *li;
  Document *document;
  gchar *ac_buffer;
  gchar *file;

  ac_buffer = document_get_current_selected_text(main_window.current_document);
  if (!ac_buffer){
    for(li = editors; li!= NULL; li = g_slist_next(li)) {
      document = li->data;
      if (document) {
         gchar *filename = document_get_filename(document);
         file = filename_parent_uri(filename);
         g_free(filename);
         if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate:file: %s\n", filename); }

        if (!strstr(ac_buffer, "://") && file) {
          gchar *filetemp= g_strdup_printf("%s/%s",file, ac_buffer);
          g_free(file);
          file=g_strdup(filetemp);
          g_free(filetemp);
        }
        else if (strstr(ac_buffer, "://")) {
            if (file) g_free(file);
            file = g_strdup(ac_buffer);
        }
        if(filename_file_exist(file)) switch_to_file_or_open(file,0);
        if (file) {
          g_free(file);
        }
      }
    }
  }
  g_free(ac_buffer);
}
void add_file_filters(GtkFileChooser *chooser){
  //store file filter
  GtkFileFilter *filter;
  //creates a new file filter
  filter = gtk_file_filter_new ();
  GString *caption;
  GString *ext_pattern = NULL;

  caption = g_string_new(_("PHP files ("));
  gchar **php_file_extensions;
  gint i;
  php_file_extensions = g_strsplit(get_preferences_manager_php_file_extensions(main_window.prefmg),",",-1);

  for (i = 0; php_file_extensions[i] != NULL; i++) {
    //make file pattern
    ext_pattern=g_string_new("*.");
    ext_pattern=g_string_append(ext_pattern, php_file_extensions[i]);
    caption =g_string_append(caption, " ");
    caption =g_string_append(caption, ext_pattern->str);
    gtk_file_filter_add_pattern(filter, ext_pattern->str);
  }
  g_strfreev(php_file_extensions);
  caption =g_string_append(caption, ")");
  gtk_file_filter_set_name (filter, caption->str);
  //add a pattern to the filter
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  //set default filter to the dialog
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("HTML files (*.html *.htm *.xhtml *tpl)"));
  gtk_file_filter_add_pattern(filter, "*.html");
  gtk_file_filter_add_pattern(filter, "*.htm");
  gtk_file_filter_add_pattern(filter, "*.xhtml");
  gtk_file_filter_add_pattern(filter, "*.tpl");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("XML files (*.xml)"));
  gtk_file_filter_add_pattern(filter, "*.xml");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Stylesheet files (*.css)"));
  gtk_file_filter_add_pattern(filter, "*.css");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("SQL files (*.sql)"));
  gtk_file_filter_add_pattern(filter, "*.sql");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Javascript files (*.js)"));
  gtk_file_filter_add_pattern(filter, "*.js");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);      
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("C/C++ files (*.c *.h *.cpp *.hh *.cc)"));
  gtk_file_filter_add_pattern(filter, "*.c");
  gtk_file_filter_add_pattern(filter, "*.h");
  gtk_file_filter_add_pattern(filter, "*.cxx");
  gtk_file_filter_add_pattern(filter, "*.cpp");
  gtk_file_filter_add_pattern(filter, "*.cc");
  gtk_file_filter_add_pattern(filter, "*.hxx");
  gtk_file_filter_add_pattern(filter, "*.hh");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Perl Files (*.pl *.ph *.pm)"));
  gtk_file_filter_add_pattern(filter, "*.pl");
  gtk_file_filter_add_pattern(filter, "*.ph");
  gtk_file_filter_add_pattern(filter, "*.pm");

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Cobol Files (*.cbl)"));
  gtk_file_filter_add_pattern(filter, "*.cbl");

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("Python Files (*.py *.pyd *.pyw)"));
  gtk_file_filter_add_pattern(filter, "*.py");
  gtk_file_filter_add_pattern(filter, "*.pyd");
  gtk_file_filter_add_pattern(filter, "*.pyw");

  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
  /* generic filter */
  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("All files"));
  gtk_file_filter_add_pattern(filter, "*");
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser), filter);
}

void on_open1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;
  gchar *folder;
  const gchar *last_opened_folder;
  // Create the selector widget
  file_selection_box = gtk_file_chooser_dialog_new("Please select files for editing", GTK_WINDOW(main_window.window),
    GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
  gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);  
  //Add filters to the open dialog
  add_file_filters(GTK_FILE_CHOOSER(file_selection_box));
  last_opened_folder = get_preferences_manager_last_opened_folder(main_window.prefmg);
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_open1_activate:last_opened_folder: %s\n", last_opened_folder); }
  /* opening of multiple files at once */
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_selection_box), TRUE);
  gchar *filename = (gchar *)document_get_filename(main_window.current_document);
  if (filename && !document_get_untitled(main_window.current_document)) {
    folder = filename_parent_uri(filename);
    if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_open1_activate:folder: %s\n", folder); }
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  folder);
    g_free(folder);
  }
  else if (last_opened_folder){
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  last_opened_folder);
  }
  
  if (gtk_dialog_run(GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
    open_file_ok(GTK_FILE_CHOOSER(file_selection_box));
  }
  gtk_widget_destroy(file_selection_box);
}

void save_file_as_ok(GtkFileChooser *file_selection_box)
{
  gchar *uri=gtk_file_chooser_get_uri(file_selection_box);
  // Set the filename of the current document to be that
  document_set_GFile(main_window.current_document, get_gfile_from_filename(uri));
  document_set_untitled(main_window.current_document, FALSE);
  gchar *basename = filename_get_basename(uri);
  g_free(uri);
  document_set_shortfilename(main_window.current_document, basename);

  // Call Save method to actually save it now it has a filename
  on_save1_activate(NULL);
}

void on_save1_activate(GtkWidget *widget)
{
  if (main_window.current_document) {
    //if document is Untitled
    if (document_get_untitled(main_window.current_document)) {
      on_save_as1_activate(widget);
    } else {
      /* show status in statusbar */
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Saving %s"), document_get_shortfilename(main_window.current_document));
      document_save(main_window.current_document);
    }
  }
}

void on_saveall1_activate(GtkWidget *widget)
{
  GSList *li;
  Document *doc;
  for(li = editors; li!= NULL; li = g_slist_next(li)) {
    doc = li->data;
    if (document_get_untitled(doc)){
      // g_print("Untitled found. Save not implemented\n");
    } else {
    document_save(doc);
    }
  }
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
}


void on_save_as1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;
  gchar *filename;
  const gchar *last_opened_folder;

  if (main_window.current_document) {
    // Create the selector widget
    file_selection_box = gtk_file_chooser_dialog_new (_("Please type the filename to save as..."), 
      GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    
    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(file_selection_box), TRUE);
    gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);
    
    if (main_window.current_document) {
      filename = document_get_filename(main_window.current_document);
      if (!document_get_untitled(main_window.current_document)) {
          gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(file_selection_box), filename);
      }
      else {
        last_opened_folder = get_preferences_manager_last_opened_folder(main_window.prefmg);
        if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:last_opened_folder: %s\n", last_opened_folder); }
        if (last_opened_folder){
          if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:Setting current_folder_uri to %s\n", last_opened_folder); }
          gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  last_opened_folder);
        }
      }
      g_free(filename);
    }
    if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
      save_file_as_ok(GTK_FILE_CHOOSER(file_selection_box));
    }
    gtk_widget_destroy(file_selection_box);    
  }
}
void on_reload1_activate(GtkWidget *widget)
{
  if (!document_get_saved_status(main_window.current_document)) {
    GtkWidget *file_revert_dialog;
    file_revert_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
            _("Are you sure you wish to reload the current file, losing your changes?"));
    gtk_window_set_title(GTK_WINDOW(file_revert_dialog), "Question");
    gint result = gtk_dialog_run (GTK_DIALOG (file_revert_dialog));
    if (result==GTK_RESPONSE_YES) {
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Opening %s"), 
          document_get_shortfilename(main_window.current_document));
    document_reload(main_window.current_document);
    }
    gtk_widget_destroy(file_revert_dialog);
  } else {
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Opening %s"), 
        document_get_shortfilename(main_window.current_document));
    
    document_reload(main_window.current_document);
  }
}

void on_tab_close_activate(GtkWidget *widget, Document *document)
{
  try_close_page(document);
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  update_app_title();
}

void rename_file(GString *newfilename)
{
  gchar *basename=filename_get_basename(newfilename->str);
  gchar *filename = document_get_filename(main_window.current_document);
  if (filename_rename(filename, basename)){
  // Set the filename of the current document to be that
  document_set_GFile(main_window.current_document, get_gfile_from_filename(newfilename->str));
  document_set_untitled(main_window.current_document, FALSE);
  document_set_shortfilename(main_window.current_document, basename);
  g_free(basename);
  // save as new filename
  on_save1_activate(NULL);
  }
  g_free(filename);
}

void rename_file_ok(GtkFileChooser *file_selection)
{
  GString *filename;
  // Extract filename from the file chooser dialog
  gchar *fileuri=gtk_file_chooser_get_uri(file_selection);
  filename = g_string_new(fileuri);
  g_free(fileuri);
  GFile *file = get_gfile_from_filename(filename->str);
  if (g_file_query_exists (file,NULL)) {
    gint result = yes_no_dialog (_("gPHPEdit"), _("This file already exists, are you sure you want to overwrite it?"));
    if (result==GTK_RESPONSE_YES) {
      rename_file(filename);
    }
    } else {
      rename_file(filename);
    }
  g_object_unref(file);
  g_string_free(filename, TRUE);
}


void on_rename1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;

  if (main_window.current_document) {
    // Create the selector widget
    file_selection_box = gtk_file_chooser_dialog_new(_("Please type the filename to rename this file to..."),
      GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    
    if (main_window.current_document) {
      gchar *filename = document_get_filename(main_window.current_document);
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(file_selection_box), filename);
      g_free(filename);
    }

    if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
      rename_file_ok(GTK_FILE_CHOOSER(file_selection_box));
    }

    gtk_widget_destroy(file_selection_box);
  }
}


void set_active_tab(page_num)
{
  Document *new_current_editor;

  new_current_editor = DOCUMENT(g_slist_nth_data(editors, page_num));

  if (new_current_editor) {
    //page_num = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),(new_current_editor->scintilla));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  }

  main_window.current_document = new_current_editor;

  update_app_title();

}
void update_zoom_level(void){
  gphpedit_statusbar_set_zoom_level((GphpeditStatusbar *)main_window.appbar, document_get_zoom_level(main_window.current_document));
}

/**
 * Close a tab in the Editor. Removes the notebook page, frees the editor object,
 * and sets the active tab correcty
 * 
 * @param editor - The editor object corresponding to the tab that is going to be closed.
 * @return void
 */

void close_page(Document *document)
{
  gint page_num;
  gint page_num_closing;
  gint current_active_tab;

  page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor), document_get_editor_widget(document));
  current_active_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor));
  
  if (page_num_closing != current_active_tab) {
    page_num = current_active_tab;
  }
  else {
    // If there is a tab before the current one then set it as the active tab.
    page_num = page_num_closing - 1;
    // If the current tab is the 0th tab then set the current tab as 0 itself.
    // If there are are subsequent tabs, then this will set the next tab as active.
    if (page_num < 0) {
      page_num = 0;
    }  
  }
  set_active_tab(page_num);
  gtk_notebook_remove_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num_closing);
  g_object_unref(document);
}

/**
 * get_window_icon
 * returns a pixbuf with gphpedit icon
 * @return GdkPixbuf
 */

GdkPixbuf *get_window_icon (void){
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (GPHPEDIT_PIXMAP_FULL_PATH, NULL);
  return pixbuf;
}


gboolean try_save_page(Document *document, gboolean close_if_can)
{

  GtkWidget *confirm_dialog;
  gint ret;
  GString *string;
  string = g_string_new("");
  confirm_dialog=gtk_message_dialog_new (GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_NONE,_("The file '%s' has not been saved since your last changes, are you sure you want to close it and lose these changes?"), document_get_shortfilename(document));
  gchar *filename = document_get_filename(document);
  g_string_printf(string,_("Unsaved changes to '%s'"), filename);
  g_free(filename);
  gtk_window_set_title(GTK_WINDOW(confirm_dialog), string->str);
  g_string_free(string,TRUE);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("Close and _lose changes"),0);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("_Save file"),1);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("_Cancel closing"),2);
  ret = gtk_dialog_run (GTK_DIALOG (confirm_dialog));
  gtk_widget_destroy(confirm_dialog);
  switch (ret) {
    case 0:
      if (close_if_can) {
          close_page(document);
          editors = g_slist_remove(editors, document);
          if (!editors) {
            main_window.current_document = NULL;
          }
      }
      return TRUE;
    case 1:
      on_save1_activate(NULL);
      // If chose neither of these, dialog either cancelled or closed. Do nothing.
  }
  return FALSE;
}


gboolean try_close_page(Document *document)
{
  if (document_get_can_save(document) && document_get_saved_status(document) && !document_get_is_empty(document)) {
    return try_save_page(document, TRUE);
  }
  else {
    close_page(document);
    editors = g_slist_remove(editors, document);
    if (!editors) {
      main_window.current_document = NULL;
    }
    return TRUE;
  }
}


void on_close1_activate(GtkWidget *widget)
{
  if (main_window.current_document != NULL) {
    try_close_page(main_window.current_document);
    classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
    update_app_title();
    update_zoom_level();
    classbrowser_force_label_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  }
}

void on_quit1_activate(GtkWidget *widget)
{
  if (!main_window_delete_event(NULL, NULL, NULL)) {
    quit_application();
    gtk_main_quit ();
  }
}

void on_cut1_activate(GtkWidget *widget)
{
  document_cut(main_window.current_document, main_window.clipboard);
}

void on_copy1_activate(GtkWidget *widget)
{
  document_copy(main_window.current_document, main_window.clipboard);
}


void selectiontoupper(void){
  document_selection_to_upper(main_window.current_document);
}

void selectiontolower(void){
  document_selection_to_lower(main_window.current_document);
}

void on_paste1_activate(GtkWidget *widget)
{
  document_paste(main_window.current_document, main_window.clipboard);
}


void on_selectall1_activate(GtkWidget *widget)
{
  document_select_all(main_window.current_document);
}


void on_find1_activate(GtkWidget *widget)
{
  if (main_window.current_document) {
    GtkWidget *find_dialog = search_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(find_dialog);    
  }
}


void on_replace1_activate(GtkWidget *widget)
{
  if (main_window.current_document) {
    GtkWidget *replace_dialog = replace_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(replace_dialog);    
  }
}

void on_undo1_activate(GtkWidget *widget)
{
  document_undo(main_window.current_document);
}


void on_redo1_activate(GtkWidget *widget)
{
  document_redo(main_window.current_document);
}


void keyboard_macro_startstop(GtkWidget *widget)
{
  document_keyboard_macro_startstop(main_window.current_document);
}

void keyboard_macro_playback(GtkWidget *widget)
{
  document_keyboard_macro_playback(main_window.current_document);
}


void on_preferences1_activate(GtkWidget *widget)
{
  preferences_dialog_create();

  if (gtk_dialog_run(GTK_DIALOG(preferences_dialog.window)) == GTK_RESPONSE_ACCEPT) {
    apply_preferences(NULL, NULL);
  }  
  // destroy/null dialog
  gtk_widget_destroy(preferences_dialog.window);
  preferences_dialog.window = NULL;
}


void context_help(GtkWidget *widget)
{
  gchar *buffer = NULL;

  buffer = document_get_current_selected_text(main_window.current_document);
  if (buffer){
    add_new_document(TAB_HELP, buffer, 0);
  }
}

/**
 * The about dialog box.
 */
void on_about1_activate(GtkWidget *widget)
{
  const gchar *authors[] = {
                "Current Maintainers",
                "Anoop John <anoop.john@zyxware.com>",
                "Jose Rostagno <rostagnojose@yahoo.com>",
                "",
                "Original Developer",
                "Andy Jeffries <andy@gphpedit.org>",
                "",
                "Assistance with Fixes/Enhancements:",
                "Jonh Wendell <wendell@bani.com.br>",
                "Tim Jackson <tim@timj.co.uk>",
                "Sven Herzberg <herzi@gnome-de.org>",
                NULL
               };
  gchar *translator_credits = _("translator_credits");
  const gchar *documenters[] = {NULL};
  GtkWidget *dialog = NULL;
  dialog=gtk_about_dialog_new();
  gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), PACKAGE_NAME);
  gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), VERSION);
  gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog),
      _("Copyright  2003-2006 Andy Jeffries, 2009-2010 Anoop John"));
  gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog),
     _("gPHPEdit is a GNOME2 editor specialised for editing PHP "
               "scripts and related files (HTML/CSS/JS)."));
  #ifdef PACKAGE_URL
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog),PACKAGE_URL);
  #endif
  GdkPixbuf *logo = NULL;
  logo=get_window_icon ();
  gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog),logo);
  if (logo != NULL) {
  g_object_unref (logo);
  }
  gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog),(const gchar **) authors);
  gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(dialog),translator_credits);
  gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG(dialog),(const gchar **) documenters);
  /* 
     http://library.gnome.org/devel/gtk/stable/GtkWindow.html#gtk-window-set-transient-for
     Dialog windows should be set transient for the main application window they were spawned from. 
     This allows window managers  to e.g. keep the dialog on top of the main window, or center the dialog over the main window.
  */
  gtk_window_set_transient_for (GTK_WINDOW(dialog),GTK_WINDOW(main_window.window));
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}
void update_status_combobox(Document *document){
      /* set statuscombo */
      switch(document_get_document_type(document)) {
        case(TAB_PHP):   
         set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("PHP/HTML/XML"));          
         break;
        case (TAB_CSS):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("CSS"));
          break;
        case (TAB_CXX):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("C/C++"));
          break;
        case (TAB_COBOL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("Cobol"));
          break;
        case (TAB_SQL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("SQL"));
          break;
        case (TAB_PERL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("Perl"));
          break;
        case (TAB_PYTHON):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("Python"));
          break;
        default:
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window.appbar),_("Text-Plain"));
      }
}
void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page,
                gint page_num, gpointer user_data)
{
  Document *data;
  GtkWidget *child;
  child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  data = document_manager_find_document ((void *) child);
  if (data){
    // Store it in the global main_window.current_editor value
    main_window.current_document = data;
    document_grab_focus(data);
  } else {
    g_print(_("Unable to get data for page %d\n"), page_num);
  }
  if (!is_app_closing) {
    // Change the title of the main application window to the full filename
    update_app_title();
    on_tab_change_update_classbrowser(main_window.notebook_editor);
  }
  check_externally_modified();
}

gboolean on_notebook_focus_tab(GtkNotebook *notebook,
                 GtkNotebookTab arg1, gpointer user_data)
{
  document_grab_focus(main_window.current_document);
  return TRUE;
}

void inc_search_typed (GtkEntry *entry, const gchar *text, gint length,
             gint *position, gpointer data)
{
  gchar *current_text;
  current_text = (gchar *)gtk_entry_get_text(entry);
  document_incremental_search(main_window.current_document, current_text, FALSE);
}

gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data)
{
  //Auto focus editor tab only if it is a scintilla tab
    if (event->keyval == GDK_Escape) {
        document_grab_focus(main_window.current_document);
      return TRUE;
    }

  return FALSE;
}

void add_to_search_history(const gchar *current_text){
    /* add text to search history*/
    GSList *walk;
    gint i=0;
    for (walk = get_preferences_manager_php_search_history(main_window.prefmg); walk!=NULL; walk = g_slist_next(walk)) {
      i++;
      if (g_strcmp0((gchar *) walk->data,current_text)==0){
        return;  /* already in the list */
        }
    }
    set_preferences_manager_new_search_history_item(main_window.prefmg, i, current_text);
    #ifdef DEBUG
    g_print("added:%s\n",current_text);
    #endif
    toolbar_completion_add_text(TOOLBAR(main_window.toolbar_find), current_text);
}


void inc_search_activate(GtkEntry *entry,gpointer user_data)
{
  gchar *current_text;

  //Inc search only if the current tab is not a help tab
  current_text = (gchar *)gtk_entry_get_text(entry);

  document_incremental_search(main_window.current_document, current_text, TRUE);
  add_to_search_history(current_text);

}

/*
gboolean is_valid_digits_only(gchar *text)
{
  while (*text) {
    if (*text < 48 || *text > 57) {
      return FALSE;
    }
  }

  return TRUE;
}
*/


void goto_line(gchar *text)
{
  gint line;
  line = atoi(text);
  document_goto_line(main_window.current_document,line);
}

void goto_line_activate(GtkEntry *entry,gpointer user_data)
{
  gchar *current_text;
  if (main_window.current_document) {
    current_text = (gchar *)gtk_entry_get_text(entry);
    goto_line(current_text);
  }
}


void block_indent(GtkWidget *widget)
{
  document_block_indent(main_window.current_document, get_preferences_manager_indentation_size(main_window.prefmg));
}


void block_unindent(GtkWidget *widget)
{
  document_block_unindent(main_window.current_document, get_preferences_manager_indentation_size(main_window.prefmg));
}

//zoom in
void zoom_in(GtkWidget *widget)
{
  document_zoom_in(main_window.current_document);
  update_zoom_level();
}

//zoom out
void zoom_out(GtkWidget *widget)
{
  document_zoom_out(main_window.current_document);
  update_zoom_level();
}

void zoom_100(GtkWidget *widget)
{
  document_zoom_restore(main_window.current_document);
  update_zoom_level();
}

void syntax_check(GtkWidget *widget)
{
   gtk_syntax_check_window_run_check(main_window.win, main_window.current_document);
}


void syntax_check_clear(GtkWidget *widget)
{
  gtk_widget_hide(GTK_WIDGET(main_window.win));
}


void classbrowser_show(void)
{
//  g_print("class browser show\n");
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane), get_preferences_manager_classbrowser_get_size(main_window.prefmg));
  set_preferences_manager_parse_classbrowser_status(main_window.prefmg, FALSE);
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
}


void classbrowser_hide(void)
{
  //g_print("class browser hide\n");
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),0);
  set_preferences_manager_parse_classbrowser_status(main_window.prefmg, TRUE);
}

void classbrowser_show_hide(GtkWidget *widget)
{
  gboolean hidden = get_preferences_manager_classbrowser_status(main_window.prefmg);
  menubar_set_classbrowser_status(MENUBAR(main_window.menu), hidden);
  if (hidden == 1)
    classbrowser_show();
  else
    classbrowser_hide();
}

void force_php(GtkWidget *widget)
{
    set_document_to_php(main_window.current_document);
}

void force_css(GtkWidget *widget)
{
    set_document_to_css(main_window.current_document);
}

void force_sql(GtkWidget *widget)
{
  set_document_to_sql(main_window.current_document);
}

void force_cxx(GtkWidget *widget)
{
  set_document_to_cxx(main_window.current_document);
}

void force_perl(GtkWidget *widget)
{
    set_document_to_perl(main_window.current_document);
}

void force_cobol(GtkWidget *widget)
{
  set_document_to_cobol(main_window.current_document);
}
void force_python(GtkWidget *widget)
{
  set_document_to_python(main_window.current_document);
}

//function to refresh treeview when the current tab changes 
//view is refreshed only if the parse only current file parameter is set
gint on_tab_change_update_classbrowser(GtkWidget *widget)
{
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  return FALSE;
}

void process_external (GtkInfoBar *info_bar, gint response_id, Document *document)
{
  if (response_id==1){
   gchar *filename = document_get_filename(document);
   gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Opening %s"), filename); 
    g_free(filename);
    document_reload(document);
  } else { 
   document_update_modified_mark(document); /*set current time*/
  }
  gtk_widget_hide (GTK_WIDGET(info_bar));  
}

void check_externally_modified(void)
{
    if (document_check_externally_modified(main_window.current_document)){
      gchar *filename = document_get_filename(main_window.current_document);
      gchar *message=g_strdup_printf(_("The file %s has been externally modified. Do you want reload it?"), filename);
      g_free(filename);
      gtk_label_set_text (GTK_LABEL (main_window.infolabel), message);
      g_free(message);
      gtk_widget_show (main_window.infobar);
      return;
    }
  gtk_widget_hide (main_window.infobar);
}

gboolean main_window_activate_focus (GtkWidget *widget,GdkEventFocus *event,gpointer user_data)
{
  check_externally_modified();
  return FALSE;
}
