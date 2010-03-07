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
#include "stdlib.h"
#include "syntax_check.h"
#include "preferences.h"
#include "main_window.h"
#include "main_window_callbacks.h"

gchar *run_php_lint(gchar *command_line)
{
	gboolean result;
	gchar *stdout;
	gint exit_status;
	GError *error;

	error = NULL;

	result = g_spawn_command_line_sync (command_line,
	                                    &stdout, NULL, &exit_status, &error);

	if (!result) {
		return NULL;
	}
	return stdout;
}


void syntax_add_lines(gchar *output)
{
	GtkTreeIter   iter;
	gchar *copy;
	gchar *token;
	gchar *line_number;
	gchar *first_error = NULL;
	gint line_start;
	gint line_end;
	gint indent;

	first_error = 0;
	copy = output;
	gtk_scintilla_set_indicator_current(GTK_SCINTILLA(main_window.current_editor->scintilla), 20);
	gtk_scintilla_indic_set_style(GTK_SCINTILLA(main_window.current_editor->scintilla), 20, INDIC_SQUIGGLE);
	gtk_scintilla_indic_set_fore(GTK_SCINTILLA(main_window.current_editor->scintilla), 20, 0x0000ff);
	while ((token = strtok(copy, "\n"))) {
		if ((strncmp(token, "PHP Warning:  ", MIN(strlen(token), 14))!=0) && (strncmp(token, "Content-type", MIN(strlen(token), 12))!=0)) { 
			gtk_list_store_append (main_window.lint_store, &iter);
			gtk_list_store_set (main_window.lint_store, &iter, 0, token, -1);
	
			line_number = strrchr(token, ' ');
			line_number++;
	
			if (atoi(line_number)>0) {
				if (!first_error) {
					first_error = line_number;
				}
				indent = gtk_scintilla_get_line_indentation(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
	
				line_start = gtk_scintilla_position_from_line(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
				line_start += (indent/preferences.indentation_size);
	
				line_end = gtk_scintilla_get_line_end_position(GTK_SCINTILLA(main_window.current_editor->scintilla), atoi(line_number)-1);
							
//				gtk_scintilla_start_styling(GTK_SCINTILLA(main_window.current_editor->scintilla), line_start, 128);
//				gtk_scintilla_set_styling(GTK_SCINTILLA(main_window.current_editor->scintilla), line_end-line_start, INDIC2_MASK);
				gtk_scintilla_indicator_fill_range(GTK_SCINTILLA(main_window.current_editor->scintilla), line_start, line_end-line_start);
			}
			else {
				g_print("Line number is 0\n");
			}
		}
		copy = NULL;
	}

	if (first_error) {
		goto_line(first_error);
	}
}


GString *save_as_temp_file(void)
{
	gchar *write_buffer = NULL;
	gsize text_length;
	gint status;
	gchar *rawfilename;
	GString *filename;
	int file_handle;

	file_handle = g_file_open_tmp("gphpeditXXXXXX",&rawfilename,NULL);
	if (file_handle != -1) {
		filename = g_string_new(rawfilename);
		
		text_length = gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla));
		write_buffer = g_malloc0(text_length+1); // Include terminating null

		if (write_buffer == NULL) {
			g_warning ("%s", _("Cannot allocate write buffer"));
			return NULL;
		}
		
		gtk_scintilla_get_text(GTK_SCINTILLA(main_window.current_editor->scintilla), text_length+1, write_buffer);
	
		status = write (file_handle, write_buffer, text_length+1);
		
		g_free (write_buffer);
		g_free(rawfilename);
		close(file_handle);

		return filename;
	}
	
	return NULL;
}


void syntax_check_run(void)
{
	GtkTreeIter iter;
	GString *command_line;
	gchar *output;
	gboolean using_temp;
	GString *filename;
	
	/* Tim: this doesn't work if php_binary_location is not an absolute path 

	struct stat *buf = NULL;
	if (stat(preferences.php_binary_location, buf)==-1) {
		g_print("PHP command line binary not found.\n");
	}
	else */
	if (main_window.current_editor) {
		if (main_window.current_editor->saved==TRUE) {
			filename = g_string_new(editor_convert_to_local(main_window.current_editor));
			using_temp = FALSE;
		}
		else {
			filename = save_as_temp_file();
			using_temp = TRUE;
		}

		command_line = g_string_new(preferences.php_binary_location);
		command_line = g_string_append(command_line, " -q -l -d html_errors=Off -f ");
		command_line = g_string_append(command_line, filename->str);

		g_print("eject:%s\n", command_line->str);

		output = run_php_lint(command_line->str);
		g_string_free(command_line, TRUE);

		
		main_window.lint_store = gtk_list_store_new (1, G_TYPE_STRING);

//		gtk_scintilla_start_styling(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, INDIC2_MASK);
//		gtk_scintilla_set_styling(GTK_SCINTILLA(main_window.current_editor->scintilla),
//		                          gtk_scintilla_get_length(GTK_SCINTILLA(main_window.current_editor->scintilla)), 0);
	gtk_scintilla_indicator_clear_range(GTK_SCINTILLA(main_window.current_editor->scintilla), 0, gtk_scintilla_get_text_length(GTK_SCINTILLA(main_window.current_editor->scintilla)));

		gtk_list_store_clear(main_window.lint_store);
		if (output) {
			syntax_add_lines(output);
			g_free(output);
		}
		else {
			gtk_list_store_append (main_window.lint_store, &iter);
			gtk_list_store_set (main_window.lint_store, &iter, 0, _("Error calling PHP CLI (is PHP command line binary installed? If so, check if it's in your path or set php_binary in ~/.gnome2/gPHPEdit)\n"), -1);
		}
		gtk_tree_view_set_model(GTK_TREE_VIEW(main_window.lint_view), GTK_TREE_MODEL(main_window.lint_store));
		
		if (using_temp) {
			unlink(filename->str);
		}
		g_string_free(filename, TRUE);
	}
	else {
		main_window.lint_store = gtk_list_store_new (1, G_TYPE_STRING);
		gtk_list_store_clear(main_window.lint_store);
		gtk_list_store_append (main_window.lint_store, &iter);
		gtk_list_store_set (main_window.lint_store, &iter, 0, _("You don't have any files open to check\n"), -1);
		gtk_tree_view_set_model(GTK_TREE_VIEW(main_window.lint_view), GTK_TREE_MODEL(main_window.lint_store));
	}
}
