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

#ifndef PREFERENCES_H
#define PREFERENCES_H
#include "main.h"

typedef struct
{
	// Window memory
	gint width;
	gint height;
	gint left;
	gint top;
	gboolean maximized;

	// Default settings
	gint set_sel_back;
	gint marker_back;
	gint indentation_size;
	gint tab_size;
	gint show_indentation_guides;
	gint show_folding;
	gint edge_mode;
	gint edge_column;
	gint edge_colour;
	gchar *php_binary_location;
	gchar *shared_source_location;
	//gint auto_indent_after_brace;
	gint auto_complete_delay;
	gint calltip_delay;
	gint line_wrapping;
	gchar *php_file_extensions;
	gboolean save_session;
	gboolean use_tabs_instead_spaces;
	gboolean single_instance_only;
	gboolean showfolderbrowser;
	gint font_quality;	
	GSList *search_history;
	// Default formatting
	gchar *default_font;
	gint default_fore;
	gint default_size;
	gboolean default_bold;
	gboolean default_italic;
	gint default_back;
	gchar *line_number_font;
	gint line_number_fore;
	gint line_number_back;
	gint line_number_size;
	gboolean line_number_bold;
	gboolean line_number_italic;

	// HTML Formatting
	gint html_tag_back;
	gint html_tag_fore;
	gchar *html_tag_font;
	gint html_tag_size;
	gboolean html_tag_bold;
	gboolean html_tag_italic;
	gint html_tag_unknown_back;
	gint html_tag_unknown_fore;
	gchar *html_tag_unknown_font;
	gint html_tag_unknown_size;
	gboolean html_tag_unknown_bold;
	gboolean html_tag_unknown_italic;
	gint html_attribute_back;
	gint html_attribute_fore;
	gchar *html_attribute_font;
	gint html_attribute_size;
	gboolean html_attribute_bold;
	gboolean html_attribute_italic;
	gint html_attribute_unknown_back;
	gint html_attribute_unknown_fore;
	gchar *html_attribute_unknown_font;
	gint html_attribute_unknown_size;
	gboolean html_attribute_unknown_bold;
	gboolean html_attribute_unknown_italic;
	gint html_number_back;
	gint html_number_fore;
	gchar *html_number_font;
	gint html_number_size;
	gboolean html_number_bold;
	gboolean html_number_italic;
	gint html_single_string_back;
	gint html_single_string_fore;
	gchar *html_single_string_font;
	gint html_single_string_size;
	gboolean html_single_string_bold;
	gboolean html_single_string_italic;
	gint html_double_string_back;
	gint html_double_string_fore;
	gchar *html_double_string_font;
	gint html_double_string_size;
	gboolean html_double_string_bold;
	gboolean html_double_string_italic;
	gint html_comment_back;
	gint html_comment_fore;
	gchar *html_comment_font;
	gint html_comment_size;
	gboolean html_comment_bold;
	gboolean html_comment_italic;
	gint html_entity_back;
	gint html_entity_fore;
	gchar *html_entity_font;
	gint html_entity_size;
	gboolean html_entity_bold;
	gboolean html_entity_italic;
	gint html_script_back;
	gint html_script_fore;
	gchar *html_script_font;
	gint html_script_size;
	gboolean html_script_bold;
	gboolean html_script_italic;
	gint html_question_back;
	gint html_question_fore;
	gchar *html_question_font;
	gint html_question_size;
	gboolean html_question_bold;
	gboolean html_question_italic;
	gint html_value_back;
	gint html_value_fore;
	gchar *html_value_font;
	gint html_value_size;
	gboolean html_value_bold;
	gboolean html_value_italic;

	// JavaScript Formatting
	gint javascript_comment_back;
	gint javascript_comment_fore;
	gchar *javascript_comment_font;
	gint javascript_comment_size;
	gboolean javascript_comment_bold;
	gboolean javascript_comment_italic;
	gint javascript_comment_line_back;
	gint javascript_comment_line_fore;
	gchar *javascript_comment_line_font;
	gint javascript_comment_line_size;
	gboolean javascript_comment_line_bold;
	gboolean javascript_comment_line_italic;
	gint javascript_comment_doc_back;
	gint javascript_comment_doc_fore;
	gchar *javascript_comment_doc_font;
	gint javascript_comment_doc_size;
	gboolean javascript_comment_doc_bold;
	gboolean javascript_comment_doc_italic;
	gint javascript_word_back;
	gint javascript_word_fore;
	gchar *javascript_word_font;
	gint javascript_word_size;
	gboolean javascript_word_bold;
	gboolean javascript_word_italic;
	gint javascript_keyword_back;
	gint javascript_keyword_fore;
	gchar *javascript_keyword_font;
	gint javascript_keyword_size;
	gboolean javascript_keyword_bold;
	gboolean javascript_keyword_italic;
	gint javascript_doublestring_back;
	gint javascript_doublestring_fore;
	gchar *javascript_doublestring_font;
	gint javascript_doublestring_size;
	gboolean javascript_doublestring_bold;
	gboolean javascript_doublestring_italic;
	gint javascript_singlestring_back;
	gint javascript_singlestring_fore;
	gchar *javascript_singlestring_font;
	gint javascript_singlestring_size;
	gboolean javascript_singlestring_bold;
	gboolean javascript_singlestring_italic;
	gint javascript_symbols_back;
	gint javascript_symbols_fore;
	gchar *javascript_symbols_font;
	gint javascript_symbols_size;
	gboolean javascript_symbols_bold;
	gboolean javascript_symbols_italic;

	// PHP Formatting
	gchar *php_default_font;
	gint php_default_fore;
	gint php_default_back;
	gint php_default_size;
	gboolean php_default_bold;
	gboolean php_default_italic;
	gint php_hstring_fore;
	gchar *php_hstring_font;
	gint php_hstring_back;
	gint php_hstring_size;
	gboolean php_hstring_bold;
	gboolean php_hstring_italic;
	gint php_simplestring_fore;
	gchar *php_simplestring_font;
	gint php_simplestring_back;
	gint php_simplestring_size;
	gboolean php_simplestring_bold;
	gboolean php_simplestring_italic;
	gint php_word_fore;
	gchar *php_word_font;
	gint php_word_back;
	gint php_word_size;
	gboolean php_word_bold;
	gboolean php_word_italic;
	gint php_number_fore;
	gchar *php_number_font;
	gint php_number_back;
	gint php_number_size;
	gboolean php_number_bold;
	gboolean php_number_italic;
	gint php_variable_fore;
	gchar *php_variable_font;
	gint php_variable_back;
	gint php_variable_size;
	gboolean php_variable_bold;
	gboolean php_variable_italic;
	gint php_comment_fore;
	gchar *php_comment_font;
	gint php_comment_back;
	gint php_comment_size;
	gboolean php_comment_bold;
	gboolean php_comment_italic;
	gint php_comment_line_fore;
	gchar *php_comment_line_font;
	gint php_comment_line_back;
	gint php_comment_line_size;
	gboolean php_comment_line_bold;
	gboolean php_comment_line_italic;

	gint css_tag_back;
	gchar *css_tag_font;
	gint css_tag_fore;
	gint css_tag_size;
	gboolean css_tag_bold;
	gboolean css_tag_italic;
	gint css_class_fore;
	gchar *css_class_font;
	gint css_class_back;
	gint css_class_size;
	gboolean css_class_bold;
	gboolean css_class_italic;
	gint css_pseudoclass_back;
	gchar *css_pseudoclass_font;
	gint css_pseudoclass_fore;
	gint css_pseudoclass_size;
	gboolean css_pseudoclass_bold;
	gboolean css_pseudoclass_italic;
	gint css_unknown_pseudoclass_fore;
	gchar *css_unknown_pseudoclass_font;
	gint css_unknown_pseudoclass_back;
	gint css_unknown_pseudoclass_size;
	gboolean css_unknown_pseudoclass_bold;
	gboolean css_unknown_pseudoclass_italic;
	gint css_operator_fore;
	gchar *css_operator_font;
	gint css_operator_back;
	gint css_operator_size;
	gboolean css_operator_bold;
	gboolean css_operator_italic;
	gint css_identifier_fore;
	gchar *css_identifier_font;
	gint css_identifier_back;
	gint css_identifier_size;
	gboolean css_identifier_bold;
	gboolean css_identifier_italic;
	gint css_unknown_identifier_fore;
	gchar *css_unknown_identifier_font;
	gint css_unknown_identifier_back;
	gint css_unknown_identifier_size;
	gboolean css_unknown_identifier_bold;
	gboolean css_unknown_identifier_italic;
	gint css_value_fore;
	gchar *css_value_font;
	gint css_value_back;
	gint css_value_size;
	gboolean css_value_bold;
	gboolean css_value_italic;
	gint css_comment_fore;
	gchar *css_comment_font;
	gint css_comment_back;
	gint css_comment_size;
	gboolean css_comment_bold;
	gboolean css_comment_italic;
	gchar *css_id_font;
	gint css_id_fore;
	gint css_id_back;
	gint css_id_size;
	gboolean css_id_bold;
	gboolean css_id_italic;
	gchar *css_important_font;
	gint css_important_fore;
	gint css_important_back;
	gint css_important_size;
	gboolean css_important_bold;
	gboolean css_important_italic;
	gint css_directive_fore;
	gchar *css_directive_font;
	gint css_directive_back;
	gint css_directive_size;
	gboolean css_directive_bold;
	gboolean css_directive_italic;
	
	gint sql_word_back;
	gchar *sql_word_font;
	gint sql_word_fore;
	gint sql_word_size;
	gboolean sql_word_bold;
	gboolean sql_word_italic;
	gint sql_identifier_back;
	gchar *sql_identifier_font;
	gint sql_identifier_fore;
	gint sql_identifier_size;
	gboolean sql_identifier_bold;
	gboolean sql_identifier_italic;
	gint sql_number_back;
	gchar *sql_number_font;
	gint sql_number_fore;
	gint sql_number_size;
	gboolean sql_number_bold;
	gboolean sql_number_italic;
	gint sql_string_back;
	gchar *sql_string_font;
	gint sql_string_fore;
	gint sql_string_size;
	gboolean sql_string_bold;
	gboolean sql_string_italic;
	gint sql_operator_back;
	gchar *sql_operator_font;
	gint sql_operator_fore;
	gint sql_operator_size;
	gboolean sql_operator_bold;
	gboolean sql_operator_italic;
	gint sql_comment_back;
	gchar *sql_comment_font;
	gint sql_comment_fore;
	gint sql_comment_size;
	gboolean sql_comment_bold;
	gboolean sql_comment_italic;
	
	//C/C++ formating
	gint c_default_back;
	gchar *c_default_font;
	gint c_default_fore;
	gint c_default_size;
	gboolean c_default_bold;
	gboolean c_default_italic;
	gint c_string_back;
	gchar *c_string_font;
	gint c_string_fore;
	gint c_string_size;
	gboolean c_string_bold;
	gboolean c_string_italic;
	gint c_character_back;
	gchar *c_character_font;
	gint c_character_fore;
	gint c_character_size;
	gboolean c_character_bold;
	gboolean c_character_italic;
	gint c_word_back;
	gchar *c_word_font;
	gint c_word_fore;
	gint c_word_size;
	gboolean c_word_bold;
	gboolean c_word_italic;
	gint c_number_back;
	gchar *c_number_font;
	gint c_number_fore;
	gint c_number_size;
	gboolean c_number_bold;
	gboolean c_number_italic;
	gint c_identifier_back;
	gchar *c_identifier_font;
	gint c_identifier_fore;
	gint c_identifier_size;
	gboolean c_identifier_bold;
	gboolean c_identifier_italic;
	gint c_comment_back;
	gchar *c_comment_font;
	gint c_comment_fore;
	gint c_comment_size;
	gboolean c_comment_bold;
	gboolean c_comment_italic;
	gint c_commentline_back;
	gchar *c_commentline_font;
	gint c_commentline_fore;
	gint c_commentline_size;
	gboolean c_commentline_bold;
	gboolean c_commentline_italic;
	gint c_preprocesor_back;
	gchar *c_preprocesor_font;
	gint c_preprocesor_fore;
	gint c_preprocesor_size;
	gboolean c_preprocesor_bold;
	gboolean c_preprocesor_italic;
	gint c_operator_back;
	gchar *c_operator_font;
	gint c_operator_fore;
	gint c_operator_size;
	gboolean c_operator_bold;
	gboolean c_operator_italic;
	gint c_regex_back;
	gchar *c_regex_font;
	gint c_regex_fore;
	gint c_regex_size;
	gboolean c_regex_bold;
	gboolean c_regex_italic;
	gint c_uuid_back;
	gchar *c_uuid_font;
	gint c_uuid_fore;
	gint c_uuid_size;
	gboolean c_uuid_bold;
	gboolean c_uuid_italic;
	gint c_verbatim_back;
	gchar *c_verbatim_font;
	gint c_verbatim_fore;
	gint c_verbatim_size;
	gboolean c_verbatim_bold;
	gboolean c_verbatim_italic; 
    	gint c_globalclass_back;
	gchar *c_globalclass_font;
	gint c_globalclass_fore;
	gint c_globalclass_size;
	gboolean c_globalclass_bold;
	gboolean c_globalclass_italic;
}
Preferences;


extern Preferences preferences;

void preferences_load(void);
void preferences_save(void);
//void preferences_show(void);
void main_window_size_save_details(void);

void move_classbrowser_position(void);
void save_classbrowser_position(void);
void preferences_apply(void);

#endif
