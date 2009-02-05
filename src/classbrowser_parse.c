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


#include "classbrowser_parse.h"
//#define DEBUG_CLASSBROWSER 1


static gchar *read_text_file( gchar *filename )
{
	FILE *fp;
	gchar *buffer = NULL;
	struct stat buf;
	size_t size_read;

	stat(filename, &buf);

	buffer = g_malloc(buf.st_size+1);

	fp = fopen( filename, "r" );

	if (fp) {
		size_read = fread(buffer, 1, buf.st_size, fp);
		if (size_read != buf.st_size) {
			g_print("ERROR: classbrowser reading %s, read in %d bytes, expecting %d bytes\n", filename, size_read, (guint)(buf.st_size));
		}
		buffer[buf.st_size] = '\0';
		fclose(fp);
	}

	return buffer;
}


static gboolean is_whitespace(gchar character)
{
	if ( (character == ' ') ||
	        (character == '\t') ||
	        (character == '\n') ||
	        (character == '\r') ) {
		return TRUE;
	}

	return FALSE;
}


static gboolean is_identifier_char(gchar character)
{
	if ((character >= 'a') && (character <= 'z')) {
		return TRUE;
	}
	if ((character >= 'A') && (character <= 'Z')) {
		return TRUE;
	}
	if ((character >= '0') && (character <= '9')) {
		return TRUE;
	}
	if (character == '_') {
		return TRUE;
	}
	return FALSE;
}


static gboolean is_opening_brace(gchar character)
{
	if (character == '{') {
		return TRUE;
	}
	return FALSE;
}


static gboolean is_closing_brace(gchar character)
{
	if (character == '}') {
		return TRUE;
	}
	return FALSE;
}


static gboolean is_opening_parenthesis(gchar character)
{
	if (character == '(') {
		return TRUE;
	}
	return FALSE;
}


static gboolean is_closing_parenthesis(gchar character)
{
	if (character == ')') {
		return TRUE;
	}
	return FALSE;
}


static gboolean is_newline(gchar character)
{
	//if (character == '\n' || character == '\r') {
	if (character == '\n') {
		return TRUE;
	}
	return FALSE;
}


static gboolean check_previous(gchar *original, gchar *current, gchar *search)
{
	gint search_len;

	search_len = strlen(search);

	// First, make sure we're not going to go out of bounds.
	if (current-original < search_len) {
		return FALSE;
	}

	// Second, go back from current search_len positions
	current = current - search_len;

	// Then check to see if we match
	if (g_ascii_strncasecmp(current, search, search_len)==0) {
		return TRUE;
	}

	return FALSE;
}


static gboolean non_letter_before(gchar *original, gchar *current, gchar *search)
{
	gint search_len;

	search_len = strlen(search);

	// First, make sure we're not going to go out of bounds.
	if (current-original < (search_len+1)) {
		return FALSE;
	}

	// Second, go back from current search_len positions
	current = current - (search_len+1);

	// Then check to see if we match
	if (!is_identifier_char(*current) && *current != '$') {
		return TRUE;
	}

	return FALSE;
}


void classbrowser_parse_file(gchar *filename)
{
	gchar *file_contents;
	gchar *o; // original pointer to start of contents
	gchar *c; // current position within contents

	gboolean within_php;
	gboolean within_single_line_comment;
	gboolean within_multi_line_comment;
	gboolean within_single_string;
	gboolean within_double_string;
	guint brace_count;
	guint parenthesis_count;
	guint line_number;

	gchar *within_class;
	guint class_length;
	gboolean looking_for_class_name;
	gboolean within_class_name;
	gchar *start_class_name = NULL;
	gchar *within_function;
	guint function_length = 0;
	gboolean looking_for_function_name;
	gboolean within_function_name;
	gchar *start_function_name = NULL;
	gboolean within_function_param_list;
	gchar *start_param_list;
	gchar *param_list;
	guint param_list_length;
	gboolean function_awaiting_brace_or_parenthesis;

	file_contents = read_text_file(filename);

	o = file_contents;
	c = o;

	within_php = FALSE;
	within_single_line_comment = FALSE;
	within_multi_line_comment = FALSE;
	within_single_string = FALSE;
	within_double_string = FALSE;
	brace_count = 0;
	line_number = 1;
	within_class = NULL;
	looking_for_class_name = FALSE;
	within_class_name = FALSE;
	within_function = NULL;
	looking_for_function_name = FALSE;
	within_function_name = FALSE;
	within_function_param_list = FALSE;
	start_param_list = NULL;
	param_list = NULL;
	function_awaiting_brace_or_parenthesis = FALSE;

	while (*c) {
		if (!within_php) {
			if (check_previous(o, c, "<?")) {
				within_php=TRUE;
			}
		}
		else {
			if (within_single_line_comment && is_newline(*c)) {
				within_single_line_comment = FALSE;
			}
			else if (within_multi_line_comment && check_previous(o, c, "*/")) {
				within_multi_line_comment = FALSE;
			}
			if (within_single_string && *c=='\'') {
				within_single_string = FALSE;
			}
			else if (within_double_string && *c=='"') {
				within_double_string = FALSE;
			}
			else if (!within_multi_line_comment && !within_single_line_comment && !within_double_string && !within_single_string) {
				if (check_previous(o, c, "?>")) {
					within_php = FALSE;
				}
				else if (*c == '\'' && !check_previous(o, c, "\\'")) {
					within_single_string=TRUE;
				}
				else if (*c == '"' && !check_previous(o, c, "\\\"")) {
					within_double_string=TRUE;
				}
				else if (check_previous(o, c, "//")) {
					within_single_line_comment = TRUE;
				}
				else if (check_previous(o, c, "/*")) {
					within_multi_line_comment = TRUE;
				}
				else {
					if (check_previous(o, c, "class ") && non_letter_before(o, c, "class ")) {
#ifdef DEBUG_CLASSBROWSER
						g_print("%s(%d): Found Class\n", filename, line_number);
#endif

						looking_for_class_name = TRUE;
					}
					else if (is_identifier_char(*c) && looking_for_class_name && !within_class_name) {
						start_class_name = c-1;
						looking_for_class_name = FALSE;
						within_class_name = TRUE;
					}
					else if ( (is_whitespace(*c) || is_opening_brace(*c)) && within_class_name) {
						class_length = (c - start_class_name);
						if (within_class) {
							g_free(within_class);
						}
						within_class = g_malloc(class_length+1);
						strncpy(within_class, start_class_name, class_length);
						within_class[class_length]='\0';
#ifdef DEBUG_CLASSBROWSER

						g_print("%s(%d): Class '%s'\n", filename, line_number, within_class);
#endif

						classbrowser_classlist_add(within_class, filename, line_number);
						within_class_name = FALSE;
					}
					else if (check_previous(o, c, "function ") && non_letter_before(o, c, "function ")) {
#ifdef DEBUG_CLASSBROWSER
						g_print("Looking for function name\n");
#endif

						looking_for_function_name = TRUE;
					}

					if (is_identifier_char(*c) && looking_for_function_name && !within_function_name) {
#ifdef DEBUG_CLASSBROWSER
						g_print("Storing function name\n");
#endif

						start_function_name = c;
						function_length = 0;
						looking_for_function_name = FALSE;
						within_function_name = TRUE;
					}

					if ( (is_whitespace(*c) || is_opening_brace(*c) || is_opening_parenthesis(*c)) && within_function_name && function_length==0) {
#ifdef DEBUG_CLASSBROWSER
						g_print("Found function\n");
#endif

						function_length = (c - start_function_name);
						if (within_function) {
							g_free(within_function);
						}
						within_function = g_malloc(function_length+1);
						strncpy(within_function, start_function_name, function_length);
						within_function[function_length]='\0';
						function_awaiting_brace_or_parenthesis = TRUE;
						within_function_name = FALSE;
					}

					if ( function_awaiting_brace_or_parenthesis && is_opening_brace(*c)) {
						function_awaiting_brace_or_parenthesis = FALSE;
						if (within_class) {
							classbrowser_functionlist_add(within_class, within_function, filename, line_number, NULL);
#ifdef DEBUG_CLASSBROWSER

							g_print("%s(%d): Class method %s::%s\n", filename, line_number, within_class, within_function);
#endif

						}
						else {
							classbrowser_functionlist_add(NULL, within_function, filename, line_number, NULL);
#ifdef DEBUG_CLASSBROWSER

							g_print("%s(%d): Function %s\n", filename, line_number, within_function);
#endif

						}
					}
					else if (function_awaiting_brace_or_parenthesis && is_opening_parenthesis(*c)) {
						within_function_param_list = TRUE;
						start_param_list = c+1;
						function_awaiting_brace_or_parenthesis = FALSE;
					}
					else if (is_closing_parenthesis(*c) && within_function_param_list) {
						param_list_length = (c - start_param_list);
						if (param_list) {
							g_free(param_list);
						}
						param_list = g_malloc(param_list_length+1);
						strncpy(param_list, start_param_list, param_list_length);
						param_list[param_list_length]='\0';
						//TODO: condense_param_list(&param_list);
						if (within_class) {
							classbrowser_functionlist_add(within_class, within_function, filename, line_number, param_list);
#ifdef DEBUG_CLASSBROWSER

							g_print("%s(%d): Class method %s::%s(%s)\n", filename, line_number, within_class, within_function, param_list);
#endif

						}
						else {
							classbrowser_functionlist_add(NULL, within_function, filename, line_number, param_list);
#ifdef DEBUG_CLASSBROWSER

							g_print("%s(%d): Function %s(%s)\n", filename, line_number, within_function, param_list);
#endif

						}
						within_function_param_list = FALSE;
					}


					if (is_opening_brace(*c)) {
						brace_count++;
#ifdef DEBUG_CLASSBROWSER

						g_print("Brace count %d:%c\n", brace_count, *c);
#endif

					}
					else if (is_closing_brace(*c)) {
						brace_count--;
#ifdef DEBUG_CLASSBROWSER

						g_print("Brace count %d:%c\n", brace_count, *c);
#endif

						if (brace_count == 0) {
							if (within_class) {
#ifdef DEBUG_CLASSBROWSER
								g_print("Freeing class %s\n", within_class);
#endif

								g_free(within_class);
								within_class = NULL;
							}
						}
					}
					else if (is_opening_parenthesis(*c)) {
						parenthesis_count++;
					}
					else if (is_closing_parenthesis(*c)) {
						parenthesis_count--;
					}
				}
			}

		}

		if (is_newline(*c)) {
			line_number++;
		}

		c++;
	}

	g_free(file_contents);
}
