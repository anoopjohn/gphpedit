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
#include <libgnomevfs/gnome-vfs.h>

#include "main_window_callbacks.h"
#include "find_replace.h"
#include "main_window.h"
#include "preferences_dialog.h"
#include "tab.h"
#include "templates.h"


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
	FILE *fp;

	session_file = g_string_new( g_get_home_dir());
	session_file = g_string_append(session_file, "/.gphpedit/session");
	
	unlink(session_file->str);
	
	if (preferences.save_session && (g_slist_length(editors) > 0)) {
		current_focus_editor = main_window.current_editor;
	
		fp = fopen(session_file->str, "w");
  		if (!fp) {	
			g_print(_("ERROR: cannot save session to %s\n"), session_file->str);
			return;
		}

		for(walk = editors; walk!= NULL; walk = g_slist_next(walk)) {
			editor = walk->data;
			if (editor) {
				if (!editor->is_untitled) {
					if (editor == current_focus_editor) {
						fputs("*", fp);
					}
					if (GTK_IS_SCINTILLA(editor->scintilla)) {
						fputs(editor->filename->str, fp);
						fputs("\n", fp);
					}
					else { // it's a help page
						fputs("phphelp:", fp);
						fputs(editor->help_function, fp);
						fputs("\n", fp);
					}
				}
			}
		}
		
		fclose(fp);
	}
}


void session_reopen(void)
{
	GString *session_file;
	FILE *fp;
	char buf[16384];
	char *filename;
	int focus_tab=-1;
	gboolean focus_this_one = FALSE;
	GString *target;

	session_file = g_string_new( g_get_home_dir());
	session_file = g_string_append(session_file, "/.gphpedit/session");
	
	if (g_file_exists(session_file->str)) {
		fp = fopen(session_file->str, "r");
  		if (!fp) {	
			g_print(_("ERROR: cannot open session file (%s)\n"), session_file->str);
			return;
		}

		while (fgets(buf, sizeof(buf), fp)) {
			/* buf contains possibly:
				file:///blah\n
				*file:///blah\n
				phphelp:function\n
				*phphelp:function\n
			*/
			
			filename = buf;
			str_replace(filename, 10, 0);
			if (buf[0]=='*') {
				filename++;
				focus_this_one = TRUE;
			}

			if (strstr(filename, "phphelp:")) {
				filename += 8;
				target = g_string_new(filename);
				tab_create_new(TAB_HELP, target);
				g_string_free(target, TRUE);
				if (focus_this_one && (main_window.current_editor)) {
					focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),main_window.current_editor->help_scrolled_window);
				}
			}
			else {
				switch_to_file_or_open(filename,0);
				if (focus_this_one && (main_window.current_editor)) {
					focus_tab = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),main_window.current_editor->scintilla);
				}
			}
			
			focus_this_one=FALSE;
		}
		
		fclose(fp);
		gtk_notebook_set_current_page( GTK_NOTEBOOK(main_window.notebook_editor), focus_tab);

		unlink(session_file->str);
	}
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
	//g_io_channel_unref(inter_gphpedit_io);
	//unlink("/tmp/gphpedit.sock");
	quit_application();
	
	// Old code had a main_window_delete_event call in here, not necessary, Gtk/GNOME does that anyway...
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


gboolean main_window_delete_event(GtkWidget *widget,
								  GdkEvent *event, gpointer user_data)
{
	gboolean cancel_quit = FALSE;

	cancel_quit = !can_all_tabs_be_saved();

	if (cancel_quit) {
		update_app_title();
	}
	return cancel_quit;
}


void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
{
	//main_window_size_save_details(allocation->x, allocation->y, allocation->width, allocation->height);
	main_window_size_save_details();
}

void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
	gboolean maximized = GDK_WINDOW_STATE_MAXIMIZED && event->new_window_state;
	gnome_config_set_bool("gPHPEdit/main_window/maximized", maximized);
	gnome_config_sync();
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
	save_classbrowser_position();
	return TRUE;
}

gint main_window_key_press_event(GtkWidget   *widget,
								 GdkEventKey *event,gpointer user_data)
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
					   gtk_entry_set_text(GTK_ENTRY(main_window.toolbar_find_search_entry), search_buffer);
				}
				gtk_widget_grab_focus(GTK_WIDGET(main_window.toolbar_find_search_entry));
				//gtk_editable_select_region(GTK_EDITABLE(main_window.toolbar_find_search_entry),0, -1);
			}
			return TRUE;
		}
		else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_g) || (event->keyval == GDK_G))) {
			gtk_widget_grab_focus(GTK_WIDGET(main_window.toolbar_find_goto_entry));
			return TRUE;
		}
		else if (((event->state & (GDK_CONTROL_MASK | GDK_SHIFT_MASK))==(GDK_CONTROL_MASK | GDK_SHIFT_MASK)) && (event->keyval == GDK_space)) {
			current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
			show_call_tip(main_window.current_editor->scintilla, current_pos);
			return TRUE;
		}
		else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_j) || (event->keyval == GDK_J)))	{
			template_find_and_insert();
			return TRUE;
		}
		else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && ((event->keyval == GDK_F2)))	{
			///add a marker
			current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
			current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
			mod_marker(current_line);
			return TRUE;
		}
		else if ((event->keyval == GDK_F2))	{
			///add a marker
			current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
			current_line = gtk_scintilla_line_from_position(GTK_SCINTILLA(main_window.current_editor->scintilla), current_pos);
			find_next_marker(current_line);
			return TRUE;
		}
		else if ((event->state & GDK_CONTROL_MASK)==GDK_CONTROL_MASK && (event->keyval == GDK_space) && 
				 (main_window.current_editor->type != TAB_HELP)) {
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

void reopen_recent(GtkWidget *widget, gpointer data)
{
	gchar *filename;
	GString *key;
	
	key = g_string_new("gPHPEdit/recent/");
	g_string_append_printf(key, "%d=NOTFOUND", (gint)data); // Back to being gint from gulong due to compiler warning
	filename = gnome_config_get_string (key->str);
	g_string_free(key, TRUE);

	if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:reopen_recent:filename: %s\n", filename); }
	
	switch_to_file_or_open(filename, 0);
}

void run_plugin(GtkWidget *widget, gpointer data)
{
	plugin_exec((gulong) data);// was (gint)
}

gchar *get_gnome_vfs_dirname(gchar *filename)
{
	gchar *end = NULL;
	gchar *dirname = NULL;
	gint length=0;
	
	end = strrchr(filename, '/');
	if (end) {
		length = (end-filename);
		dirname = g_malloc0(length+1);
		strncpy(dirname, filename, length);
	}
	return dirname;
}

GString *get_folder(GString *filename)
{
	gchar *dir;
	GString *folder;

	if (strstr(filename->str, "/")) {
		dir = get_gnome_vfs_dirname(filename->str);
	}
	else {
		dir = g_get_current_dir();
	}
	folder = g_string_new(dir);
	folder = g_string_append(folder, "/");
	g_free(dir);

	return folder;
}

void on_openselected1_activate(GtkWidget *widget)
{
	GSList *li;
	Editor *editor;

	gint current_pos;
	gint wordStart;
	gint wordEnd;
	gchar *ac_buffer;
	gint ac_length;
	GString *file;
	GnomeVFSURI *uri;

	if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP) 
		return;
	
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
	wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
	wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
	ac_buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);

	for(li = editors; li!= NULL; li = g_slist_next(li)) {
		editor = li->data;
		if (editor) {
			if (editor->opened_from) {
				file = get_folder(editor->opened_from);
				if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate (opened_from) :file->str: %s\n", file->str); }
			}
			else {
				file = get_folder(editor->filename);
				if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_selected1_activate:file->str: %s\n", file->str); }
			}

			if (!strstr(ac_buffer, "://") && file->str) {
				file = g_string_append(file, ac_buffer);
			}
			else if (strstr(ac_buffer, "://")) {
				file = g_string_new(ac_buffer);
			}

			uri = gnome_vfs_uri_new (file->str);
			if (gnome_vfs_uri_exists (uri))
				switch_to_file_or_open(file->str,0);
			gnome_vfs_uri_unref (uri);
			
			
			if (file) {
				g_string_free(file, TRUE);
			}
		}
	}
}
void add_file_filters(GtkFileChooser *chooser){
	//store file filter
        GtkFileFilter *filter;
	//creates a new file filter
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("PHP files (*.php *.inc)"));
	//add a pattern to the filter
        gtk_file_filter_add_pattern(filter, "*.php");
        gtk_file_filter_add_pattern(filter, "*.inc");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	//set default filter to the dialog
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (chooser),
				     filter);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("HTML files (*.html *.htm *.xhtml)"));
        gtk_file_filter_add_pattern(filter, "*.html");
        gtk_file_filter_add_pattern(filter, "*.htm");
        gtk_file_filter_add_pattern(filter, "*.xhtml");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("XML files (*.xml)"));
        gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Stylesheet files (*.css)"));
        gtk_file_filter_add_pattern(filter, "*.css");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("SQL files (*.sql)"));
        gtk_file_filter_add_pattern(filter, "*.sql");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Javascript files (*.js)"));
        gtk_file_filter_add_pattern(filter, "*.js");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);			
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("C/C++ files (*.c *.h *.cpp *.hh *.cc)"));
        gtk_file_filter_add_pattern(filter, "*.c");
        gtk_file_filter_add_pattern(filter, "*.h");
        gtk_file_filter_add_pattern(filter, "*.cxx");
        gtk_file_filter_add_pattern(filter, "*.cpp");
        gtk_file_filter_add_pattern(filter, "*.cc");
        gtk_file_filter_add_pattern(filter, "*.hxx");
        gtk_file_filter_add_pattern(filter, "*.hh");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Perl Files (*.pl *.ph *.pm)"));
        gtk_file_filter_add_pattern(filter, "*.pl");
        gtk_file_filter_add_pattern(filter, "*.ph");
        gtk_file_filter_add_pattern(filter, "*.pm");

	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("Python Files (*.py *.pyd *.pyw)"));
        gtk_file_filter_add_pattern(filter, "*.py");
        gtk_file_filter_add_pattern(filter, "*.pyd");
        gtk_file_filter_add_pattern(filter, "*.pyw");

	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
	/* generic filter */
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, _("All files"));
        gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (chooser),
				     filter);
}
void on_open1_activate(GtkWidget *widget)
{
	GtkWidget *file_selection_box;
	GString *folder;
	gchar *last_opened_folder;
	// Create the selector widget
	file_selection_box = gtk_file_chooser_dialog_new("Please select files for editing", GTK_WINDOW(main_window.window),
		GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	
	gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(file_selection_box), FALSE);
	gtk_dialog_set_default_response (GTK_DIALOG(file_selection_box), GTK_RESPONSE_ACCEPT);	
	//Add filters to the open dialog
	add_file_filters(GTK_FILE_CHOOSER(file_selection_box));
	last_opened_folder = gnome_config_get_string("gPHPEdit/general/last_opened_folder=NOTFOUND");
	if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_open1_activate:last_opened_folder: %s\n", last_opened_folder); }
	/* opening of multiple files at once */
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_selection_box), TRUE);
	
	if (main_window.current_editor && !main_window.current_editor->is_untitled) {
		folder = get_folder(main_window.current_editor->filename);
		if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_open1_activate:folder: %s\n", folder->str); }
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  folder->str);
		g_string_free(folder, TRUE);
	}
	else if (strcmp(last_opened_folder, "NOTFOUND")!=0) {
		gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  last_opened_folder);
	}
	
	if (gtk_dialog_run(GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
		open_file_ok(GTK_FILE_CHOOSER(file_selection_box));
	}
	
	gtk_widget_destroy(file_selection_box);
}


void save_file_as_confirm_overwrite(gint reply,gpointer data)
{
	GString *filename;

	if (reply==0) {
		filename = data; //g_string_new(gtk_file_selection_get_filename(GTK_FILE_SELECTION(data)));

		// Set the filename of the current editor to be that
		if (main_window.current_editor->filename) {
			g_string_free(main_window.current_editor->filename, TRUE);
		}
		main_window.current_editor->filename=g_string_new(filename->str);
		main_window.current_editor->short_filename = g_path_get_basename(filename->str);

		tab_check_php_file(main_window.current_editor);

		if (main_window.current_editor->opened_from) {
			g_string_free(main_window.current_editor->opened_from, TRUE);
			main_window.current_editor->opened_from = NULL;
		}

		// Call Save method to actually save it now it has a filename
		on_save1_activate(NULL);
	}
}


void save_file_as_ok(GtkFileChooser *file_selection_box)
{
	GString *filename;
	GtkWidget *file_exists_dialog;
	struct stat st;

	// Extract filename from the file selection dialog
	filename = g_string_new(gtk_file_chooser_get_uri(file_selection_box));

	if (stat (filename->str, &st) == 0) {
		file_exists_dialog = gnome_question_dialog_modal(_("This file already exists, are you sure you want to overwrite it?"),
			save_file_as_confirm_overwrite,filename);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_exists_dialog));
	}
	else {
		// Set the filename of the current editor to be that
		if (main_window.current_editor->filename) {
			g_string_free(main_window.current_editor->filename, TRUE);
		}
		main_window.current_editor->filename=g_string_new(filename->str);
		main_window.current_editor->is_untitled=FALSE;
		main_window.current_editor->short_filename = g_path_get_basename(filename->str);
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
}


void on_save1_activate(GtkWidget *widget)
{
	gchar *filename = NULL;
	GnomeVFSAsyncHandle *fg;
	
	if (main_window.current_editor) {
		filename = main_window.current_editor->filename->str;

		//if filename is Untitled
		if (main_window.current_editor->is_untitled) {// && !main_window.current_editor->saved) {
			on_save_as1_activate(widget);
		}
		else {
			gnome_vfs_async_create(&fg, filename, GNOME_VFS_OPEN_WRITE, FALSE, 0755, GNOME_VFS_PRIORITY_DEFAULT, tab_file_save_opened, main_window.current_editor);
		}
	}
}


void on_saveall1_activate(GtkWidget *widget)
{
	FILE *stream;
	gchar *write_buffer = NULL;
	gsize text_length;
	gint status;
	gchar *filename;
	GSList *li;
	Editor *editor;

	for(li = editors; li!= NULL; li = g_slist_next(li)) {
		editor = li->data;
		if (editor) {
			filename = editor->filename->str;
			if (strcmp(filename, _("Untitled"))!=0) {
				text_length = gtk_scintilla_get_length(GTK_SCINTILLA(editor->scintilla));
	
				write_buffer = g_malloc0(text_length+1); // Include terminating null
	
				if (write_buffer == NULL) {
					g_warning ("%s", _("Cannot allocate write buffer"));
					return;
				}
	
				if ((stream = fopen (filename, "w")) == NULL) {
					g_message (_("Cannot open '%s' for writing"), editor->filename->str);
					return;
				}
	
				gtk_scintilla_get_text(GTK_SCINTILLA(editor->scintilla), text_length+1, write_buffer);
	
				status = fwrite (write_buffer, text_length, 1, stream);
	
				fclose (stream);
	
				g_free (write_buffer);
	
				gtk_scintilla_set_save_point (GTK_SCINTILLA(editor->scintilla));
			}
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
	
		last_opened_folder = gnome_config_get_string("gPHPEdit/general/last_opened_folder=NOTFOUND");
		if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:last_opened_folder: %s\n", last_opened_folder); }
	
		if (main_window.current_editor) {
			filename = main_window.current_editor->filename->str;
			if (main_window.current_editor->is_untitled == FALSE) {
				gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(file_selection_box), filename);
			}
			else {
				if (strcmp(last_opened_folder, "NOTFOUND")!=0) {
					if (DEBUG_MODE) { g_print("DEBUG: main_window_callbacks.c:on_save_as1_activate:Setting current_folder_uri to %s\n", last_opened_folder); }
					gtk_file_chooser_set_current_folder_uri(GTK_FILE_CHOOSER(file_selection_box),  last_opened_folder);
				}
			}
		}

		if (gtk_dialog_run (GTK_DIALOG(file_selection_box)) == GTK_RESPONSE_ACCEPT) {
			save_file_as_ok(GTK_FILE_CHOOSER(file_selection_box));
		}

		gtk_widget_destroy(file_selection_box);		
	}
}

void on_reload_confirm(gint reply,gpointer filename)
{
	if (reply==0) {
		tab_load_file(main_window.current_editor);
	}
}


void on_reload1_activate(GtkWidget *widget)
{
	GtkWidget *file_revert_dialog;
	if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP) 
		return;

	if (main_window.current_editor && (main_window.current_editor->saved == FALSE)) {
		file_revert_dialog = gnome_question_dialog_modal(_("Are you sure you wish to reload the current file, losing your changes?"),
							 on_reload_confirm,NULL);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_revert_dialog));
	}
	else if (main_window.current_editor) {
		tab_load_file(main_window.current_editor);
	}
}

void on_tab_close_activate(GtkWidget *widget, Editor *editor)
{
	try_close_page(editor);
}

void on_tab_close_set_style(GtkWidget *hbox, GtkWidget *button)
{
	gint w=0, h=0;
	gtk_icon_size_lookup_for_settings(gtk_widget_get_settings(hbox),
									GTK_ICON_SIZE_MENU,
									&w,
									&h);
	//debug("%d-%d", w, h);
	gtk_widget_set_size_request(button, w+2, h+2);
}



void rename_file(GString *newfilename)
{
	// unlink old filename
	// unlink((main_window.current_editor->filename->str));
	gnome_vfs_unlink((const gchar *)main_window.current_editor->filename->str);
	// set current_editor->filename
	main_window.current_editor->filename=newfilename;
	main_window.current_editor->short_filename = g_path_get_basename(newfilename->str);

	// save as new filename
	on_save1_activate(NULL);
}


void rename_file_overwrite_confirm(gint reply,gpointer filename)
{
	if (reply==0) {
		// Call rename_file
		rename_file(filename);
	}
}



void rename_file_ok(GtkFileChooser *file_selection)
{
	GString *filename;
	GtkWidget *file_exists_dialog;
	struct stat st;

	// Extract filename from the file chooser dialog
	filename = g_string_new(gtk_file_chooser_get_uri(file_selection));

	if (stat (filename->str, &st) == 0) {
		file_exists_dialog = gnome_question_dialog_modal(_("This file already exists, are you sure you want to overwrite it?"),
							 save_file_as_confirm_overwrite,filename);
		gnome_dialog_run_and_close(GNOME_DIALOG(file_exists_dialog));
	}

	rename_file(filename);
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

/**
 * Close a tab in the Editor. Removes the notebook page, frees the editor object,
 * and sets the active tab correcty
 * 
 * @param editor - The editor object corresponding to the tab that is going to be closed.
 * @return void
 * 
 */

void close_page(Editor *editor)
{
	gint page_num;
	gint page_num_closing;
	gint current_active_tab;

	if (GTK_IS_SCINTILLA(editor->scintilla)) {
		page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window.notebook_editor),editor->scintilla);
		g_free(editor->short_filename);
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
	g_free(editor);
	gtk_notebook_remove_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num_closing);
}


gboolean try_save_page(Editor *editor, gboolean close_if_can)
{
	GtkWidget *confirm_dialog;
	int ret;
	GString *string;

	string = g_string_new("");
	g_string_printf(string, _("The file '%s' has not been saved since your last changes, are you sure you want to close it and lose these changes?"), editor->short_filename);
	confirm_dialog = gnome_message_box_new (string->str,GNOME_MESSAGE_BOX_WARNING,
		_("Close and _lose changes"), _("_Save file"), _("_Cancel closing"), NULL);
	g_string_printf(string,_("Unsaved changes to '%s'"), editor->filename->str);
	gtk_window_set_title(GTK_WINDOW(confirm_dialog), string->str);
	ret = gnome_dialog_run (GNOME_DIALOG (confirm_dialog));
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
	if (!editor->saved && editor->type!=TAB_HELP) {
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
	}
	//gtk_scintilla_cut(GTK_SCINTILLA(main_window.current_editor->scintilla));
}

static gchar *gtkhtml2_selection_get_text (HtmlView *view)
{
	GSList *list = view->sel_list;
	GString *str = g_string_new ("");
	gchar *ptr;

	if (view->sel_list == NULL)
		return NULL;
	
	while (list) {
		HtmlBoxText *text = HTML_BOX_TEXT (list->data);

		list = list->next;
		/*
		 * Some boxes may not have any text
		 */
		if (text->canon_text == NULL)
			continue;
		switch (text->selection) {
		case HTML_BOX_TEXT_SELECTION_NONE:
			g_assert_not_reached ();
			break;
		case HTML_BOX_TEXT_SELECTION_END:
			g_string_append_len (str, (gchar *)text->canon_text, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_end_index) - (gchar *)text->canon_text);
			break;
		case HTML_BOX_TEXT_SELECTION_START:
			g_string_append_len (str, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_start_index),
					 g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->length) - g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->sel_start_index));
			break;
		case HTML_BOX_TEXT_SELECTION_FULL:
			g_string_append_len (str, (gchar *)text->canon_text, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)text->length) - (gchar *)text->canon_text);
			break;
		case HTML_BOX_TEXT_SELECTION_BOTH:
			g_string_append_len (str, g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MIN (text->sel_start_index, text->sel_end_index)),
					g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MAX (text->sel_end_index, text->sel_start_index)) - 
					g_utf8_offset_to_pointer ((gchar *)text->canon_text, (glong)MIN (text->sel_start_index, text->sel_end_index)));
			break;
		}
	}
	ptr = str->str;
	g_string_free (str, FALSE);
	return ptr;
}

void on_copy1_activate(GtkWidget *widget)
{
	gint wordStart;
	gint wordEnd;
	gint length;
	gchar *buffer;
	
	if (main_window.current_editor == NULL)
		return;
	
	if (main_window.current_editor->type == TAB_HELP) {
		buffer = gtkhtml2_selection_get_text(HTML_VIEW(main_window.current_editor->help_view));
		gtk_clipboard_set_text(main_window.clipboard, buffer, 1);
		g_free(buffer);
	}
	else {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		if (wordStart != wordEnd) {
			buffer = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &length);
	
			gtk_clipboard_set_text(main_window.clipboard,buffer,length);
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

void on_paste1_activate(GtkWidget *widget)
{
	if (main_window.current_editor == NULL || main_window.current_editor->type == TAB_HELP)
		return;
	
	gtk_clipboard_request_text(main_window.clipboard, on_paste_got_from_cliboard,main_window.current_editor->scintilla);
	
	//gtk_scintilla_paste(GTK_SCINTILLA(main_window.current_editor->scintilla));

	// Possible fix for rendering issues after pasting
	//gtk_scintilla_colourise(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, -1);
}


void on_selectall1_activate(GtkWidget *widget)
{
	if (main_window.current_editor == NULL)
		return;
	gtk_scintilla_select_all(GTK_SCINTILLA(main_window.current_editor->scintilla));
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


void on_properties1_activate(GtkWidget *widget)
{}


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
						g_print(_("Unhandle keyboard macro function %d, please report to macro@gphpedit.org\n"), event->message);
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
								"Current Maintainer",
								"Anoop John <anoop.john@zyxware.com>",
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
	const gchar *documenters[] = {
									 NULL
								 };
	gchar *translator_credits = _("translator_credits");
	GtkWidget *about;
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;

	pixbuf = gdk_pixbuf_new_from_file (PIXMAP_DIR "/" GPHPEDIT_PIXMAP_ICON, &error);

	if (error) {
		g_warning (G_STRLOC ": cannot open icon: %s", error->message);
		g_error_free (error);
	}
	about = gnome_about_new ("gPHPEdit", VERSION,
							 _("Copyright  2003-2006 Andy Jeffries, 2009 Anoop John"),
						 	 _("gPHPEdit is a GNOME2 editor specialised for editing PHP "
							 "scripts and related files (HTML/CSS/JS)."),
							 (const gchar **) authors,
							 (const gchar **) documenters,
							 strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
							 pixbuf);
	gtk_widget_show(about);

	if (pixbuf) {
		gdk_pixbuf_unref (pixbuf);
	}
		
	gtk_window_set_transient_for (GTK_WINDOW (about), NULL);
	/* This line causes a segfault - should be right but commented out!
	gtk_signal_connect(GTK_OBJECT(about), "destroy",
					   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);*/
	gtk_widget_show (about);
}

void on_notebook_switch_page (GtkNotebook *notebook, GtkNotebookPage *page,
							  gint page_num, gpointer user_data)
{
	Editor *data;
	GtkWidget *child;

	child = gtk_notebook_get_nth_page(GTK_NOTEBOOK(main_window.notebook_editor), page_num);
	data = editor_find_from_scintilla(child);
	if (data) {
		if (GTK_IS_SCINTILLA(data->scintilla)) {
			// Grab the focus in to the editor
			gtk_scintilla_grab_focus(GTK_SCINTILLA(data->scintilla));

			// Store it in the global main_window.current_editor value
			main_window.current_editor = data;
		}
	}
	else {
		data = editor_find_from_help((void *)child);
		if (data) {
			main_window.current_editor = data;
		}
		else {
 			g_print(_("Unable to get data for page %d\n"), page_num);
		}
	}
	
	if (!is_app_closing) {
		// Change the title of the main application window to the full filename
		update_app_title();
	}
	on_tab_change_update_classbrowser(main_window.notebook_editor);
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
	gint current_pos;

	gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
	gtk_scintilla_goto_line(GTK_SCINTILLA(main_window.current_editor->scintilla), line-1); // seems to be off by one...
	current_pos = gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla));
	gtk_scintilla_scroll_caret(GTK_SCINTILLA(main_window.current_editor->scintilla));
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
	move_block(preferences.indentation_size);
}


void block_unindent(GtkWidget *widget)
{
	move_block(0-preferences.indentation_size);
}
//zoom in

void zoom_in(GtkWidget *widget)
{
gtk_scintilla_zoom_in(GTK_SCINTILLA(main_window.current_editor->scintilla));
}

//zoom out
void zoom_out(GtkWidget *widget)
{
gtk_scintilla_zoom_out(GTK_SCINTILLA(main_window.current_editor->scintilla));
}

//add marker
void add_marker(int line)
{
gtk_scintilla_marker_define(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, SC_MARK_SHORTARROW);
gtk_scintilla_marker_set_back(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, 101);
gtk_scintilla_marker_set_fore(GTK_SCINTILLA(main_window.current_editor->scintilla), 1, 101);
gtk_scintilla_marker_add(GTK_SCINTILLA(main_window.current_editor->scintilla), line, 2);
}
//delete marker
void delete_marker(int line)
{
gtk_scintilla_marker_delete(GTK_SCINTILLA(main_window.current_editor->scintilla), line, 1);
}
//add/delete a marker
void mod_marker(int line){
	if (gtk_scintilla_marker_get(GTK_SCINTILLA(main_window.current_editor->scintilla),line)!=2){
	add_marker(line);
	}else{
	delete_marker(line);
	}
}
//circle markers
//no funciona esta mal el markermask
void find_next_marker(line_start){
gint line;
line= gtk_scintilla_marker_next(GTK_SCINTILLA(main_window.current_editor->scintilla),line_start, 1);
	if (line==-1){
		//no markers in that direccion, we should go back to the first line
		line= gtk_scintilla_marker_previous(GTK_SCINTILLA(main_window.current_editor->scintilla),0, 1);
		if (line!=-1){
		//go back to the first marker
		goto_line_int(line);
		}else{	
		g_print("No marker found");
		}
	}else{
	//goto the marker posicion
	goto_line_int(line);
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

void classbrowser_show(void)
{
	gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),classbrowser_hidden_position);
	//g_print("Width of class browser is %d\n", classbrowser_hidden_position);
	gnome_config_set_int ("gPHPEdit/main_window/classbrowser_hidden",0);
	gnome_config_sync();
	classbrowser_update();
}


void classbrowser_hide(void)
{
	classbrowser_hidden_position = gnome_config_get_int ("gPHPEdit/main_window/classbrowser_size=100");
	//g_print("Width of class browser is %d\n", classbrowser_hidden_position);
	gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),0);
	gnome_config_set_int ("gPHPEdit/main_window/classbrowser_hidden",1);
	gnome_config_sync();
}

void classbrowser_show_hide(GtkWidget *widget)
{
	gint hidden;
	
	hidden = gnome_config_get_int ("gPHPEdit/main_window/classbrowser_hidden=0");
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

		// TODO: AJ 2003-02-07
		// get file from selection
		// switch to that file, if not currently selected

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
		gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
		gtk_scintilla_scroll_caret(GTK_SCINTILLA(main_window.current_editor->scintilla));
		gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
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
	if(GTK_TOGGLE_BUTTON (main_window.chkOnlyCurFileFuncs)->active)
	{
		//debug("Is set");
		classbrowser_update();
	}
	return 0;
}
