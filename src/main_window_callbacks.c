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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
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
#include "classbrowser.h"
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
  preferences_manager_save_data(main_window.prefmg);
  g_object_unref(main_window.prefmg);
  g_object_unref(main_window.stylemg);
  g_object_unref(main_window.symbolmg);
}


void main_window_destroy_event(GtkWidget *widget, gpointer data)
{
  quit_application();
  gtk_main_quit();
}

gboolean main_window_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  gboolean cancel_quit = FALSE;
  is_app_closing = TRUE;
  cancel_quit = !document_manager_can_all_tabs_be_saved(main_window.docmg);
  is_app_closing = FALSE;
  return cancel_quit;
}

gint main_window_key_press_event(GtkWidget   *widget, GdkEventKey *event,gpointer user_data)
{

  if (main_window.notebook_editor != NULL) {
    documentable_check_externally_modified(document_manager_get_current_documentable(main_window.docmg));
    if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_KEY_ISO_Left_Tab)) {
      // Hack, for some reason when shift is held down keyval comes through as GDK_ISO_Left_Tab not GDK_Tab
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor)) == 0) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), gtk_notebook_get_n_pages(GTK_NOTEBOOK(main_window.notebook_editor))-1);
      }
      else {
        gtk_notebook_prev_page(GTK_NOTEBOOK(main_window.notebook_editor));
      }
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_KEY_Tab)) {
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor)) == gtk_notebook_get_n_pages(GTK_NOTEBOOK(main_window.notebook_editor))-1) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor),0);
      }
      else {
        gtk_notebook_next_page(GTK_NOTEBOOK(main_window.notebook_editor));
      }
      return TRUE;
    }
    else if ((event->state & GDK_MOD1_MASK)==GDK_MOD1_MASK && ((event->keyval >= GDK_KEY_0) && (event->keyval <= GDK_KEY_9))) {
      gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor),event->keyval - ((event->keyval == GDK_KEY_0) ? (GDK_KEY_0 - 9) : (GDK_KEY_0 + 1)));
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

static void add_file_filters(GtkFileChooser *chooser)
{
  //store file filter
  GtkFileFilter *filter;
  //creates a new file filter
  filter = gtk_file_filter_new ();
  GString *caption;
  GString *ext_pattern = NULL;

  caption = g_string_new(_("PHP files ("));
  gchar **php_file_extensions;
  gint i;
  gchar *php_extensions;
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
  g_free(php_extensions);
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
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  gchar *filename = documentable_get_filename(document);
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
  g_free(filename);
  gtk_widget_destroy(file_selection_box);
}

void save_file_as_ok(GtkFileChooser *file_selection_box)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  documentable_save_as(document, gtk_file_chooser_get_file(file_selection_box));
}

void on_save1_activate(GtkWidget *widget)
{
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
  if (document) {
    gboolean untitled;
    g_object_get(document, "untitled", &untitled, NULL);
    //if document is Untitled
    if (untitled) {
      on_save_as1_activate(widget);
    } else {
      documentable_save(document);
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
  const gchar *last_opened_folder;
  Documentable *document = document_manager_get_current_documentable(main_window.docmg);
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
    GFile *file;
    g_object_get(document, "GFile", &file, NULL);
    gtk_file_chooser_set_file (GTK_FILE_CHOOSER(file_selection_box), file, NULL);
  } else {
    g_object_get (main_window.prefmg, "last_opened_folder", &last_opened_folder, NULL);
    gphpedit_debug_message(DEBUG_MAIN_WINDOW, "Setting current_folder_uri to %s", last_opened_folder);
    gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box), last_opened_folder);
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER(file_selection_box), _("Untitled"));
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

void update_zoom_level(Documentable *doc)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  guint zoom_level = 100;
  if (doc) g_object_get(doc, "zoom_level", &zoom_level, NULL);
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

  if(document_manager_set_current_document_from_position(main_window.docmg, page_num)) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  }
  gtk_notebook_remove_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num_closing);
}

void on_close1_activate(GtkWidget *widget)
{
  document_manager_try_close_current_document(main_window.docmg);
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
  documentable_cut(document_manager_get_current_documentable(main_window.docmg));
}

void on_copy1_activate(GtkWidget *widget)
{
  documentable_copy(document_manager_get_current_documentable(main_window.docmg));
}


void selectiontoupper(void){
 documentable_selection_to_upper(document_manager_get_current_documentable(main_window.docmg));
}

void selectiontolower(void){
 documentable_selection_to_lower(document_manager_get_current_documentable(main_window.docmg));
}

void on_paste1_activate(GtkWidget *widget)
{
  documentable_paste(document_manager_get_current_documentable(main_window.docmg));
}


void on_selectall1_activate(GtkWidget *widget)
{
  documentable_select_all(document_manager_get_current_documentable(main_window.docmg));
}


void on_find1_activate(GtkWidget *widget)
{
  if (document_manager_get_current_documentable(main_window.docmg)) {
    GtkWidget *find_dialog = search_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(find_dialog);    
  }
}


void on_replace1_activate(GtkWidget *widget)
{
  if (document_manager_get_current_documentable(main_window.docmg)) {
    GtkWidget *replace_dialog = replace_dialog_new (GTK_WINDOW(main_window.window));
    gtk_widget_show(replace_dialog);    
  }
}

void on_undo1_activate(GtkWidget *widget)
{
  documentable_undo(document_manager_get_current_documentable(main_window.docmg));
}


void on_redo1_activate(GtkWidget *widget)
{
  documentable_redo(document_manager_get_current_documentable(main_window.docmg));
}


void keyboard_macro_startstop(GtkWidget *widget)
{
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);
  if(OBJECT_IS_DOCUMENT_SCINTILLA(doc)) {
  document_scintilla_keyboard_macro_startstop(DOCUMENT_SCINTILLA(doc));
  }
}

void keyboard_macro_playback(GtkWidget *widget)
{
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);
  if(OBJECT_IS_DOCUMENT_SCINTILLA(doc)) {
  document_scintilla_keyboard_macro_playback(DOCUMENT_SCINTILLA(doc));
  }
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
                "Jose Rostagno <joserostagno@vijona.com.ar>",
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
  GtkWidget *dialog = gtk_about_dialog_new();
  GtkAboutDialog *about = GTK_ABOUT_DIALOG(dialog);
  gtk_about_dialog_set_program_name(about, PACKAGE_NAME);
  gtk_about_dialog_set_version(about, VERSION);
  gtk_about_dialog_set_copyright(about,
      _("Copyright \xc2\xa9 2003-2006 Andy Jeffries, 2009-2010 Anoop John"));
  gtk_about_dialog_set_comments(about,
     _("gPHPEdit is a GNOME2 editor specialised for editing PHP "
               "scripts and related files (HTML/CSS/JS)."));
  #ifdef PACKAGE_URL
    gtk_about_dialog_set_website(about,PACKAGE_URL);
  #endif

  gtk_about_dialog_set_logo_icon_name (about, "gphpedit");
  gtk_about_dialog_set_authors(about,(const gchar **) authors);
  gtk_about_dialog_set_translator_credits(about,translator_credits);
  gtk_about_dialog_set_documenters (about,(const gchar **) documenters);
  /* 
     http://library.gnome.org/devel/gtk/stable/GtkWindow.html#gtk-window-set-transient-for
     Dialog windows should be set transient for the main application window they were spawned from. 
     This allows window managers  to e.g. keep the dialog on top of the main window, or center the dialog over the main window.
  */
  gtk_window_set_transient_for (GTK_WINDOW(dialog), GTK_WINDOW(main_window.window));
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}

void update_status_combobox(Documentable *document)
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

void on_notebook_switch_page (GtkNotebook *notebook, GtkWidget *page,
                gint page_num, gpointer user_data)
{
  if(!document_manager_set_current_document_from_widget (main_window.docmg, page)) {
    gphpedit_debug_message(DEBUG_MAIN_WINDOW,_("Unable to get data for page %d"), page_num);
  }
}

gboolean on_notebook_focus_tab(GtkNotebook *notebook,
                 GtkNotebookTab arg1, gpointer user_data)
{
  GtkWidget *document_widget;
  Documentable *doc = document_manager_get_current_documentable(main_window.docmg);
  g_object_get(doc, "editor_widget", &document_widget, NULL);
  gtk_widget_grab_focus(document_widget);
  return TRUE;
}

void block_indent(GtkWidget *widget)
{
  documentable_block_indent(document_manager_get_current_documentable(main_window.docmg));
}

void block_unindent(GtkWidget *widget)
{
  documentable_block_unindent(document_manager_get_current_documentable(main_window.docmg));
}

void zoom_in(GtkWidget *widget)
{
  documentable_zoom_in(document_manager_get_current_documentable(main_window.docmg));
}

void zoom_out(GtkWidget *widget)
{
  documentable_zoom_out(document_manager_get_current_documentable(main_window.docmg));
}

void zoom_100(GtkWidget *widget)
{
  documentable_zoom_restore(document_manager_get_current_documentable(main_window.docmg));
}

void force_php(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_PHP);
}

void force_css(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_CSS);
}

void force_sql(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_SQL);
}

void force_cxx(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_CXX);
}

void force_perl(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_PERL);
}

void force_cobol(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_COBOL);
}
void force_python(GtkWidget *widget)
{
  documentable_set_type(document_manager_get_current_documentable(main_window.docmg), TAB_PYTHON);
}

gboolean main_window_activate_focus (GtkWidget *widget,GdkEventFocus *event, gpointer user_data)
{
  documentable_check_externally_modified(document_manager_get_current_documentable(main_window.docmg));
  return FALSE;
}

void document_manager_close_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  close_page(DOCUMENT(doc));
  update_app_title(document_manager_get_current_documentable(docmg));
  gchar *filename = documentable_get_filename (doc);
  gint ftype;
  g_object_get(doc, "type", &ftype, NULL);
  symbol_manager_purge_file (main_window.symbolmg, filename, ftype);
  g_free(filename);
}

void document_manager_new_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  gint ftype;
  g_object_get(doc, "type", &ftype, NULL);
  gchar *filename = documentable_get_filename(doc);
  symbol_manager_add_file (main_window.symbolmg, filename, ftype);
  g_free(filename);
}

void document_manager_change_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  if (!is_app_closing) {
    update_app_title(doc);
    documentable_check_externally_modified(doc);
  }
}

void document_manager_zoom_change_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  update_zoom_level(doc);
}
