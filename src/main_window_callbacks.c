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
#include "stdlib.h"
#include "main_window_callbacks.h"
#include "find_replace.h"
#include "preferences_dialog.h"
#include "tab.h"
#include "templates.h"
#include "folderbrowser.h"
#include "plugin.h"
#include "gvfs_utils.h"
#include <gdk/gdkkeysyms.h>

gboolean is_app_closing = FALSE;
gint classbrowser_hidden_position;

// session_save relies on the fact that all tabs can be closed without 
// prompting, they should already be saved.  Also, the title won't be set
// afterwards.
void session_save(void)
{
  GSList *walk;
  Editor *editor;
  Editor *current_focus_editor;
  GString *session_file;
  GString *session_file_contents;

  session_file = g_string_new(g_get_home_dir());
  session_file = g_string_append(session_file, "/.gphpedit/session");
  
  GFile *file=get_gfile_from_filename(session_file->str);
  g_string_free(session_file,TRUE);
  GError *error=NULL;


  if (preferences.save_session && (g_slist_length(editors) > 0)) {
    current_focus_editor = main_window.current_editor;
    session_file_contents=g_string_new(NULL);
    for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
      editor = walk->data;
      if (editor) {
        if (!editor->is_untitled) {
          if (editor == current_focus_editor) {
            session_file_contents = g_string_append(session_file_contents,"*");
          }
          if (GTK_IS_SCINTILLA(editor->scintilla)) {
            g_string_append_printf (session_file_contents,"%s\n",editor->filename->str);
          } else {
            if (editor->type==TAB_HELP){
              /* it's a help page */
            g_string_append_printf (session_file_contents,"phphelp:%s\n",editor->help_function);
            } else {
              /* it's a preview page */
              gchar *temp=editor->filename->str;
              temp+=9;
              g_string_append_printf (session_file_contents,"preview:%s\n",temp);
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
  GString *target;

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
        target = g_string_new(filename);
        tab_create_new(TAB_HELP, target);
        g_string_free(target, TRUE);
          
        if (focus_this_one && (main_window.current_editor)) {
          focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),main_window.current_editor->help_scrolled_window);
        }
      }else if (g_str_has_prefix(filename, "preview:")){
        filename += 8;
        target = g_string_new(filename);
        tab_create_new(TAB_PREVIEW, target);
        g_string_free(target, TRUE);
        if (focus_this_one && (main_window.current_editor)) {
          focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),main_window.current_editor->help_scrolled_window);
        }
      } else {
        switch_to_file_or_open(filename,0);
        if (focus_this_one && (main_window.current_editor)) {
          focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),main_window.current_editor->scintilla);
        }
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
        g_print(_("GIO Error deleting file: %s, code %d\n"),error->message,error->code);
      }
      g_error_free (error);
  }
  g_string_free(session_file,TRUE);
  g_object_unref (file);
}


/* Actual action functions */

void quit_application()
{
  preferences_save();
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:quit_application:Saved preferences\n"); }
  template_db_close();
  session_save();
  close_all_tabs();
}


void main_window_destroy_event(GtkWidget *widget, gpointer data)
{
  quit_application();
  g_slice_free(Mainmenu, main_window.menu); /* free menu struct*/
  g_slice_free(Maintoolbar, main_window.toolbar_main); /* free toolbar struct*/
  g_slice_free(Findtoolbar, main_window.toolbar_find); /* free toolbar struct*/
  cleanup_classbrowser();
  cleanup_calltip();
  cleanup_plugins();
  gtk_main_quit();
}


// This procedure relies on the fact that all tabs will be closed without prompting
// for whether they need saving beforehand.  If in doubt, call can_all_tabs_be_saved
// and pay attention to the return value.
void close_all_tabs(void)
{
  GSList *walk;
  Editor *editor;

  is_app_closing = TRUE;
  
  while (g_slist_length(editors)>0) {
    walk = editors;
    editor = walk->data;
    if (editor) {
      close_page(editor);
      editors = g_slist_remove(editors, editor);
    }
  }
  editors = NULL;
  main_window.current_editor=FALSE;
  is_app_closing = FALSE;
  return;
}


// Returns true if all tabs are either saved or closed
gboolean can_all_tabs_be_saved(void)
{
  GSList *walk;
  Editor *editor;
  gboolean saved;

  is_app_closing = TRUE;
  for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
    if (g_slist_length(editors) == 0) {
      break;
    }
    editor = walk->data;
    if (editor && (editor->scintilla || editor->help_scrolled_window)) {
      if (!editor->saved && editor->type!=TAB_HELP) {
        saved = try_save_page(editor, FALSE);
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


void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data) {
  main_window_size_save_details();
}

void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
  preferences.maximized = GDK_WINDOW_STATE_MAXIMIZED && event->new_window_state;
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
  save_classbrowser_position();
  return TRUE;
}

gint main_window_key_press_event(GtkWidget   *widget, GdkEventKey *event,gpointer user_data)
{
  guint current_pos;
  guint current_line;
  gint search_length;
  gchar *search_buffer;
  gchar *member_function_buffer;
  gint member_function_length;
  gint wordStart;
  gint wordEnd;
  
  if (main_window.notebook_editor != NULL) {
    if (main_window.current_editor && main_window.current_editor->scintilla) check_externally_modified();
    if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_ISO_Left_Tab)) {
      // Hack, for some reason when shift is held down keyval comes through as GDK_ISO_Left_Tab not GDK_Tab
      if (gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window.notebook_editor)) == 0) {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor),gtk_notebook_get_n_pages(GTK_NOTEBOOK(main_window.notebook_editor))-1);
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
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_i) || (event->keyval == GDK_I))) {
      if ((event->state & GDK_SHIFT_MASK)==GDK_SHIFT_MASK) {
        return FALSE;
      }
      if (main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
        wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
        wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
        if (wordStart != wordEnd && (wordEnd-wordStart)<=25) {
             search_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &search_length);
             gtk_entry_set_text(GTK_ENTRY(main_window.toolbar_find->search_entry), search_buffer);
        }
        gtk_widget_grab_focus(GTK_WIDGET(main_window.toolbar_find->search_entry));
      }
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_g) || (event->keyval == GDK_G))) {
      gtk_widget_grab_focus(GTK_WIDGET(main_window.toolbar_find->goto_entry));
      return TRUE;
    }
    else if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_space)) {
      current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
      show_call_tip(main_window.current_editor->scintilla,main_window.current_editor->type, current_pos);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_j) || (event->keyval == GDK_J)))  {
      template_find_and_insert();
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_F2)))  {
      ///add a marker
      //bugfix:segfault if type=TABHELP
      if (main_window.current_editor && main_window.current_editor->type != TAB_HELP && main_window.current_editor->type != TAB_PREVIEW) {
      current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
      current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
      mod_marker(current_line);
      }
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_0)))  {
      if (main_window.menu->plugins[0]) plugin_exec(0);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_1)))  {
      if (main_window.menu->plugins[1]) plugin_exec(1);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_2)))  {
      if (main_window.menu->plugins[2]) plugin_exec(2);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_3)))  {
      if (main_window.menu->plugins[3]) plugin_exec(3);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_4)))  {
      if (main_window.menu->plugins[4]) plugin_exec(4);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_5)))  {
      if (main_window.menu->plugins[5]) plugin_exec(5);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_6)))  {
      if (main_window.menu->plugins[6]) plugin_exec(6);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_7)))  {
      if (main_window.menu->plugins[7]) plugin_exec(7);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_8)))  {
      if (main_window.menu->plugins[8]) plugin_exec(8);
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_9)))  {
      if (main_window.menu->plugins[9]) plugin_exec(9);
      return TRUE;
    }
    else if ((event->keyval == GDK_F2))  {
      ///find next marker
      //bugfix:segfault if type=TABHELP
      if (main_window.current_editor && main_window.current_editor->type != TAB_HELP && main_window.current_editor->type != TAB_PREVIEW) {
      current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
      current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
      find_next_marker(current_line);
      }
      return TRUE;
    }
    else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_space) && 
         (main_window.current_editor->type != TAB_HELP) && (main_window.current_editor->type != TAB_PREVIEW)) {
      current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
      current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
      wordStart = gtk_scintilla_word_start_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);
      wordEnd = gtk_scintilla_word_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-1, TRUE);

      member_function_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos-2, current_pos, &member_function_length);
      if (gtk_scintilla_get_line_state(GTK_SCINTILLA(main_window.current_editor->scintilla), current_line)==274) {
        if (strcmp(member_function_buffer, "->")==0) {
          autocomplete_member_function(main_window.current_editor->scintilla, wordStart, wordEnd);
        }
        else if (main_window.current_editor->type == TAB_PHP) {
          autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
        }
        else if (main_window.current_editor->type == TAB_CSS) {
          css_autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
        }
        else if (main_window.current_editor->type == TAB_SQL) {
          sql_autocomplete_word(main_window.current_editor->scintilla, wordStart, wordEnd);
        }
      }
      g_free(member_function_buffer);
      return TRUE;
    }
  }

  return FALSE;
}

void on_new1_activate(GtkWidget *widget)
{
  // Create a new untitled tab
  tab_create_new(TAB_FILE, NULL);
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

void reopen_recent(GtkWidget *widget, gpointer data) {
  const gchar *filename;
  filename=gtk_menu_item_get_label ((GtkMenuItem *)widget);
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:reopen_recent:filename: %s\n", filename); }
  
  switch_to_file_or_open((gchar *)filename, 0);
}

void run_plugin(GtkWidget *widget, gpointer data) {
  plugin_exec((gulong) data);// was (gint)
}

void on_openselected1_activate(GtkWidget *widget)
{
  GSList *li;
  Editor *editor;

  gint wordStart;
  gint wordEnd;
  gchar *ac_buffer;
  gint ac_length;
  gchar *file;

  if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP || main_window.current_editor->type == TAB_PREVIEW) return;
  
  wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
  wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
  ac_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);
  if (wordStart!=wordEnd){
    for(li = editors; li!= NULL; li = g_slist_next(li)) {
      editor = li->data;
      if (editor) {
        if (editor->opened_from) {
          file = filename_parent_uri(editor->opened_from->str);
          if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate (opened_from) :file: %s\n", file); }
        }
        else {
          file = filename_parent_uri(editor->filename->str);
          if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate:file: %s\n", file); }
        }

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
  php_file_extensions = g_strsplit(preferences.php_file_extensions,",",-1);

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
  gchar *last_opened_folder;
  // Create the selector widget
  file_selection_box = gtk_file_chooser_dialog_new("Please select files for editing", GTK_WINDOW(main_window.window),
    GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
  gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);  
  //Add filters to the open dialog
  add_file_filters(GTK_FILE_CHOOSER(file_selection_box));
  GConfClient *config;
  GError *error = NULL;
  config=gconf_client_get_default ();
  last_opened_folder = gconf_client_get_string(config,"/gPHPEdit/general/last_opened_folder",&error);
  if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_open1_activate:last_opened_folder: %s\n", last_opened_folder); }
  /* opening of multiple files at once */
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_selection_box), TRUE);
  if (main_window.current_editor && !main_window.current_editor->is_untitled) {
    folder = filename_parent_uri(main_window.current_editor->filename->str);
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
  g_free(last_opened_folder);
  gtk_widget_destroy(file_selection_box);
}

void save_file_as_ok(GtkFileChooser *file_selection_box)
{
  GString *filename;
  gchar *uri=gtk_file_chooser_get_uri(file_selection_box);
  filename = g_string_new(uri);
  g_free(uri);
  // Set the filename of the current editor to be that
  if (main_window.current_editor->filename) {
    g_string_free(main_window.current_editor->filename, TRUE);
  }
  main_window.current_editor->filename=g_string_new(filename->str);
  main_window.current_editor->is_untitled=FALSE;
  main_window.current_editor->short_filename = filename_get_basename(filename->str);
  tab_check_php_file(main_window.current_editor);
  tab_check_css_file(main_window.current_editor);

  // Call Save method to actually save it now it has a filename
  on_save1_activate(NULL);

  if (main_window.current_editor->opened_from) {
    g_string_free(main_window.current_editor->opened_from, TRUE);
    main_window.current_editor->opened_from = NULL;
  }
  g_string_free(filename, FALSE);
}

void on_save1_activate(GtkWidget *widget)
{
  gchar *filename = NULL;
  GFile *file;
  if (main_window.current_editor) {
    filename = main_window.current_editor->filename->str;

    //if filename is Untitled
    if (main_window.current_editor->is_untitled) {
      on_save_as1_activate(widget);
    } else {
      file=get_gfile_from_filename(filename);
      tab_file_save_opened(main_window.current_editor,file);
    }
  }
}

void on_saveall1_activate(GtkWidget *widget)
{
  gchar *write_buffer = NULL;
  gsize text_length;
  gchar *filename;
  GSList *li;
  Editor *editor;
  gchar *converted_text = NULL;
  gsize utf8_size; // was guint
  GError *error = NULL;

  for(li = editors; li!= NULL; li = g_slist_next(li)) {
    editor = li->data;
    if (editor && editor->type!=TAB_HELP && editor->type!=TAB_PREVIEW && editor->is_untitled!=TRUE) {
      filename = editor->filename->str;
        GFile *file;
        file=get_gfile_from_filename(filename);
        text_length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
        write_buffer = g_malloc0(text_length+1); // Include terminating null
        if (write_buffer == NULL) {
          g_warning ("%s", _("Cannot allocate write buffer"));
          break;
        }
        gtk_scintilla_get_text(GTK_SCINTILLA(editor->scintilla), text_length+1, write_buffer);
        // If we converted to UTF-8 when loading, convert back to the locale to save
        if (editor->converted_to_utf8) {
          converted_text = g_locale_from_utf8(write_buffer, text_length, NULL, &utf8_size, &error);
          if (error != NULL) {
            g_print(_("UTF-8 Error: %s\n"), error->message);
            g_error_free(error);      
            } else {
            #ifdef DEBUGTAB
            g_print("DEBUG: Converted size: %d\n", utf8_size);
            #endif
            g_free(write_buffer);
            write_buffer = converted_text;
            text_length = utf8_size;
            g_free(converted_text);
            }
        }
        if (!g_file_replace_contents (file,write_buffer,text_length,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
          g_object_unref(file);  
          g_print(_("GIO Error: %s saving file:%s\n"),error->message,editor->filename->str);
          break;
        }
        g_object_unref(file);  
        g_get_current_time (&editor->file_mtime); /*set current time*/
        g_free (write_buffer);
        gtk_scintilla_set_save_point (GTK_SCINTILLA(editor->scintilla));
    }
  }
  classbrowser_update();
}


void on_save_as1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;
  gchar *filename;
  gchar *last_opened_folder;

  if (main_window.current_editor) {
    // Create the selector widget
    file_selection_box = gtk_file_chooser_dialog_new (_("Please type the filename to save as..."), 
      GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    
    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER(file_selection_box), TRUE);
    gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);
    
    if (main_window.current_editor) {
      filename = main_window.current_editor->filename->str;
      if (main_window.current_editor->is_untitled == FALSE) {
          gchar *uri=filename_get_uri(filename);
          gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(file_selection_box), uri);
          g_free(uri);
      }
      else {
        GConfClient *config;
        GError *error = NULL;
        config=gconf_client_get_default ();
        last_opened_folder = gconf_client_get_string(config,"/gPHPEdit/general/last_opened_folder",&error);
        if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:last_opened_folder: %s\n", last_opened_folder); }
        if (last_opened_folder){
          if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:Setting current_folder_uri to %s\n", last_opened_folder); }
          gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  last_opened_folder);
              g_free(last_opened_folder);
        }
      }
    }
    if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
      save_file_as_ok(GTK_FILE_CHOOSER(file_selection_box));
    }
    gtk_widget_destroy(file_selection_box);    
  }
}
void on_reload1_activate(GtkWidget *widget)
{
  if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP) return;
  if (main_window.current_editor && (main_window.current_editor->saved == FALSE)) {
    GtkWidget *file_revert_dialog;
    file_revert_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
            _("Are you sure you wish to reload the current file, losing your changes?"));
    gtk_window_set_title(GTK_WINDOW(file_revert_dialog), "Question");
    gint result = gtk_dialog_run (GTK_DIALOG (file_revert_dialog));
    if (result==GTK_RESPONSE_YES) {
      tab_load_file(main_window.current_editor);
    }
    gtk_widget_destroy(file_revert_dialog);
  }
  else if (main_window.current_editor) {
    tab_load_file(main_window.current_editor);
  }
}

void on_tab_close_activate(GtkWidget *widget, Editor *editor)
{
  try_close_page(editor);
  classbrowser_update();
}

void on_tab_close_set_style(GtkWidget *hbox, GtkWidget *button)
{
  gint w=0, h=0;
  gtk_icon_size_lookup_for_settings(gtk_widget_get_settings(hbox), GTK_ICON_SIZE_MENU, &w, &h);
  //debug("%d-%d", w, h);
  if (button){
   gtk_widget_set_size_request(button, w+2, h+2);
  }
}



void rename_file(GString *newfilename)
{
  GFile *file;
  GError *error;
  file=get_gfile_from_filename(main_window.current_editor->filename->str);
  gchar *basename=filename_get_basename(newfilename->str);
  file=g_file_set_display_name (file,basename,NULL,&error);
  g_free(basename);
  if (!file){
  g_print("GIO Error renaming file: %s\n",error->message);
  }
        
  // set current_editor->filename
  main_window.current_editor->filename=newfilename;
  main_window.current_editor->short_filename = filename_get_basename(newfilename->str);

  // save as new filename
  on_save1_activate(NULL);
}

void rename_file_ok(GtkFileChooser *file_selection)
{
  GString *filename;
  GtkWidget *file_exists_dialog;
  // Extract filename from the file chooser dialog
  gchar *fileuri=gtk_file_chooser_get_uri(file_selection);
  filename = g_string_new(fileuri);
  g_free(fileuri);
  GFile *file = get_gfile_from_filename(filename->str);
  if (g_file_query_exists (file,NULL)) {
    file_exists_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,
       _("This file already exists, are you sure you want to overwrite it?"));
    gtk_window_set_title(GTK_WINDOW(file_exists_dialog), _("gPHPEdit"));
    gint result = gtk_dialog_run (GTK_DIALOG (file_exists_dialog));
    if (result==GTK_RESPONSE_YES) {
      rename_file(filename);
    }
    gtk_widget_destroy(file_exists_dialog);
    } else {
      rename_file(filename);
    }
  g_object_unref(file);
  g_string_free(filename, TRUE);
}


void on_rename1_activate(GtkWidget *widget)
{
  GtkWidget *file_selection_box;

  if (main_window.current_editor) {
    // Create the selector widget
    file_selection_box = gtk_file_chooser_dialog_new(_("Please type the filename to rename this file to..."),
      GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
    
    if (main_window.current_editor) {
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER(file_selection_box), main_window.current_editor->filename->str);
    }

    if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
      rename_file_ok(GTK_FILE_CHOOSER(file_selection_box));
    }

    gtk_widget_destroy(file_selection_box);
  }
}


void set_active_tab(page_num)
{
  Editor *new_current_editor;

  new_current_editor = (Editor *)g_slist_nth_data(editors, page_num);

  if (new_current_editor) {
    //page_num = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),(new_current_editor->scintilla));
    gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  }

  main_window.current_editor = new_current_editor;

  update_app_title();

}
void update_zoom_level(void){
  if (main_window.current_editor == NULL) return;
    gint p=0;
    if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
      p= gtk_scintilla_get_zoom(GTK_SCINTILLA(main_window.current_editor->scintilla));
      p= (p*10) + 100;
    }else{
      if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
        gfloat d= webkit_web_view_get_zoom_level (main_window.current_editor->help_view);
        p=d*100;
      }
  }
  gchar *caption=g_strdup_printf("%s%d%s",_("Zoom:"),p,"%");
  gtk_label_set_text (GTK_LABEL(main_window.zoomlabel),caption);
  g_free(caption);
}

/**
 * Close a tab in the Editor. Removes the notebook page, frees the editor object,
 * and sets the active tab correcty
 * 
 * @param editor - The editor object corresponding to the tab that is going to be closed.
 * @return void
 */

void close_page(Editor *editor)
{
  gint page_num;
  gint page_num_closing;
  gint current_active_tab;

  if (GTK_IS_SCINTILLA(editor->scintilla)) {
    page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),editor->scintilla);
    //g_free(editor->short_filename);
  }
  else {
    page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),editor->help_scrolled_window);
  }
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
  g_string_free(editor->filename, TRUE);
  g_free(editor->short_filename);
  g_free(editor->contenttype);
  g_slice_free(Editor,editor);
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


gboolean try_save_page(Editor *editor, gboolean close_if_can)
{
  GtkWidget *confirm_dialog;
  gint ret;
  GString *string;
  string = g_string_new("");
  g_string_printf(string, _("The file '%s' has not been saved since your last changes, are you sure you want to close it and lose these changes?"), editor->short_filename);
  confirm_dialog=gtk_message_dialog_new (GTK_WINDOW(main_window.window),GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_WARNING,GTK_BUTTONS_NONE,_("The file '%s' has not been saved since your last changes, are you sure you want to close it and lose these changes?"),editor->short_filename);
  g_string_printf(string,_("Unsaved changes to '%s'"), editor->filename->str);
  gtk_window_set_title(GTK_WINDOW(confirm_dialog), string->str);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("Close and _lose changes"),0);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("_Save file"),1);
  gtk_dialog_add_button (GTK_DIALOG(confirm_dialog),_("_Cancel closing"),2);
  ret = gtk_dialog_run (GTK_DIALOG (confirm_dialog));
  gtk_widget_destroy(confirm_dialog);
  switch (ret) {
    case 0:
      if (close_if_can) {
          close_page(editor);
          editors = g_slist_remove(editors, editor);
          if (g_slist_length(editors) == 0) {
            editors = NULL;
            main_window.current_editor = NULL;
          }
      }
      return TRUE;
    case 1:
      on_save1_activate(NULL);
      // If chose neither of these, dialog either cancelled or closed.  Do nothing.
  }
  return FALSE;
}


gboolean try_close_page(Editor *editor)
{
  if (!editor->saved && editor->type!=TAB_HELP && !(editor->is_untitled && gtk_scintilla_get_text_length(GTK_SCINTILLA(editor->scintilla))==0)) {
    return try_save_page(editor, TRUE);
  }
  else {
    close_page(editor);
    editors = g_slist_remove(editors, editor);
    if (g_slist_length(editors) == 0) {
      editors = NULL;
      main_window.current_editor = NULL;
    }
    return TRUE;
  }
}


void on_close1_activate(GtkWidget *widget)
{
  GtkTreeIter iter;

  if (main_window.current_editor != NULL) {
    try_close_page(main_window.current_editor);
    classbrowser_update();
    update_app_title();
    update_zoom_level();
    if (!gtk_tree_selection_get_selected (main_window.classtreeselect, NULL, &iter)) {
      gtk_label_set_text(GTK_LABEL(main_window.treeviewlabel), _("FILE:"));
    }
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
  gint wordStart;
  gint wordEnd;
  gint length;
  gchar *buffer;
  
  if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP)
    return;
  
  wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
  wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
  if (wordStart != wordEnd) {
    buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
    gtk_clipboard_set_text(main_window.clipboard,buffer,length);
    gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), "");
    g_free(buffer);
  }
  //gtk_scintilla_cut(GTK_SCINTILLA(main_window.current_editor->scintilla));
}

void on_copy1_activate(GtkWidget *widget)
{
  gint wordStart;
  gint wordEnd;
  gint length;
  gchar *buffer;
  
  if (main_window.current_editor == NULL) return;
  
  if (main_window.current_editor->type == TAB_HELP) {
     webkit_web_view_copy_clipboard (WEBKIT_WEB_VIEW(main_window.current_editor->help_view));
  }
  else {
    wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
    wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
    if (wordStart != wordEnd) {
      buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
      gtk_clipboard_set_text(main_window.clipboard,buffer,length);
      g_free(buffer);
    }
  }
  macro_record (main_window.current_editor->scintilla, 2178, 0, 0); // As copy doesn't change the buffer it doesn't get recorded, so do it manually
  
  //gtk_scintilla_copy(GTK_SCINTILLA(main_window.current_editor->scintilla));
}


void on_paste_got_from_cliboard(GtkClipboard *clipboard, const gchar *text, gpointer data)
{
  Editor *editor;
  editor = editor_find_from_scintilla(data);
  editor->is_pasting = editor->is_macro_recording;
  gtk_scintilla_replace_sel(GTK_SCINTILLA(data), text);
  editor->is_pasting = FALSE;

  // Possible fix for rendering issues after pasting
  gtk_scintilla_colourise(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, -1);
}

void selectiontoupper(void){
  gint wordStart;
  gint wordEnd;
  gint length;
  gchar *buffer;

  if (main_window.current_editor == NULL)
    return;

  if (main_window.current_editor->type != TAB_HELP) {
    wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
    wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
    if (wordStart != wordEnd) {
      buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
      /* buffer to upper */
      gchar *tmpbuffer=g_utf8_strup (buffer,strlen(buffer));
      g_free(buffer);
      /* replace sel */
      gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), tmpbuffer);
      g_free(tmpbuffer);
    }
  }
}
void selectiontolower(void){
  gint wordStart;
  gint wordEnd;
  gint length;
  gchar *buffer;

  if (main_window.current_editor == NULL) return;

  if (main_window.current_editor->type != TAB_HELP) {
    wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
    wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
    if (wordStart != wordEnd) {
      buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
       /* buffer to lower */
      gchar *tmpbuffer=g_utf8_strdown (buffer,strlen(buffer));
      g_free(buffer);
      /* replace sel */
      gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), tmpbuffer);
      g_free(tmpbuffer);
    }
  }
}
void on_paste1_activate(GtkWidget *widget)
{
  if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP || main_window.current_editor->type == TAB_PREVIEW)
    return;
  
  gtk_clipboard_request_text(main_window.clipboard, on_paste_got_from_cliboard,main_window.current_editor->scintilla);
}


void on_selectall1_activate(GtkWidget *widget)
{
  if (main_window.current_editor == NULL)
    return;
  if (main_window.current_editor->type == TAB_HELP){
  //select text in help tab
    webkit_web_view_select_all (WEBKIT_WEB_VIEW(main_window.current_editor->help_view));
  } else {
    gtk_scintilla_select_all(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }
}


void on_find1_activate(GtkWidget *widget)
{
  if (main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
    if (find_dialog.window1==NULL) {
      find_create();
    }
    gtk_widget_show(find_dialog.window1);
  }
}


void on_replace1_activate(GtkWidget *widget)
{
  if (main_window.current_editor) {
    if (replace_dialog.window2 == NULL) {
      replace_create();
    }
    gtk_widget_show(replace_dialog.window2);
  }
}

void on_undo1_activate(GtkWidget *widget)
{
  if (main_window.current_editor) {
    gtk_scintilla_undo(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }
}


void on_redo1_activate(GtkWidget *widget)
{
  if (main_window.current_editor) {
    gtk_scintilla_redo(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }
}


void keyboard_macro_startstop(GtkWidget *widget)
{
  if (main_window.current_editor) {
    if (main_window.current_editor->is_macro_recording) {
      gtk_scintilla_stop_record(GTK_SCINTILLA(main_window.current_editor->scintilla));
      main_window.current_editor->is_macro_recording = FALSE;
    }
    else {
      if (main_window.current_editor->keyboard_macro_list) {
        keyboard_macro_empty_old(main_window.current_editor);
      }
      gtk_scintilla_start_record(GTK_SCINTILLA(main_window.current_editor->scintilla));
      main_window.current_editor->is_macro_recording = TRUE;
    }
  }
}

void keyboard_macro_playback(GtkWidget *widget)
{
  GSList *current;
  MacroEvent *event;

  if (main_window.current_editor) {
    gtk_scintilla_begin_undo_action(GTK_SCINTILLA(main_window.current_editor->scintilla));
    if (main_window.current_editor->keyboard_macro_list) {
      for (current = main_window.current_editor->keyboard_macro_list; current; current = g_slist_next(current)) {
        event = current->data;
        if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:keyboard_macro_playback:Message: %d (%s)\n", event->message, macro_message_to_string(event->message)); }

        switch (event->message) {
          case (2170) : gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), (gchar *)event->lparam); break;
          case (2177) : gtk_scintilla_cut(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2178) : gtk_scintilla_copy(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2179) : gtk_scintilla_paste(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2180) : gtk_scintilla_clear(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2300) : gtk_scintilla_line_down(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2301) : gtk_scintilla_line_down_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2302) : gtk_scintilla_line_up(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2303) : gtk_scintilla_line_up_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2304) : gtk_scintilla_char_left(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2305) : gtk_scintilla_char_left_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2306) : gtk_scintilla_char_right(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2307) : gtk_scintilla_char_right_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2308) : gtk_scintilla_word_left(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2309) : gtk_scintilla_word_left_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2310) : gtk_scintilla_word_right(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2311) : gtk_scintilla_word_right_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2312) : gtk_scintilla_home(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2313) : gtk_scintilla_home_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2314) : gtk_scintilla_line_end(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2315) : gtk_scintilla_line_end_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2316) : gtk_scintilla_document_start(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2317) : gtk_scintilla_document_start_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2318) : gtk_scintilla_document_end(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2319) : gtk_scintilla_document_end_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2320) : gtk_scintilla_page_up(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2321) : gtk_scintilla_page_up_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2322) : gtk_scintilla_page_down(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2323) : gtk_scintilla_page_down_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2324) : gtk_scintilla_edit_toggle_overtype(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2325) : gtk_scintilla_cancel(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2326) : gtk_scintilla_delete_back(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2327) : gtk_scintilla_tab(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2328) : gtk_scintilla_back_tab(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2329) : gtk_scintilla_new_line(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2330) : gtk_scintilla_form_feed(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2331) : gtk_scintilla_v_c_home(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2332) : gtk_scintilla_v_c_home_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2333) : gtk_scintilla_zoom_in(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2334) : gtk_scintilla_zoom_out(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2335) : gtk_scintilla_del_word_left(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2336) : gtk_scintilla_del_word_right(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2337) : gtk_scintilla_line_cut(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2338) : gtk_scintilla_line_delete(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2339) : gtk_scintilla_line_transpose(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2340) : gtk_scintilla_lower_case(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2341) : gtk_scintilla_upper_case(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2342) : gtk_scintilla_line_scroll_down(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2343) : gtk_scintilla_line_scroll_up(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2344) : gtk_scintilla_delete_back_not_line(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2345) : gtk_scintilla_home_display(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2346) : gtk_scintilla_home_display_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2347) : gtk_scintilla_line_end_display(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          case (2348) : gtk_scintilla_line_end_display_extend(GTK_SCINTILLA(main_window.current_editor->scintilla)); break;
          default:
            g_print(_("Unhandle keyboard macro function %d, please report\n"), event->message);
        }
      }
    }
    gtk_scintilla_end_undo_action(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }
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
  gint length;
  GString *function;
  gint wordStart;
  gint wordEnd;
  
  if (main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
    wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
    wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
    if (wordStart != wordEnd) {
      buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
      //g_print("%s\n", buffer);
      function = g_string_new(buffer);
      tab_create_new(TAB_HELP, function);
      g_string_free(function, TRUE);
    }
  }

  g_free(buffer);
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
     http://library.gnome.org/devel/gtk/unstable/GtkWindow.html#gtk-window-set-transient-for
     Dialog windows should be set transient for the main application window they were spawned from. 
     This allows window managers  to e.g. keep the dialog on top of the main window, or center the dialog over the main window.
  */
  gtk_window_set_transient_for (GTK_WINDOW(dialog),GTK_WINDOW(main_window.window));
  gtk_dialog_run(GTK_DIALOG (dialog));
  gtk_widget_destroy(dialog);
}

void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page,
                gint page_num, gpointer user_data)
{
  Editor *data;
  GtkWidget *child;

  child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
  if (GTK_IS_SCINTILLA(child)){
    data = editor_find_from_scintilla(child);
    if (GTK_IS_SCINTILLA(data->scintilla)) {
      // Grab the focus in to the editor
      gtk_scintilla_grab_focus(GTK_SCINTILLA(data->scintilla));
    }
  } else {
    data = editor_find_from_help((void *)child);
  }
  if (data){
    // Store it in the global main_window.current_editor value
    main_window.current_editor = data;
  } else {
    g_print(_("Unable to get data for page %d\n"), page_num);
  }
  if (!is_app_closing) {
    // Change the title of the main application window to the full filename
    update_app_title();
  }
  on_tab_change_update_classbrowser(main_window.notebook_editor);
  check_externally_modified();
}

gboolean on_notebook_focus_tab(GtkNotebook *notebook,
                 GtkNotebookTab arg1, gpointer user_data)
{
  gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
  return TRUE;
}

void inc_search_typed (GtkEntry *entry, const gchar *text, gint length,
             gint *position, gpointer data)
{
  gint found_pos;
  glong text_min, text_max;
  gchar *current_text;

  //Inc search only if the current tab is not a help tab
  if (main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
    current_text = (gchar *)gtk_entry_get_text(entry);

    found_pos = gtk_scintilla_find_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, current_text, 0, gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla)), &text_min, &text_max);
    if (found_pos != -1) {
      gtk_scintilla_set_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), text_min, text_max);
    }
  }
}


gboolean inc_search_key_release_event(GtkWidget *widget,GdkEventKey *event,gpointer user_data)
{
  //Auto focus editor tab only if it is a scintilla tab
  if(main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
    if (event->keyval == GDK_Escape) {
      gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
      return TRUE;
    }
  }
  return FALSE;
}
void add_to_search_history(const gchar *current_text){
    /* add text to search history*/
    GSList *walk;
    gint i=0;
    for (walk = preferences.search_history; walk!=NULL; walk = g_slist_next(walk)) {
      i++;
      if (strcmp((gchar *) walk->data,current_text)==0){
        return;  /* already in the list */
        }
    }
    preferences.search_history = g_slist_prepend (preferences.search_history, g_strdup(current_text));
    if (i==16){
    /* delete last item */
    GSList *temp= g_slist_nth (preferences.search_history,16);
    preferences.search_history = g_slist_remove (preferences.search_history, temp->data);
    }
    #ifdef DEBUG
    g_print("added:%s\n",current_text);
    #endif
    gtk_entry_completion_insert_action_text (main_window.toolbar_find->completion,0,g_strdup(current_text));    
}

void inc_search_activate(GtkEntry *entry,gpointer user_data)
{
  gint current_pos;
  gint found_pos;
  glong text_min, text_max;
  gchar *current_text;

  //Inc search only if the current tab is not a help tab
  if (main_window.current_editor && main_window.current_editor->type != TAB_HELP) {
    current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
    current_text = (gchar *)gtk_entry_get_text(entry);

    found_pos = gtk_scintilla_find_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, current_text, current_pos+1, gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla)), &text_min, &text_max);
    if (found_pos != -1) {
      gtk_scintilla_set_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), text_min, text_max);
    }
    else {
      found_pos = gtk_scintilla_find_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, current_text, 0, gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla)), &text_min, &text_max);
      if (found_pos != -1) {
        gtk_scintilla_set_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), text_min, text_max);
      }
    }
    add_to_search_history(current_text);
  }
}


gboolean is_valid_digits_only(gchar *text)
{
  while (*text) {
    if (*text < 48 || *text > 57) {
      return FALSE;
    }
  }

  return TRUE;
}

void goto_line_int(gint line)
{
  //gint current_pos;
  //bugfix: seg fault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
  gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
  gtk_scintilla_goto_line(GTK_SCINTILLA(main_window.current_editor->scintilla), line + gtk_scintilla_lines_on_screen(GTK_SCINTILLA(main_window.current_editor->scintilla))); //move some lines to center in screen
  gtk_scintilla_goto_line(GTK_SCINTILLA(main_window.current_editor->scintilla), line-1); // seems to be off by one...
  }
}


void goto_line(gchar *text)
{
  gint line;
  line = atoi(text);
  goto_line_int(line);
}

void goto_line_activate(GtkEntry *entry,gpointer user_data)
{
  gchar *current_text;

  if (main_window.current_editor) {
    current_text = (gchar *)gtk_entry_get_text(entry);
    goto_line(current_text);
  }
}


void move_block(gint indentation_size)
{
  gint startline;
  gint endline;
  gint line;
  gint indent;

  if (main_window.current_editor) {
    gtk_scintilla_begin_undo_action(GTK_SCINTILLA(main_window.current_editor->scintilla));

    startline = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla)));
    endline = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla)));

    for (line = startline; line <= endline; line++) {
      indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(main_window.current_editor->scintilla), line);
      gtk_scintilla_set_line_indentation(GTK_SCINTILLA(main_window.current_editor->scintilla), line, indent+indentation_size);
    }
    gtk_scintilla_end_undo_action(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }
}


void block_indent(GtkWidget *widget)
{
//bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){ 
    move_block(preferences.indentation_size);
  }
}


void block_unindent(GtkWidget *widget)
{
//bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){ 
    move_block(0-preferences.indentation_size);
  }
}

//zoom in
void zoom_in(GtkWidget *widget)
{
//bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){ 
    gtk_scintilla_zoom_in(GTK_SCINTILLA(main_window.current_editor->scintilla));
  }else{
    if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
      webkit_web_view_zoom_in (main_window.current_editor->help_view);
    }
  }
  update_zoom_level();
}

//zoom out
void zoom_out(GtkWidget *widget)
{
//bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    gtk_scintilla_zoom_out(GTK_SCINTILLA(main_window.current_editor->scintilla));
  } else {
    if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
      webkit_web_view_zoom_out (main_window.current_editor->help_view);
    }
  }
  update_zoom_level();
}

void zoom_100(GtkWidget *widget)
{
//bugfix:segfault if not scintilla
if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
  gtk_scintilla_set_zoom(GTK_SCINTILLA(main_window.current_editor->scintilla), 0);
} else {
  if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
    webkit_web_view_set_zoom_level (main_window.current_editor->help_view, 1);
  }
}
update_zoom_level();
}

//add marker
void add_marker(int line)
{
gtk_scintilla_marker_define(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, SC_MARK_SHORTARROW);
gtk_scintilla_marker_set_back(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, 101);
gtk_scintilla_marker_set_fore(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, 101);
gtk_scintilla_marker_add(GTK_SCINTILLA(main_window.current_editor->scintilla), line, 1);
}

//delete marker
void delete_marker(int line)
{
gtk_scintilla_marker_delete(GTK_SCINTILLA(main_window.current_editor->scintilla), line, 1);
}

//add/delete a marker
void mod_marker(int line){
//bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    if (gtk_scintilla_marker_get(GTK_SCINTILLA(main_window.current_editor->scintilla),line)!=2){
      add_marker(line);
    }else{
      delete_marker(line);
    }
  }
}
//circle markers
void find_next_marker(line_start){
  //bugfix:segfault if not scintilla
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    gint line;
    //skip the current line
    line= gtk_scintilla_marker_next(GTK_SCINTILLA(main_window.current_editor->scintilla),line_start + 1, 2);
    if (line==-1){
      //no markers in that direccion, we should go back to the first line
      line= gtk_scintilla_marker_next(GTK_SCINTILLA(main_window.current_editor->scintilla),0, 2);
      if (line!=-1){
        //go back to the first marker
        //bugfix the maker is in the next line
        goto_line_int(line+1);
      }else{  
        g_print("No marker found\n");
      }
    }else{
      //goto the marker posicion
      goto_line_int(line+1);
    }
  }
}

void syntax_check(GtkWidget *widget)
{
  if (main_window.current_editor) {
    if (editor_is_local(main_window.current_editor)) {
      gtk_widget_show(main_window.scrolledwindow1);
      gtk_widget_show(main_window.lint_view);
      syntax_check_run();
    }
  }
}


void syntax_check_clear(GtkWidget *widget)
{
  gtk_widget_hide(main_window.scrolledwindow1);
  gtk_widget_hide(main_window.lint_view);
}

void pressed_button_file_chooser(GtkButton *widget, gpointer data) {

  GtkWidget *pFileSelection;

  pFileSelection = gtk_file_chooser_dialog_new("Open...", GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
  gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(pFileSelection), FALSE);
  // sets the label folder as start folder 
  gchar *lbl;
  lbl=(gchar*)gtk_button_get_label(GTK_BUTTON(main_window.button_dialog));
  if (!IS_DEFAULT_DIR(lbl)){
     gboolean res;
     res=gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(pFileSelection), lbl);
  }
  gchar *sChemin=NULL;

  switch(gtk_dialog_run(GTK_DIALOG(pFileSelection))) {
         case GTK_RESPONSE_OK:
             sChemin = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(pFileSelection));
             break;
         default:
             break;
  }
  gtk_widget_destroy(pFileSelection);
  if(sChemin){
    gtk_button_set_label(GTK_BUTTON(widget), sChemin);
    /*store folder in config*/
    GConfClient *config;
    config=gconf_client_get_default ();
    gconf_client_set_string (config,"/gPHPEdit/main_window/folderbrowser/folder", sChemin,NULL);
    GtkTreeIter iter2;
    GtkTreeIter* iter=NULL;
    init_folderbrowser(main_window.pTree,sChemin,iter,&iter2);
    g_free(sChemin);
   }
}

void classbrowser_show(void)
{
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),classbrowser_hidden_position);
  //g_print("Width of class browser is %d\n", classbrowser_hidden_position);
  GConfClient *config;
  config=gconf_client_get_default ();
       
  gconf_client_set_int (config, "/gPHPEdit/main_window/classbrowser_hidden", 0,NULL);
  classbrowser_update();
}


void classbrowser_hide(void)
{
  GConfClient *config;
  config=gconf_client_get_default ();
  GError *error = NULL;
  classbrowser_hidden_position = gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_size",&error);
  if (classbrowser_hidden_position==0 && error!=NULL){
    classbrowser_hidden_position=100;
  }
  //g_print("Width of class browser is %d\n", classbrowser_hidden_position);
  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),0);
  gconf_client_set_int (config, "/gPHPEdit/main_window/classbrowser_hidden", 1,NULL);
}

void classbrowser_show_hide(GtkWidget *widget)
{
  gint hidden;
  GConfClient *config;
  config=gconf_client_get_default ();
  hidden = gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_hidden",NULL);
  gtk_check_menu_item_set_active ((GtkCheckMenuItem *) main_window.menu->tog_class,hidden);
  if (hidden == 1)
    classbrowser_show();
  else
    classbrowser_hide();
}

void lint_row_activated (GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *line;
  gchar *space;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, 0, &line, -1);

    // Get the line
    space = strrchr(line, ' ');
    space++;
    // Go to that line
      goto_line(space);

    g_free (line);
  }
}


gint treeview_double_click(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
  GtkTreeIter iter;
  gchar *filename = NULL;
  guint line_number;

  if (event->type==GDK_2BUTTON_PRESS ||
      event->type==GDK_3BUTTON_PRESS) {
      if (gtk_tree_selection_get_selected (main_window.classtreeselect, NULL, &iter)) {
        gtk_tree_model_get (GTK_TREE_MODEL(main_window.classtreestore), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
        if (filename) {
          switch_to_file_or_open(filename, line_number);
          g_free (filename);
        }
      }
  }
  return FALSE;
}


gint treeview_click_release(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
  GtkTreeIter iter;
  gchar *filename = NULL;
  guint line_number;

  if (gtk_tree_selection_get_selected (main_window.classtreeselect, NULL, &iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL(main_window.classtreestore), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
    if (filename) {
      classbrowser_update_selected_label(filename, line_number);
      g_free (filename);
    }
  }
  if (main_window.current_editor) {
    if(GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
    gtk_scintilla_scroll_caret(GTK_SCINTILLA(main_window.current_editor->scintilla));
    gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
    }
  }
  
  return FALSE;
}

void force_php(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_php(main_window.current_editor);
  }
}

void force_css(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_css(main_window.current_editor);
  }
}

void force_sql(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_sql(main_window.current_editor);
  }
}

void force_cxx(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_cxx(main_window.current_editor);
  }
}

void force_perl(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_perl(main_window.current_editor);
  }
}

void force_cobol(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_cobol(main_window.current_editor);
  }
}
void force_python(GtkWidget *widget)
{
  if (main_window.current_editor) {
    set_editor_to_python(main_window.current_editor);
  }
}

//function to refresh treeview when the parse only current file checkbox is clicked
//or when the checkbox is clicked and the files tabbar is clicked
gint on_parse_current_click (GtkWidget *widget)
{
  GConfClient *config;
  config=gconf_client_get_default ();
  gconf_client_set_int (config,"/gPHPEdit/classbrowser/onlycurrentfile", gtk_toggle_button_get_active((GtkToggleButton *)widget),NULL);
  
  classbrowser_update();
  return 0;
}
//function to refresh treeview when the current tab changes 
//view is refreshed only if the parse only current file parameter is set
gint on_tab_change_update_classbrowser(GtkWidget *widget)
{
  //debug("Toggled");
  //if parse only current file is set then add only the file in the current tab
  //the filteration logic is set inside classbrowser_update
  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (main_window.chkOnlyCurFileFuncs)))
  {
    //debug("Is set");
    classbrowser_update();
  }
  return 0;
}

void process_external (GtkInfoBar *info_bar, gint response_id, Editor *editor){
  if (response_id==1) tab_load_file(main_window.current_editor);
  else g_get_current_time (&main_window.current_editor->file_mtime); /*set current time*/
  gtk_widget_hide (GTK_WIDGET(info_bar));  
}

void check_externally_modified(void){
  if (!main_window.current_editor) return;
  if (!main_window.current_editor->is_untitled && GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    /* verify if file has been externally modified */
    if (get_file_modified(main_window.current_editor->filename->str, &main_window.current_editor->file_mtime, FALSE)){
      gchar *message=g_strdup_printf(_("The file %s has been externally modified. Do you want reload it?"), main_window.current_editor->filename->str);
      gtk_label_set_text (GTK_LABEL (main_window.infolabel), message);
      g_free(message);
      gtk_widget_show (main_window.infobar);
      return;
    }
  }
  gtk_widget_hide (main_window.infobar);
}

gboolean main_window_activate_focus (GtkWidget *widget,GdkEventFocus *event,gpointer user_data){
  check_externally_modified();
  return FALSE;
}
