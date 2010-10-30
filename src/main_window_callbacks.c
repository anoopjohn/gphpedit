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

#include "debug.h"
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
#include "document_manager.h"
#include "syntax_check_window.h"

gboolean is_app_closing = FALSE;

/* Actual action functions */

void quit_application()
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  g_object_unref(main_window.tempmg);
  is_app_closing = TRUE;
  g_object_unref(main_window.docmg);
  is_app_closing = FALSE;
  g_object_unref(main_window.prefmg);
  g_object_unref(main_window.stylemg);
  g_object_unref(main_window.clltipmg);
}


void main_window_destroy_event(GtkWidget *widget, gpointer data)
{
  quit_application();
  gtk_main_quit();
}

gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data){
  gboolean cancel_quit = FALSE;
  is_app_closing = TRUE;
  cancel_quit = !document_manager_can_all_tabs_be_saved(main_window.docmg);
  is_app_closing = FALSE;

  if (cancel_quit) {
    update_app_title(document_manager_get_current_document(main_window.docmg));
  }
  return cancel_quit;
}

gint main_window_key_press_event(GtkWidget   *widget, GdkEventKey *event,gpointer user_data)
{

  if (main_window.notebook_editor != NULL) {
    check_externally_modified(document_manager_get_current_document(main_window.docmg));
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
  }
  return FALSE;
}

void on_new1_activate(GtkWidget *widget)
{
  // Create a new untitled tab
  document_manager_add_new_document(main_window.docmg, TAB_FILE, NULL, 0);
}

void open_file_ok(GtkFileChooser *file_selection)
{
  GSList *filenames; 
  GSList *walk;

  // Extract filename from the file selection dialog
  filenames = gtk_file_chooser_get_uris(file_selection);
  
  for(walk = filenames; walk!= NULL; walk = g_slist_next(walk)) {
    document_manager_switch_to_file_or_open(main_window.docmg, walk->data, 0);
  }
  g_slist_free(filenames);
}

void reopen_recent(GtkRecentChooser *chooser, gpointer data) {
  gchar *filename = gtk_recent_chooser_get_current_uri  (chooser);
  if (!filename) return;
  gphpedit_debug_message(DEBUG_MAIN_WINDOW,"filename: %s", filename);
  document_manager_switch_to_file_or_open(main_window.docmg, filename, 0);
  g_free(filename);
}

void on_openselected1_activate(GtkWidget *widget)
{
  document_manager_open_selected(main_window.docmg);
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
  const gchar *php_extensions;
  g_object_get(main_window.prefmg, "php_file_extensions", &php_extensions, NULL);

  php_file_extensions = g_strsplit(php_extensions, ",", -1);

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

  filter = gtk_file_filter_new ();
  gtk_file_filter_set_name (filter, _("HTML files (*.html *.htm *.xhtml *.tpl)"));
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

  //set default filter to the dialog
  gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser), filter);
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
  /* opening of multiple files at once */
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_selection_box), TRUE);

  //Add filters to the open dialog
  add_file_filters(GTK_FILE_CHOOSER(file_selection_box));
  Document *document = document_manager_get_current_document(main_window.docmg);
  gchar *filename = (gchar *)document_get_filename(document);
  gboolean untitled;
  g_object_get(document, "untitled", &untitled, NULL);

  if (filename && !untitled) {
    folder = filename_parent_uri(filename);
    gphpedit_debug_message(DEBUG_MAIN_WINDOW,"folder: %s", folder);
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  folder);
    g_free(folder);
  } else {
    g_object_get (main_window.prefmg, "last_opened_folder", &last_opened_folder, NULL);
    gphpedit_debug_message(DEBUG_MAIN_WINDOW,"last_opened_folder: %s", last_opened_folder);
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box), last_opened_folder);
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
  Document *document = document_manager_get_current_document(main_window.docmg);
  g_object_set(document, "GFile", gtk_file_chooser_get_file(file_selection_box), NULL);
  gchar *basename = filename_get_basename(uri);
  g_free(uri);
  g_object_set(document, "untitled", FALSE,"short_filename", basename, NULL);

  // Call Save method to actually save it now it has a filename
  on_save1_activate(NULL);
}

void on_save1_activate(GtkWidget *widget)
{
  Document *document = document_manager_get_current_document(main_window.docmg);
  if (document) {
    gboolean untitled;
    g_object_get(document, "untitled", &untitled, NULL);
    //if document is Untitled
    if (untitled) {
      on_save_as1_activate(widget);
    } else {
      /* show status in statusbar */
      const gchar *short_filename;
      g_object_get(document, "short_filename", &short_filename, NULL);
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,_("Saving %s"), short_filename);
      document_save(document);
    }
  }
}

void on_saveall1_activate(GtkWidget *widget)
{
  document_manager_save_all(main_window.docmg);
}


void on_save_as1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;
  gchar *filename;
  const gchar *last_opened_folder;
  Document *document = document_manager_get_current_document(main_window.docmg);
  if (!document) return ;
  // Create the selector widget
  file_selection_box = gtk_file_chooser_dialog_new (_("Please type the filename to save as..."),
    GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
  
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(file_selection_box), TRUE);
  gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);

  gboolean untitled;
  g_object_get(document, "untitled", &untitled, NULL);
  if (!untitled) {
    filename = document_get_filename(document);
    gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(file_selection_box), filename);
    g_free(filename);
  } else {
    g_object_get (main_window.prefmg, "last_opened_folder", &last_opened_folder, NULL);
    gphpedit_debug_message(DEBUG_MAIN_WINDOW, "Setting current_folder_uri to %s", last_opened_folder);
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box), last_opened_folder);
  }
  if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
    save_file_as_ok(GTK_FILE_CHOOSER(file_selection_box));
  }
  gtk_widget_destroy(file_selection_box);
}
void on_reload1_activate(GtkWidget *widget)
{
  document_manager_document_reload(main_window.docmg);
}

void on_tab_close_activate(GtkWidget *widget, Document *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  document_manager_try_close_document(main_window.docmg, document);
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  update_app_title(document_manager_get_current_document(main_window.docmg));
}

void rename_file(GString *newfilename)
{
  Document *document = document_manager_get_current_document(main_window.docmg);
  gchar *basename=filename_get_basename(newfilename->str);
  gchar *filename = document_get_filename(document);
  if (filename_rename(filename, basename)){
  // Set the filename of the current document to be that
  g_object_set(document, "GFile", g_file_new_for_commandline_arg(newfilename->str), NULL);
  g_object_set(document, "untitled", FALSE, "short_filename", basename, NULL);
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
  GFile *file = g_file_new_for_commandline_arg(filename->str);
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

  if (document_manager_get_current_document(main_window.docmg)) {
    // Create the selector widget
    file_selection_box = gtk_file_chooser_dialog_new(_("Please type the filename to rename this file to..."),
      GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    
    if (document_manager_get_current_document(main_window.docmg)) {
      gchar *filename = document_get_filename(document_manager_get_current_document(main_window.docmg));
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
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  if(document_manager_set_current_document_from_position(main_window.docmg, page_num)) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  }
  update_app_title(document_manager_get_current_document(main_window.docmg));
}

void update_zoom_level(void){
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  Document *doc = document_manager_get_current_document(main_window.docmg);
  guint zoom_level;
  if (doc)
    g_object_get(document_manager_get_current_document(main_window.docmg), "zoom_level", &zoom_level, NULL);
  else
    zoom_level = 100;
  gphpedit_statusbar_set_zoom_level((GphpeditStatusbar *)main_window.appbar, zoom_level);
}

/**
 * Close a tab in the Editor. Removes the notebook page,
 * and sets the active tab correcty
 * 
 * @param editor - The editor object corresponding to the tab that is going to be closed.
 * @return void
 */

void close_page(Document *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gint page_num;
  gint page_num_closing;
  gint current_active_tab;
  GtkWidget *document_widget;

  g_object_get(document, "editor_widget", &document_widget, NULL);
  page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor), document_widget);
  current_active_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor));
  
  if (page_num_closing != current_active_tab) {
    page_num = current_active_tab;
  } else {
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

void on_close1_activate(GtkWidget *widget)
{
  document_manager_try_close_current_document(main_window.docmg);
  if(document_manager_get_document_count(main_window.docmg)!=0){
    classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
    update_app_title(document_manager_get_current_document(main_window.docmg));
    update_zoom_level();
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
  document_cut(document_manager_get_current_document(main_window.docmg), main_window.clipboard);
}

void on_copy1_activate(GtkWidget *widget)
{
  document_copy(document_manager_get_current_document(main_window.docmg), main_window.clipboard);
}


void selectiontoupper(void){
  document_selection_to_upper(document_manager_get_current_document(main_window.docmg));
}

void selectiontolower(void){
  document_selection_to_lower(document_manager_get_current_document(main_window.docmg));
}

void on_paste1_activate(GtkWidget *widget)
{
  document_paste(document_manager_get_current_document(main_window.docmg), main_window.clipboard);
}


void on_selectall1_activate(GtkWidget *widget)
{
  document_select_all(document_manager_get_current_document(main_window.docmg));
}


void on_find1_activate(GtkWidget *widget)
{
  if (document_manager_get_current_document(main_window.docmg)) {
    GtkWidget *find_dialog = search_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(find_dialog);    
  }
}


void on_replace1_activate(GtkWidget *widget)
{
  if (document_manager_get_current_document(main_window.docmg)) {
    GtkWidget *replace_dialog = replace_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(replace_dialog);    
  }
}

void on_undo1_activate(GtkWidget *widget)
{
  document_undo(document_manager_get_current_document(main_window.docmg));
}


void on_redo1_activate(GtkWidget *widget)
{
  document_redo(document_manager_get_current_document(main_window.docmg));
}


void keyboard_macro_startstop(GtkWidget *widget)
{
  document_keyboard_macro_startstop(document_manager_get_current_document(main_window.docmg));
}

void keyboard_macro_playback(GtkWidget *widget)
{
  document_keyboard_macro_playback(document_manager_get_current_document(main_window.docmg));
}


void on_preferences1_activate(GtkWidget *widget)
{
  GtkWidget *preferences_dialog = preferences_dialog_new (GTK_WINDOW(main_window.window));
  gtk_widget_show(preferences_dialog);
}


void context_help(GtkWidget *widget)
{
  document_manager_get_context_help(main_window.docmg);
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
      _("Copyright \xc2\xa9 2003-2006 Andy Jeffries, 2009-2010 Anoop John"));
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
void update_status_combobox(Document *document)
{
      if (is_app_closing) return ;
      gint type = -1;
      if (document) g_object_get(document, "type", &type, NULL);
      /* set statuscombo */
      switch(type) {
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
  GtkWidget *child;
  child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  if(!document_manager_set_current_document_from_widget (main_window.docmg, child)){
    gphpedit_debug_message(DEBUG_MAIN_WINDOW,_("Unable to get data for page %d"), page_num);
  }
  if (!is_app_closing) {
    // Change the title of the main application window to the full filename
    update_app_title(document_manager_get_current_document(main_window.docmg));
    on_tab_change_update_classbrowser(main_window.notebook_editor);
  }
  check_externally_modified(document_manager_get_current_document(main_window.docmg));
}

gboolean on_notebook_focus_tab(GtkNotebook *notebook,
                 GtkNotebookTab arg1, gpointer user_data)
{
  document_grab_focus(document_manager_get_current_document(main_window.docmg));
  return TRUE;
}

void inc_search_typed (GtkEntry *entry, const gchar *text, gint length,
             gint *position, gpointer data)
{
  gchar *current_text;
  current_text = (gchar *)gtk_entry_get_text(entry);
  document_incremental_search(document_manager_get_current_document(main_window.docmg), current_text, FALSE);
}

gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data)
{
  //Auto focus editor tab only if it is a scintilla tab
    if (event->keyval == GDK_Escape) {
        document_grab_focus(document_manager_get_current_document(main_window.docmg));
      return TRUE;
    }

  return FALSE;
}

void add_to_search_history(const gchar *current_text){
    /* add text to search history*/
    GSList *walk;
    gint i=0;
    for (walk = get_preferences_manager_search_history(main_window.prefmg); walk!=NULL; walk = g_slist_next(walk)) {
      i++;
      if (g_strcmp0((gchar *) walk->data,current_text)==0){
        return;  /* already in the list */
        }
    }
    set_preferences_manager_new_search_history_item(main_window.prefmg, i, current_text);
    gphpedit_debug_message(DEBUG_MAIN_WINDOW,"added:%s",current_text);
    toolbar_completion_add_text(TOOLBAR(main_window.toolbar_find), current_text);
}


void inc_search_activate(GtkEntry *entry,gpointer user_data)
{
  gchar *current_text;

  //Inc search only if the current tab is not a help tab
  current_text = (gchar *)gtk_entry_get_text(entry);

  document_incremental_search(document_manager_get_current_document(main_window.docmg), current_text, TRUE);
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
  document_goto_line(document_manager_get_current_document(main_window.docmg),line);
}

void goto_line_activate(GtkEntry *entry,gpointer user_data)
{
  gchar *current_text;
  if (document_manager_get_current_document(main_window.docmg)) {
    current_text = (gchar *)gtk_entry_get_text(entry);
    goto_line(current_text);
  }
}


void block_indent(GtkWidget *widget)
{
  gint indentation_size;
  g_object_get(main_window.prefmg, "indentation_size", &indentation_size, NULL);
  document_block_indent(document_manager_get_current_document(main_window.docmg), indentation_size);
}


void block_unindent(GtkWidget *widget)
{
  gint indentation_size;
  g_object_get(main_window.prefmg, "indentation_size", &indentation_size, NULL);
  document_block_unindent(document_manager_get_current_document(main_window.docmg), indentation_size);
}

//zoom in
void zoom_in(GtkWidget *widget)
{
  document_zoom_in(document_manager_get_current_document(main_window.docmg));
  update_zoom_level();
}

//zoom out
void zoom_out(GtkWidget *widget)
{
  document_zoom_out(document_manager_get_current_document(main_window.docmg));
  update_zoom_level();
}

void zoom_100(GtkWidget *widget)
{
  document_zoom_restore(document_manager_get_current_document(main_window.docmg));
  update_zoom_level();
}

void syntax_check(GtkWidget *widget)
{
   gtk_syntax_check_window_run_check(GTK_SYNTAX_CHECK_WINDOW(main_window.win), document_manager_get_current_document(main_window.docmg));
}


void syntax_check_clear(GtkWidget *widget)
{
  gtk_widget_hide(GTK_WIDGET(main_window.win));
}


void classbrowser_show(void)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gint size;
  g_object_get(main_window.prefmg, "side_panel_size", &size, NULL);
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane), size);
  g_object_set(main_window.prefmg, "side_panel_hidden", FALSE, NULL);
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
}


void classbrowser_hide(void)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane), 0);
  g_object_set(main_window.prefmg, "side_panel_hidden", TRUE, NULL);
}

void classbrowser_show_hide(GtkWidget *widget)
{
  gboolean hidden;
  g_object_get(main_window.prefmg, "side_panel_hidden", &hidden, NULL);
  menubar_set_classbrowser_status(MENUBAR(main_window.menu), hidden);
  if (hidden)
    classbrowser_show();
  else
    classbrowser_hide();
}

void force_php(GtkWidget *widget)
{
    set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_PHP);
}

void force_css(GtkWidget *widget)
{
    set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_CSS);
}

void force_sql(GtkWidget *widget)
{
  set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_SQL);
}

void force_cxx(GtkWidget *widget)
{
  set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_CXX);
}

void force_perl(GtkWidget *widget)
{
    set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_PERL);
}

void force_cobol(GtkWidget *widget)
{
  set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_COBOL);
}
void force_python(GtkWidget *widget)
{
  set_document_to_type(document_manager_get_current_document(main_window.docmg), TAB_PYTHON);
}

//function to refresh treeview when the current tab changes 
//view is refreshed only if the parse only current file parameter is set
gint on_tab_change_update_classbrowser(GtkWidget *widget)
{
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.classbrowser));
  return FALSE;
}

gboolean main_window_activate_focus (GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  check_externally_modified(document_manager_get_current_document(main_window.docmg));
  return FALSE;
}
