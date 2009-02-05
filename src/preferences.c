/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries
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


#include "preferences.h"
#include "main_window.h"
#include "main.h"

Preferences preferences;


void check_for_pango_fonts(void)
{
	GtkWidget *dialog;

	// This is crap - it should use a nice function and pass in a pointer to each font element for freeing and resetting
	// but it's been far too long since I used pointers that in-depth - Andy.

	if (strstr(preferences.default_font, "*") != NULL) {	
		dialog = gtk_message_dialog_new (NULL,
                                  GTK_DIALOG_MODAL,
				  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_CLOSE,
                                  _("Your old settings used non-Pango (modern) fonts.  gPHPEdit no longer supports old fonts, your preferences have been updated."));
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		// Only need to set the font names, the colours will be the same, and in 
		// previous versions size, italic and bold weren't set so they will have
		// been set using gnome_config_get*
		
		preferences.default_font = "!Sans";
		preferences.line_number_font = "!Sans";
		preferences.html_tag_font = "!Sans";
		preferences.html_tag_unknown_font = "!Sans";
		preferences.html_attribute_font = "!Sans";
		preferences.html_attribute_unknown_font = "!Sans";
		preferences.html_number_font = "!Sans";
		preferences.html_single_string_font = "!Sans";
		preferences.html_double_string_font = "!Sans";
		preferences.html_comment_font = "!Sans";
		preferences.html_entity_font = "!Sans";
		preferences.html_script_font = "!Sans";
		preferences.html_question_font = "!Sans";
		preferences.html_value_font = "!Sans";
		preferences.javascript_comment_font = "!Sans";
		preferences.javascript_comment_line_font = "!Sans";
		preferences.javascript_comment_doc_font = "!Sans";
		preferences.javascript_word_font = "!Sans";
		preferences.javascript_keyword_font = "!Sans";
		preferences.javascript_doublestring_font = "!Sans";
		preferences.javascript_singlestring_font = "!Sans";
		preferences.javascript_symbols_font = "!Sans";
		preferences.php_default_font = "!Sans";
		preferences.php_hstring_font = "!Sans";
		preferences.php_simplestring_font = "!Sans";
		preferences.php_word_font = "!Sans";
		preferences.php_number_font = "!Sans";
		preferences.php_variable_font = "!Sans";
		preferences.php_comment_font = "!Sans";
		preferences.php_comment_line_font = "!Sans";
		preferences.css_tag_font = "!Sans";
		preferences.css_class_font = "!Sans";
		preferences.css_pseudoclass_font = "!Sans";
		preferences.css_unknown_pseudoclass_font = "!Sans";
		preferences.css_operator_font = "!Sans";
		preferences.css_identifier_font = "!Sans";
		preferences.css_unknown_identifier_font = "!Sans";
		preferences.css_value_font = "!Sans";
		preferences.css_comment_font = "!Sans";
		preferences.css_id_font = "!Sans";
		preferences.css_important_font = "!Sans";
		preferences.css_directive_font = "!Sans";
		preferences.sql_word_font = "!Sans";
		preferences.sql_string_font = "!Sans";
		preferences.sql_operator_font = "!Sans";
		preferences.sql_comment_font = "!Sans";
		preferences.sql_number_font = "!Sans";
		preferences.sql_identifier_font = "!Sans";
		
		preferences_save();
	}
}


void preferences_apply(void)
{
	preferences.left   = gnome_config_get_int ("gPHPEdit/main_window/x=20");
	preferences.top    = gnome_config_get_int ("gPHPEdit/main_window/y=20");
	preferences.width  = gnome_config_get_int ("gPHPEdit/main_window/width=400");
	preferences.height = gnome_config_get_int ("gPHPEdit/main_window/height=400");
	preferences.maximized = gnome_config_get_bool ("gPHPEdit/main_window/maximized=false");
 
	gtk_window_move(GTK_WINDOW(main_window.window), preferences.left, preferences.top);
	gtk_window_set_default_size(GTK_WINDOW(main_window.window), preferences.width, preferences.height);
 	
	if (preferences.maximized) {
 		gtk_window_maximize(GTK_WINDOW(main_window.window));
	}
}

void preferences_load(void)
{
	gboolean ignore;

	preferences.set_sel_back = gnome_config_get_int ("gPHPEdit/default_style/selection=11250603");
	preferences.marker_back = gnome_config_get_int ("gPHPEdit/default_style/bookmark=15908608");
	preferences.php_binary_location = gnome_config_get_string ("gPHPEdit/locations/php_binary=php");
	preferences.shared_source_location = gnome_config_get_string ("gPHPEdit/locations/shared_source=");
	preferences.indentation_size = gnome_config_get_int ("gPHPEdit/defaults/indentationsize=4");
	preferences.tab_size = gnome_config_get_int ("gPHPEdit/defaults/tabsize=4");
	preferences.auto_complete_delay = gnome_config_get_int ("gPHPEdit/defaults/auto_complete_delay=500");
	preferences.calltip_delay = gnome_config_get_int ("gPHPEdit/defaults/calltip_delay=500");
	preferences.show_indentation_guides = gnome_config_get_int ("gPHPEdit/defaults/showindentationguides=1");
	preferences.show_folding = gnome_config_get_int ("gPHPEdit/defaults/showfolding=1");
	preferences.edge_mode = gnome_config_get_int ("gPHPEdit/defaults/edgemode=1");
	preferences.edge_column = gnome_config_get_int ("gPHPEdit/defaults/edgecolumn=80");
	preferences.edge_colour = gnome_config_get_int ("gPHPEdit/defaults/edgecolour=8355712");
	preferences.line_wrapping = gnome_config_get_int ("gPHPEdit/defaults/linewrapping=1");
	//preferences.auto_indent_after_brace = gnome_config_get_int ("gPHPEdit/defaults/autoindentafterbrace=1");
	preferences.save_session = gnome_config_get_bool_with_default("gPHPEdit/defaults/save_session=true", &ignore);
	preferences.use_tabs_instead_spaces = gnome_config_get_bool_with_default ("gPHPEdit/defaults/use_tabs_instead_spaces=true", &ignore);
	preferences.single_instance_only = gnome_config_get_bool_with_default ("gPHPEdit/defaults/single_instance_only=false", &ignore);
	
	preferences.php_file_extensions = gnome_config_get_string("gPHPEdit/defaults/php_file_extensions=php,inc,phtml,php3,xml,htm,html");
	
	preferences.default_font = gnome_config_get_string ("gPHPEdit/default_style/font=!Sans");
	preferences.default_fore = gnome_config_get_int ("gPHPEdit/default_style/fore=0");
	preferences.default_back = gnome_config_get_int ("gPHPEdit/default_style/back=16777215");
	preferences.default_size = gnome_config_get_int ("gPHPEdit/default_style/size=12");
	preferences.default_bold = gnome_config_get_bool_with_default("gPHPEdit/default_style/bold=false", &ignore);
	preferences.default_italic = gnome_config_get_bool_with_default("gPHPEdit/default_style/italic=false", &ignore);
	
	preferences.line_number_font = gnome_config_get_string ("gPHPEdit/line_numbers/font=!Sans");
	preferences.line_number_fore = gnome_config_get_int ("gPHPEdit/line_numbers/fore=0");
	preferences.line_number_back = gnome_config_get_int ("gPHPEdit/line_numbers/back=11053224");
	preferences.line_number_size = gnome_config_get_int ("gPHPEdit/line_numbers/size=12");
	preferences.line_number_bold = gnome_config_get_bool_with_default("gPHPEdit/line_numbers/bold=false", &ignore);
	preferences.line_number_italic = gnome_config_get_bool_with_default("gPHPEdit/line_numbers/italic=false", &ignore);

	preferences.html_tag_back = gnome_config_get_int ("gPHPEdit/html_tag/back=16777215");
	preferences.html_tag_fore = gnome_config_get_int ("gPHPEdit/html_tag/fore=7553164");
	preferences.html_tag_font = gnome_config_get_string ("gPHPEdit/html_tag/font=!Sans");
	preferences.html_tag_size = gnome_config_get_int ("gPHPEdit/html_tag/size=12");
	preferences.html_tag_bold = gnome_config_get_bool_with_default("gPHPEdit/html_tag/bold=true", &ignore);
	preferences.html_tag_italic = gnome_config_get_bool_with_default("gPHPEdit/html_tag/italic=false", &ignore);

	preferences.html_tag_unknown_back = gnome_config_get_int ("gPHPEdit/html_tag_unknown/back=16777215");
	preferences.html_tag_unknown_fore = gnome_config_get_int ("gPHPEdit/html_tag_unknown/fore=7553164");
	preferences.html_tag_unknown_font = gnome_config_get_string ("gPHPEdit/html_tag_unknown/font=!Sans");
	preferences.html_tag_unknown_size = gnome_config_get_int ("gPHPEdit/html_tag_unknown/size=12");
	preferences.html_tag_unknown_bold = gnome_config_get_bool_with_default("gPHPEdit/html_tag_unknown/bold=true", &ignore);
	preferences.html_tag_unknown_italic = gnome_config_get_bool_with_default("gPHPEdit/html_tag_unknown/italic=false", &ignore);

	preferences.html_attribute_back = gnome_config_get_int ("gPHPEdit/html_attribute/back=16777215");
	preferences.html_attribute_fore = gnome_config_get_int ("gPHPEdit/html_attribute/fore=9204544");
	preferences.html_attribute_font = gnome_config_get_string ("gPHPEdit/html_attribute/font=!Sans");
	preferences.html_attribute_size = gnome_config_get_int ("gPHPEdit/html_attribute/size=12");
	preferences.html_attribute_bold = gnome_config_get_bool_with_default("gPHPEdit/html_attribute/bold=true", &ignore);
	preferences.html_attribute_italic = gnome_config_get_bool_with_default("gPHPEdit/html_attribute/italic=false", &ignore);

	preferences.html_attribute_unknown_back = gnome_config_get_int ("gPHPEdit/html_attribute_unknown/back=16777215");
	preferences.html_attribute_unknown_fore = gnome_config_get_int ("gPHPEdit/html_attribute_unknown/fore=7472544");
	preferences.html_attribute_unknown_font = gnome_config_get_string ("gPHPEdit/html_attribute_unknown/font=!Sans");
	preferences.html_attribute_unknown_size = gnome_config_get_int ("gPHPEdit/html_attribute_unknown/size=12");
	preferences.html_attribute_unknown_bold = gnome_config_get_bool_with_default("gPHPEdit/html_attribute_unknown/bold=true", &ignore);
	preferences.html_attribute_unknown_italic = gnome_config_get_bool_with_default("gPHPEdit/html_attribute_unknown/italic=false", &ignore);

	preferences.html_number_back = gnome_config_get_int ("gPHPEdit/html_number/back=16777215");
	preferences.html_number_fore = gnome_config_get_int ("gPHPEdit/html_number/fore=9204544");
	preferences.html_number_font = gnome_config_get_string ("gPHPEdit/html_number/font=!Sans");
	preferences.html_number_size = gnome_config_get_int ("gPHPEdit/html_number/size=12");
	preferences.html_number_bold = gnome_config_get_bool_with_default("gPHPEdit/html_number/bold=false", &ignore);
	preferences.html_number_italic = gnome_config_get_bool_with_default("gPHPEdit/html_number/italic=false", &ignore);

	preferences.html_single_string_back = gnome_config_get_int ("gPHPEdit/html_single_string/back=16777215");
	preferences.html_single_string_fore = gnome_config_get_int ("gPHPEdit/html_single_string/fore=32768");
	preferences.html_single_string_font = gnome_config_get_string ("gPHPEdit/html_single_string/font=!Sans");
	preferences.html_single_string_size = gnome_config_get_int ("gPHPEdit/html_single_string/size=12");
	preferences.html_single_string_bold = gnome_config_get_bool_with_default("gPHPEdit/html_single_string/bold=false", &ignore);
	preferences.html_single_string_italic = gnome_config_get_bool_with_default("gPHPEdit/html_single_string/italic=false", &ignore);

	preferences.html_double_string_back = gnome_config_get_int ("gPHPEdit/html_double_string/back=16777215");
	preferences.html_double_string_fore = gnome_config_get_int ("gPHPEdit/html_double_string/fore=32768");
	preferences.html_double_string_font = gnome_config_get_string ("gPHPEdit/html_double_string/font=!Sans");
	preferences.html_double_string_size = gnome_config_get_int ("gPHPEdit/html_double_string/size=12");
	preferences.html_double_string_bold = gnome_config_get_bool_with_default("gPHPEdit/html_double_string/bold=false", &ignore);
	preferences.html_double_string_italic = gnome_config_get_bool_with_default("gPHPEdit/html_double_string/italic=false", &ignore);

	preferences.html_comment_back = gnome_config_get_int ("gPHPEdit/html_comment/back=16777215");
	preferences.html_comment_fore = gnome_config_get_int ("gPHPEdit/html_comment/fore=8421504");
	preferences.html_comment_font = gnome_config_get_string ("gPHPEdit/html_comment/font=!Sans");
	preferences.html_comment_size = gnome_config_get_int ("gPHPEdit/html_comment/size=12");
	preferences.html_comment_bold = gnome_config_get_bool_with_default("gPHPEdit/html_comment/bold=false", &ignore);
	preferences.html_comment_italic = gnome_config_get_bool_with_default("gPHPEdit/html_comment/italic=false", &ignore);

	preferences.html_entity_back = gnome_config_get_int ("gPHPEdit/html_entity/back=16777215");
	preferences.html_entity_fore = gnome_config_get_int ("gPHPEdit/html_entity/fore=8421504");
	preferences.html_entity_font = gnome_config_get_string ("gPHPEdit/html_entity/font=!Sans");
	preferences.html_entity_size = gnome_config_get_int ("gPHPEdit/html_entity/size=12");
	preferences.html_entity_bold = gnome_config_get_bool_with_default("gPHPEdit/html_entity/bold=false", &ignore);
	preferences.html_entity_italic = gnome_config_get_bool_with_default("gPHPEdit/html_entity/italic=false", &ignore);

	preferences.html_script_back = gnome_config_get_int ("gPHPEdit/html_script/back=16777215");
	preferences.html_script_fore = gnome_config_get_int ("gPHPEdit/html_script/fore=7553165");
	preferences.html_script_font = gnome_config_get_string ("gPHPEdit/html_script/font=!Sans");
	preferences.html_script_size = gnome_config_get_int ("gPHPEdit/html_script/size=12");
	preferences.html_script_bold = gnome_config_get_bool_with_default("gPHPEdit/html_script/bold=false", &ignore);
	preferences.html_script_italic = gnome_config_get_bool_with_default("gPHPEdit/html_script/italic=false", &ignore);

	preferences.html_question_back = gnome_config_get_int ("gPHPEdit/html_question/back=16777215");
	preferences.html_question_fore = gnome_config_get_int ("gPHPEdit/html_question/fore=7553165");
	preferences.html_question_font = gnome_config_get_string ("gPHPEdit/html_question/font=!Sans");
	preferences.html_question_size = gnome_config_get_int ("gPHPEdit/html_question/size=12");
	preferences.html_question_bold = gnome_config_get_bool_with_default("gPHPEdit/html_question/bold=false", &ignore);
	preferences.html_question_italic = gnome_config_get_bool_with_default("gPHPEdit/html_question/italic=false", &ignore);

	preferences.html_value_back = gnome_config_get_int ("gPHPEdit/html_value/back=16777215");
	preferences.html_value_fore = gnome_config_get_int ("gPHPEdit/html_value/fore=21632");
	preferences.html_value_font = gnome_config_get_string ("gPHPEdit/html_value/font=!Sans");
	preferences.html_value_size = gnome_config_get_int ("gPHPEdit/html_value/size=12");
	preferences.html_value_bold = gnome_config_get_bool_with_default("gPHPEdit/html_value/bold=false", &ignore);
	preferences.html_value_italic = gnome_config_get_bool_with_default("gPHPEdit/html_value/italic=false", &ignore);

	preferences.javascript_comment_back = gnome_config_get_int ("gPHPEdit/javascript_comment/back=16777215");
	preferences.javascript_comment_fore = gnome_config_get_int ("gPHPEdit/javascript_comment/fore=8421504");
	preferences.javascript_comment_font = gnome_config_get_string ("gPHPEdit/javascript_comment/font=!Sans");
	preferences.javascript_comment_size = gnome_config_get_int ("gPHPEdit/javascript_comment/size=12");
	preferences.javascript_comment_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment/bold=false", &ignore);
	preferences.javascript_comment_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment/italic=false", &ignore);

	preferences.javascript_comment_line_back = gnome_config_get_int ("gPHPEdit/javascript_comment_line/back=16777215");
	preferences.javascript_comment_line_fore = gnome_config_get_int ("gPHPEdit/javascript_comment_line/fore=8421504");
	preferences.javascript_comment_line_font = gnome_config_get_string ("gPHPEdit/javascript_comment_line/font=!Sans");
	preferences.javascript_comment_line_size = gnome_config_get_int ("gPHPEdit/javascript_comment_line/size=12");
	preferences.javascript_comment_line_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment_line/bold=false", &ignore);
	preferences.javascript_comment_line_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment_line/italic=false", &ignore);

	preferences.javascript_comment_doc_back = gnome_config_get_int ("gPHPEdit/javascript_comment_doc/back=16777215");
	preferences.javascript_comment_doc_fore = gnome_config_get_int ("gPHPEdit/javascript_comment_doc/fore=8355712");
	preferences.javascript_comment_doc_font = gnome_config_get_string ("gPHPEdit/javascript_comment_doc/font=!Sans");
	preferences.javascript_comment_doc_size = gnome_config_get_int ("gPHPEdit/javascript_comment_doc/size=12");
	preferences.javascript_comment_doc_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment_doc/bold=false", &ignore);
	preferences.javascript_comment_doc_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_comment_doc/italic=false", &ignore);

	preferences.javascript_word_back = gnome_config_get_int ("gPHPEdit/javascript_word/back=16777215");
	preferences.javascript_word_fore = gnome_config_get_int ("gPHPEdit/javascript_word/fore=9204544");
	preferences.javascript_word_font = gnome_config_get_string ("gPHPEdit/javascript_word/font=!Sans");
	preferences.javascript_word_size = gnome_config_get_int ("gPHPEdit/javascript_word/size=12");
	preferences.javascript_word_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_word/bold=false", &ignore);
	preferences.javascript_word_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_word/italic=false", &ignore);

	preferences.javascript_keyword_back = gnome_config_get_int ("gPHPEdit/javascript_keyword/back=16777215");
	preferences.javascript_keyword_fore = gnome_config_get_int ("gPHPEdit/javascript_keyword/fore=8388608");
	preferences.javascript_keyword_font = gnome_config_get_string ("gPHPEdit/javascript_keyword/font=!Sans");
	preferences.javascript_keyword_size = gnome_config_get_int ("gPHPEdit/javascript_keyword/size=12");
	preferences.javascript_keyword_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_keyword/bold=true", &ignore);
	preferences.javascript_keyword_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_keyword/italic=false", &ignore);

	preferences.javascript_doublestring_back = gnome_config_get_int ("gPHPEdit/javascript_doublestring/back=16777215");
	preferences.javascript_doublestring_fore = gnome_config_get_int ("gPHPEdit/javascript_doublestring/fore=8388608");
	preferences.javascript_doublestring_font = gnome_config_get_string ("gPHPEdit/javascript_doublestring/font=!Sans");
	preferences.javascript_doublestring_size = gnome_config_get_int ("gPHPEdit/javascript_doublestring/size=12");
	preferences.javascript_doublestring_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_doublestring/bold=false", &ignore);
	preferences.javascript_doublestring_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_doublestring/italic=false", &ignore);

	preferences.javascript_singlestring_back = gnome_config_get_int ("gPHPEdit/javascript_singlestring/back=16777215");
	preferences.javascript_singlestring_fore = gnome_config_get_int ("gPHPEdit/javascript_singlestring/fore=8388608");
	preferences.javascript_singlestring_font = gnome_config_get_string ("gPHPEdit/javascript_singlestring/font=!Sans");
	preferences.javascript_singlestring_size = gnome_config_get_int ("gPHPEdit/javascript_singlestring/size=12");
	preferences.javascript_singlestring_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_singlestring/bold=false", &ignore);
	preferences.javascript_singlestring_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_singlestring/it=false", &ignore);

	preferences.javascript_symbols_back = gnome_config_get_int ("gPHPEdit/javascript_symbols/back=16777215");
	preferences.javascript_symbols_fore = gnome_config_get_int ("gPHPEdit/javascript_symbols/fore=8355712");
	preferences.javascript_symbols_font = gnome_config_get_string ("gPHPEdit/javascript_symbols/font=!Sans");
	preferences.javascript_symbols_size = gnome_config_get_int ("gPHPEdit/javascript_symbols/size=12");
	preferences.javascript_symbols_bold = gnome_config_get_bool_with_default("gPHPEdit/javascript_symbols/bold=true", &ignore);
	preferences.javascript_symbols_italic = gnome_config_get_bool_with_default("gPHPEdit/javascript_symbols/italic=false", &ignore);

	preferences.php_default_font = gnome_config_get_string ("gPHPEdit/php_default_style/font=!Sans");
	preferences.php_default_fore = gnome_config_get_int ("gPHPEdit/php_default_style/fore=1052688");
	preferences.php_default_back = gnome_config_get_int ("gPHPEdit/php_default_style/back=16777215");
	preferences.php_default_size = gnome_config_get_int ("gPHPEdit/php_default_style/size=12");
	preferences.php_default_bold = gnome_config_get_bool_with_default("gPHPEdit/php_default_style/bold=false", &ignore);
	preferences.php_default_italic = gnome_config_get_bool_with_default("gPHPEdit/php_default_style/italic=false", &ignore);

	preferences.php_hstring_fore = gnome_config_get_int ("gPHPEdit/php_hstring/fore=8388736");
	preferences.php_hstring_font = gnome_config_get_string ("gPHPEdit/php_hstring/font=!Sans");
	preferences.php_hstring_back = gnome_config_get_int ("gPHPEdit/php_hstring/back=16777215");
	preferences.php_hstring_size = gnome_config_get_int ("gPHPEdit/php_hstring/size=12");
	preferences.php_hstring_bold = gnome_config_get_bool_with_default("gPHPEdit/php_hstring/bold=false", &ignore);
	preferences.php_hstring_italic = gnome_config_get_bool_with_default("gPHPEdit/php_hstring/italic=false", &ignore);

	preferences.php_simplestring_fore = gnome_config_get_int ("gPHPEdit/php_simplestring/fore=8388736");
	preferences.php_simplestring_font = gnome_config_get_string ("gPHPEdit/php_simplestring/font=!Sans");
	preferences.php_simplestring_back = gnome_config_get_int ("gPHPEdit/php_simplestring/back=16777215");
	preferences.php_simplestring_size = gnome_config_get_int ("gPHPEdit/php_simplestring/size=12");
	preferences.php_simplestring_bold = gnome_config_get_bool_with_default("gPHPEdit/php_simplestring/bold=false", &ignore);
	preferences.php_simplestring_italic = gnome_config_get_bool_with_default("gPHPEdit/php_simplestring/italic=false", &ignore);

	preferences.php_word_fore = gnome_config_get_int ("gPHPEdit/php_word/fore=0");
	preferences.php_word_font = gnome_config_get_string ("gPHPEdit/php_word/font=!Sans");
	preferences.php_word_back = gnome_config_get_int ("gPHPEdit/php_word/back=16777215");
	preferences.php_word_size = gnome_config_get_int ("gPHPEdit/php_word/size=12");
	preferences.php_word_bold = gnome_config_get_bool_with_default("gPHPEdit/php_word/bold=true", &ignore);
	preferences.php_word_italic = gnome_config_get_bool_with_default("gPHPEdit/php_word/italic=false", &ignore);

	preferences.php_number_fore = gnome_config_get_int ("gPHPEdit/php_number/fore=9204544");
	preferences.php_number_font = gnome_config_get_string ("gPHPEdit/php_number/font=!Sans");
	preferences.php_number_back = gnome_config_get_int ("gPHPEdit/php_number/back=16777215");
	preferences.php_number_size = gnome_config_get_int ("gPHPEdit/php_number/size=12");
	preferences.php_number_bold = gnome_config_get_bool_with_default("gPHPEdit/php_number/bold=false", &ignore);
	preferences.php_number_italic = gnome_config_get_bool_with_default("gPHPEdit/php_number/italic=false", &ignore);

	preferences.php_variable_fore = gnome_config_get_int ("gPHPEdit/php_variable/fore=16746496");
	preferences.php_variable_font = gnome_config_get_string ("gPHPEdit/php_variable/font=!Sans");
	preferences.php_variable_back = gnome_config_get_int ("gPHPEdit/php_variable/back=16777215");
	preferences.php_variable_size = gnome_config_get_int ("gPHPEdit/php_variable/size=12");
	preferences.php_variable_bold = gnome_config_get_bool_with_default("gPHPEdit/php_variable/bold=false", &ignore);
	preferences.php_variable_italic = gnome_config_get_bool_with_default("gPHPEdit/php_variable/italic=false", &ignore);

	preferences.php_comment_fore = gnome_config_get_int ("gPHPEdit/php_comment/fore=8421504");
	preferences.php_comment_font = gnome_config_get_string ("gPHPEdit/php_comment/font=!Sans");
	preferences.php_comment_back = gnome_config_get_int ("gPHPEdit/php_comment/back=16777215");
	preferences.php_comment_size = gnome_config_get_int ("gPHPEdit/php_comment/size=12");
	preferences.php_comment_bold = gnome_config_get_bool_with_default("gPHPEdit/php_comment/bold=false", &ignore);
	preferences.php_comment_italic = gnome_config_get_bool_with_default("gPHPEdit/php_comment/italic=true", &ignore);

	preferences.php_comment_line_fore = gnome_config_get_int ("gPHPEdit/php_comment_line/fore=8421504");
	preferences.php_comment_line_font = gnome_config_get_string ("gPHPEdit/php_comment_line/font=!Sans");
	preferences.php_comment_line_back = gnome_config_get_int ("gPHPEdit/php_comment_line/back=16777215");
	preferences.php_comment_line_size = gnome_config_get_int ("gPHPEdit/php_comment_line/size=12");
	preferences.php_comment_line_bold = gnome_config_get_bool_with_default("gPHPEdit/php_comment_line/bold=false", &ignore);
	preferences.php_comment_line_italic = gnome_config_get_bool_with_default("gPHPEdit/php_comment_line/italic=true", &ignore);

	preferences.css_tag_back = gnome_config_get_int ("gPHPEdit/css_tag/back=16777215");
	preferences.css_tag_font = gnome_config_get_string ("gPHPEdit/css_tag/font=!Sans");
	preferences.css_tag_fore = gnome_config_get_int ("gPHPEdit/css_tag/fore=8388608");
	preferences.css_tag_size = gnome_config_get_int ("gPHPEdit/css_tag/size=12");
	preferences.css_tag_bold = gnome_config_get_bool_with_default("gPHPEdit/css_tag/bold=true", &ignore);
	preferences.css_tag_italic = gnome_config_get_bool_with_default("gPHPEdit/css_tag/italic=false", &ignore);
	
	preferences.css_class_fore = gnome_config_get_int ("gPHPEdit/css_class/fore=8388608");
	preferences.css_class_font = gnome_config_get_string ("gPHPEdit/css_class/font=!Sans");
	preferences.css_class_back = gnome_config_get_int ("gPHPEdit/css_class/back=16777215");
	preferences.css_class_size = gnome_config_get_int ("gPHPEdit/css_class/size=12");
	preferences.css_class_bold = gnome_config_get_bool_with_default("gPHPEdit/css_class/bold=false", &ignore);
	preferences.css_class_italic = gnome_config_get_bool_with_default("gPHPEdit/css_class/italic=false", &ignore);
	
	preferences.css_pseudoclass_back = gnome_config_get_int ("gPHPEdit/css_pseudoclass/back=16777215");
	preferences.css_pseudoclass_font = gnome_config_get_string ("gPHPEdit/css_pseudoclass/font=!Sans");
	preferences.css_pseudoclass_fore = gnome_config_get_int ("gPHPEdit/css_pseudoclass/fore=8388608");
	preferences.css_pseudoclass_size = gnome_config_get_int ("gPHPEdit/css_pseudoclass/size=12");
	preferences.css_pseudoclass_bold = gnome_config_get_bool_with_default("gPHPEdit/css_pseudoclass/bold=false", &ignore);
	preferences.css_pseudoclass_italic = gnome_config_get_bool_with_default("gPHPEdit/css_pseudoclass/italic=false", &ignore);
	
	preferences.css_unknown_pseudoclass_fore = gnome_config_get_int ("gPHPEdit/css_unknown_pseudoclass/fore=16711680");
	preferences.css_unknown_pseudoclass_font = gnome_config_get_string ("gPHPEdit/css_unknown_pseudoclass/font=!Sans");
	preferences.css_unknown_pseudoclass_back = gnome_config_get_int ("gPHPEdit/css_unknown_pseudoclass/back=16777215");
	preferences.css_unknown_pseudoclass_size = gnome_config_get_int ("gPHPEdit/css_unknown_pseudoclass/size=12");
	preferences.css_unknown_pseudoclass_bold = gnome_config_get_bool_with_default("gPHPEdit/css_unknown_pseudoclass/bold=false", &ignore);
	preferences.css_unknown_pseudoclass_italic = gnome_config_get_bool_with_default("gPHPEdit/css_unknown_pseudoclass/italic=false", &ignore);
	
	preferences.css_operator_fore = gnome_config_get_int ("gPHPEdit/css_operator/fore=128");
	preferences.css_operator_font = gnome_config_get_string ("gPHPEdit/css_operator/font=!Sans");
	preferences.css_operator_back = gnome_config_get_int ("gPHPEdit/css_operator/back=16777215");
	preferences.css_operator_size = gnome_config_get_int ("gPHPEdit/css_operator/size=12");
	preferences.css_operator_bold = gnome_config_get_bool_with_default("gPHPEdit/css_operator/bold=false", &ignore);
	preferences.css_operator_italic = gnome_config_get_bool_with_default("gPHPEdit/css_operator/italic=false", &ignore);
	
	preferences.css_identifier_fore = gnome_config_get_int ("gPHPEdit/css_identifier/fore=0");
	preferences.css_identifier_font = gnome_config_get_string ("gPHPEdit/css_identifier/font=!Sans");
	preferences.css_identifier_back = gnome_config_get_int ("gPHPEdit/css_identifier/back=16777215");
	preferences.css_identifier_size = gnome_config_get_int ("gPHPEdit/css_identifier/size=12");
	preferences.css_identifier_bold = gnome_config_get_bool_with_default("gPHPEdit/css_identifier/bold=false", &ignore);
	preferences.css_identifier_italic = gnome_config_get_bool_with_default("gPHPEdit/css_identifier/italic=false", &ignore);
	
	preferences.css_unknown_identifier_fore = gnome_config_get_int ("gPHPEdit/css_unknown_identifier/fore=16711680");
	preferences.css_unknown_identifier_font = gnome_config_get_string ("gPHPEdit/css_unknown_identifier/font=!Sans");
	preferences.css_unknown_identifier_back = gnome_config_get_int ("gPHPEdit/css_unknown_identifier/back=16777215");
	preferences.css_unknown_identifier_size = gnome_config_get_int ("gPHPEdit/css_unknown_identifier/size=12");
	preferences.css_unknown_identifier_bold = gnome_config_get_bool_with_default("gPHPEdit/css_unknown_identifier/bold=false", &ignore);
	preferences.css_unknown_identifier_italic = gnome_config_get_bool_with_default("gPHPEdit/css_unknown_identifier/italic=false", &ignore);
	
	preferences.css_value_fore = gnome_config_get_int ("gPHPEdit/css_value/fore=8388736");
	preferences.css_value_font = gnome_config_get_string ("gPHPEdit/css_value/font=!Sans");
	preferences.css_value_back = gnome_config_get_int ("gPHPEdit/css_value/back=16777215");
	preferences.css_value_size = gnome_config_get_int ("gPHPEdit/css_value/size=12");
	preferences.css_value_bold = gnome_config_get_bool_with_default("gPHPEdit/css_value/bold=false", &ignore);
	preferences.css_value_italic = gnome_config_get_bool_with_default("gPHPEdit/css_value/italic=false", &ignore);
	
	preferences.css_comment_fore = gnome_config_get_int ("gPHPEdit/css_comment/fore=8421504");
	preferences.css_comment_font = gnome_config_get_string ("gPHPEdit/css_comment/font=!Sans");
	preferences.css_comment_back = gnome_config_get_int ("gPHPEdit/css_comment/back=16777215");
	preferences.css_comment_size = gnome_config_get_int ("gPHPEdit/css_comment/size=12");
	preferences.css_comment_bold = gnome_config_get_bool_with_default("gPHPEdit/css_comment/bold=false", &ignore);
	preferences.css_comment_italic = gnome_config_get_bool_with_default("gPHPEdit/css_comment/italic=true", &ignore);
	
	preferences.css_id_font = gnome_config_get_string ("gPHPEdit/css_id/fore=8388608");
	preferences.css_id_fore = gnome_config_get_int ("gPHPEdit/css_id/font=!Sans");
	preferences.css_id_back = gnome_config_get_int ("gPHPEdit/css_id/back=16777215");
	preferences.css_id_size = gnome_config_get_int ("gPHPEdit/css_id/size=12");
	preferences.css_id_bold = gnome_config_get_bool_with_default("gPHPEdit/css_id/bold=false", &ignore);
	preferences.css_id_italic = gnome_config_get_bool_with_default("gPHPEdit/css_id/italic=false", &ignore);
	
	preferences.css_important_font = gnome_config_get_string ("gPHPEdit/css_important/fore=255");
	preferences.css_important_fore = gnome_config_get_int ("gPHPEdit/css_important/font=!Sans");
	preferences.css_important_back = gnome_config_get_int ("gPHPEdit/css_important/back=16777215");
	preferences.css_important_size = gnome_config_get_int ("gPHPEdit/css_important/size=12");
	preferences.css_important_bold = gnome_config_get_bool_with_default("gPHPEdit/css_important/bold=false", &ignore);
	preferences.css_important_italic = gnome_config_get_bool_with_default("gPHPEdit/css_important/italic=false", &ignore);
	
	preferences.css_directive_fore = gnome_config_get_int ("gPHPEdit/css_directive/fore=32768");
	preferences.css_directive_font = gnome_config_get_string ("gPHPEdit/css_directive/font=!Sans");
	preferences.css_directive_back = gnome_config_get_int ("gPHPEdit/css_directive/back=16777215");
	preferences.css_directive_size = gnome_config_get_int ("gPHPEdit/css_directive/size=12");
	preferences.css_directive_bold = gnome_config_get_bool_with_default("gPHPEdit/css_directive/bold=true", &ignore);
	preferences.css_directive_italic = gnome_config_get_bool_with_default("gPHPEdit/css_directive/italic=false", &ignore);
	
	preferences.sql_word_fore = gnome_config_get_int ("gPHPEdit/sql_word/fore=0");
	preferences.sql_word_font = gnome_config_get_string ("gPHPEdit/sql_word/font=!Sans");
	preferences.sql_word_back = gnome_config_get_int ("gPHPEdit/sql_word/back=16777215");
	preferences.sql_word_size = gnome_config_get_int ("gPHPEdit/sql_word/size=12");
	preferences.sql_word_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_word/bold=true", &ignore);
	preferences.sql_word_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_word/italic=false", &ignore);

	preferences.sql_string_fore = gnome_config_get_int ("gPHPEdit/sql_string/fore=8388736");
	preferences.sql_string_font = gnome_config_get_string ("gPHPEdit/sql_string/font=!Sans");
	preferences.sql_string_back = gnome_config_get_int ("gPHPEdit/sql_string/back=16777215");
	preferences.sql_string_size = gnome_config_get_int ("gPHPEdit/sql_string/size=12");
	preferences.sql_string_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_string/bold=false", &ignore);
	preferences.sql_string_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_string/italic=false", &ignore);
	
	preferences.sql_operator_fore = gnome_config_get_int ("gPHPEdit/sql_operator/fore=0");
	preferences.sql_operator_font = gnome_config_get_string ("gPHPEdit/sql_operator/font=!Sans");
	preferences.sql_operator_back = gnome_config_get_int ("gPHPEdit/sql_operator/back=16777215");
	preferences.sql_operator_size = gnome_config_get_int ("gPHPEdit/sql_operator/size=12");
	preferences.sql_operator_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_operator/bold=false", &ignore);
	preferences.sql_operator_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_operator/italic=false", &ignore);
	
	preferences.sql_comment_fore = gnome_config_get_int ("gPHPEdit/sql_comment/fore=8421504");
	preferences.sql_comment_font = gnome_config_get_string ("gPHPEdit/sql_comment/font=!Sans");
	preferences.sql_comment_back = gnome_config_get_int ("gPHPEdit/sql_comment/back=16777215");
	preferences.sql_comment_size = gnome_config_get_int ("gPHPEdit/sql_comment/size=12");
	preferences.sql_comment_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_comment/bold=false", &ignore);
	preferences.sql_comment_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_comment/italic=true", &ignore);
	
	preferences.sql_number_fore = gnome_config_get_int ("gPHPEdit/sql_number/fore=9204544");
	preferences.sql_number_font = gnome_config_get_string ("gPHPEdit/sql_number/font=!Sans");
	preferences.sql_number_back = gnome_config_get_int ("gPHPEdit/sql_number/back=16777215");
	preferences.sql_number_size = gnome_config_get_int ("gPHPEdit/sql_number/size=12");
	preferences.sql_number_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_number/bold=false", &ignore);
	preferences.sql_number_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_number/italic=false", &ignore);
	
	preferences.sql_identifier_fore = gnome_config_get_int ("gPHPEdit/sql_identifier/fore=16746496");
	preferences.sql_identifier_font = gnome_config_get_string ("gPHPEdit/sql_identifier/font=!Sans");
	preferences.sql_identifier_back = gnome_config_get_int ("gPHPEdit/sql_identifier/back=16777215");
	preferences.sql_identifier_size = gnome_config_get_int ("gPHPEdit/sql_identifier/size=12");
	preferences.sql_identifier_bold = gnome_config_get_bool_with_default("gPHPEdit/sql_identifier/bold=false", &ignore);
	preferences.sql_identifier_italic = gnome_config_get_bool_with_default("gPHPEdit/sql_identifier/italic=false", &ignore);

	check_for_pango_fonts();
}


void move_classbrowser_position(void)
{
	gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),gnome_config_get_int ("gPHPEdit/main_window/classbrowser_size=100"));
}

void save_classbrowser_position(void)
{
	if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)) != 0) {
		gnome_config_set_int("gPHPEdit/main_window/classbrowser_size", gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)));
		gnome_config_sync();
	}
}

void main_window_size_save_details()
{
	preferences.maximized = gnome_config_get_bool ("gPHPEdit/main_window/maximized=false");
	
	if (!preferences.maximized) {
		gtk_window_get_position(GTK_WINDOW(main_window.window), &preferences.left, &preferences.top);
		gtk_window_get_size(GTK_WINDOW(main_window.window), &preferences.width, &preferences.height);

		gnome_config_set_int ("gPHPEdit/main_window/x", preferences.left);
		gnome_config_set_int ("gPHPEdit/main_window/y", preferences.top);
		gnome_config_set_int ("gPHPEdit/main_window/width", preferences.width);
		gnome_config_set_int ("gPHPEdit/main_window/height", preferences.height);
	
		gnome_config_sync();
	}
}


void preferences_save(void)
{
	gnome_config_set_string ("gPHPEdit/default_style/font", preferences.default_font);
	gnome_config_set_int ("gPHPEdit/default_style/fore", preferences.default_fore);
	gnome_config_set_int ("gPHPEdit/default_style/back", preferences.default_back);
	gnome_config_set_int ("gPHPEdit/default_style/size", preferences.default_size);
	gnome_config_set_bool ("gPHPEdit/default_style/italic", preferences.default_italic);
	gnome_config_set_bool ("gPHPEdit/default_style/bold", preferences.default_bold);

	gnome_config_set_string ("gPHPEdit/line_numbers/font", preferences.line_number_font);
	gnome_config_set_int ("gPHPEdit/line_numbers/fore", preferences.line_number_fore);
	gnome_config_set_int ("gPHPEdit/line_numbers/back", preferences.line_number_back);
	gnome_config_set_int ("gPHPEdit/line_numbers/size", preferences.line_number_size);
	gnome_config_set_bool ("gPHPEdit/line_numbers/italic", preferences.line_number_italic);
	gnome_config_set_bool ("gPHPEdit/line_numbers/bold", preferences.line_number_bold);
	
	gnome_config_set_int ("gPHPEdit/default_style/selection", preferences.set_sel_back);
	gnome_config_set_int ("gPHPEdit/default_style/bookmark", preferences.marker_back);
	gnome_config_set_string ("gPHPEdit/locations/php_binary", preferences.php_binary_location);
	gnome_config_set_string ("gPHPEdit/locations/shared_source", preferences.shared_source_location);
	gnome_config_set_int ("gPHPEdit/defaults/indentationsize", preferences.indentation_size);
	gnome_config_set_int ("gPHPEdit/defaults/tabsize", preferences.tab_size);
	gnome_config_set_int ("gPHPEdit/defaults/linewrapping", preferences.line_wrapping);
	gnome_config_set_int ("gPHPEdit/defaults/showindentationguides", preferences.show_indentation_guides);
	gnome_config_set_int ("gPHPEdit/defaults/showfolding", preferences.show_folding);
	gnome_config_set_int ("gPHPEdit/defaults/edgemode", preferences.edge_mode);
	gnome_config_set_int ("gPHPEdit/defaults/edgecolumn", preferences.edge_column);
	gnome_config_set_int ("gPHPEdit/defaults/edgecolour", preferences.edge_colour);
	gnome_config_set_int ("gPHPEdit/defaults/auto_complete_delay", preferences.auto_complete_delay);
	gnome_config_set_int ("gPHPEdit/defaults/calltip_delay", preferences.calltip_delay);
	//gnome_config_set_int ("gPHPEdit/defaults/autoindentafterbrace", preferences.auto_indent_after_brace);
	gnome_config_set_string("gPHPEdit/defaults/php_file_extensions",preferences.php_file_extensions);
	gnome_config_set_bool("gPHPEdit/defaults/save_session", preferences.save_session);
	gnome_config_set_bool("gPHPEdit/defaults/use_tabs_instead_spaces", preferences.use_tabs_instead_spaces);
	gnome_config_set_bool ("gPHPEdit/defaults/single_instance_only", preferences.single_instance_only);

	gnome_config_set_int ("gPHPEdit/html_tag/back", preferences.html_tag_back);
	gnome_config_set_int ("gPHPEdit/html_tag/fore", preferences.html_tag_fore);
	gnome_config_set_string ("gPHPEdit/html_tag/font", preferences.html_tag_font);
	gnome_config_set_int ("gPHPEdit/html_tag/size", preferences.html_tag_size);
	gnome_config_set_bool ("gPHPEdit/html_tag/italic", preferences.html_tag_italic);
	gnome_config_set_bool ("gPHPEdit/html_tag/bold", preferences.html_tag_bold);

	gnome_config_set_int ("gPHPEdit/html_tag_unknown/back", preferences.html_tag_unknown_back);
	gnome_config_set_int ("gPHPEdit/html_tag_unknown/fore", preferences.html_tag_unknown_fore);
	gnome_config_set_string ("gPHPEdit/html_tag_unknown/font", preferences.html_tag_unknown_font);
	gnome_config_set_int ("gPHPEdit/html_tag_unknown/size", preferences.html_tag_unknown_size);
	gnome_config_set_bool ("gPHPEdit/html_tag_unknown/italic", preferences.html_tag_unknown_italic);
	gnome_config_set_bool ("gPHPEdit/html_tag_unknown/bold", preferences.html_tag_unknown_bold);

	gnome_config_set_int ("gPHPEdit/html_attribute/back", preferences.html_attribute_back);
	gnome_config_set_int ("gPHPEdit/html_attribute/fore", preferences.html_attribute_fore);
	gnome_config_set_string ("gPHPEdit/html_attribute/font", preferences.html_attribute_font);
	gnome_config_set_int ("gPHPEdit/html_attribute/size", preferences.html_attribute_size);
	gnome_config_set_bool ("gPHPEdit/html_attribute/italic", preferences.html_attribute_italic);
	gnome_config_set_bool ("gPHPEdit/html_attribute/bold", preferences.html_attribute_bold);

	gnome_config_set_int ("gPHPEdit/html_attribute_unknown/back", preferences.html_attribute_unknown_back);
	gnome_config_set_int ("gPHPEdit/html_attribute_unknown/fore", preferences.html_attribute_unknown_fore);
	gnome_config_set_string ("gPHPEdit/html_attribute_unknown/font", preferences.html_attribute_unknown_font);
	gnome_config_set_int ("gPHPEdit/html_attribute_unknown/size", preferences.html_attribute_unknown_size);
	gnome_config_set_bool ("gPHPEdit/html_attribute_unknown/italic", preferences.html_attribute_unknown_italic);
	gnome_config_set_bool ("gPHPEdit/html_attribute_unknown/bold", preferences.html_attribute_unknown_bold);

	gnome_config_set_int ("gPHPEdit/html_number/back", preferences.html_number_back);
	gnome_config_set_int ("gPHPEdit/html_number/fore", preferences.html_number_fore);
	gnome_config_set_string ("gPHPEdit/html_number/font", preferences.html_number_font);
	gnome_config_set_int ("gPHPEdit/html_number/size", preferences.html_number_size);
	gnome_config_set_bool ("gPHPEdit/html_number/italic", preferences.html_number_italic);
	gnome_config_set_bool ("gPHPEdit/html_number/bold", preferences.html_number_bold);

	gnome_config_set_int ("gPHPEdit/html_single_string/back", preferences.html_single_string_back);
	gnome_config_set_int ("gPHPEdit/html_single_string/fore", preferences.html_single_string_fore);
	gnome_config_set_string ("gPHPEdit/html_single_string/font", preferences.html_single_string_font);
	gnome_config_set_int ("gPHPEdit/html_single_string/size", preferences.html_single_string_size);
	gnome_config_set_bool ("gPHPEdit/html_single_string/italic", preferences.html_single_string_italic);
	gnome_config_set_bool ("gPHPEdit/html_single_string/bold", preferences.html_single_string_bold);

	gnome_config_set_int ("gPHPEdit/html_double_string/back", preferences.html_double_string_back);
	gnome_config_set_int ("gPHPEdit/html_double_string/fore", preferences.html_double_string_fore);
	gnome_config_set_string ("gPHPEdit/html_double_string/font", preferences.html_double_string_font);
	gnome_config_set_int ("gPHPEdit/html_double_string/size", preferences.html_double_string_size);
	gnome_config_set_bool ("gPHPEdit/html_double_string/italic", preferences.html_double_string_italic);
	gnome_config_set_bool ("gPHPEdit/html_double_string/bold", preferences.html_double_string_bold);

	gnome_config_set_int ("gPHPEdit/html_comment/back", preferences.html_comment_back);
	gnome_config_set_int ("gPHPEdit/html_comment/fore", preferences.html_comment_fore);
	gnome_config_set_string ("gPHPEdit/html_comment/font", preferences.html_comment_font);
	gnome_config_set_int ("gPHPEdit/html_comment/size", preferences.html_comment_size);
	gnome_config_set_bool ("gPHPEdit/html_comment/italic", preferences.html_comment_italic);
	gnome_config_set_bool ("gPHPEdit/html_comment/bold", preferences.html_comment_bold);

	gnome_config_set_int ("gPHPEdit/html_entity/back", preferences.html_entity_back);
	gnome_config_set_int ("gPHPEdit/html_entity/fore", preferences.html_entity_fore);
	gnome_config_set_string ("gPHPEdit/html_entity/font", preferences.html_entity_font);
	gnome_config_set_int ("gPHPEdit/html_entity/size", preferences.html_entity_size);
	gnome_config_set_bool ("gPHPEdit/html_entity/italic", preferences.html_entity_italic);
	gnome_config_set_bool ("gPHPEdit/html_entity/bold", preferences.html_entity_bold);

	gnome_config_set_int ("gPHPEdit/html_script/back", preferences.html_script_back);
	gnome_config_set_int ("gPHPEdit/html_script/fore", preferences.html_script_fore);
	gnome_config_set_string ("gPHPEdit/html_script/font", preferences.html_script_font);
	gnome_config_set_int ("gPHPEdit/html_script/size", preferences.html_script_size);
	gnome_config_set_bool ("gPHPEdit/html_script/italic", preferences.html_script_italic);
	gnome_config_set_bool ("gPHPEdit/html_script/bold", preferences.html_script_bold);

	gnome_config_set_int ("gPHPEdit/html_question/back", preferences.html_question_back);
	gnome_config_set_int ("gPHPEdit/html_question/fore", preferences.html_question_fore);
	gnome_config_set_string ("gPHPEdit/html_question/font", preferences.html_question_font);
	gnome_config_set_int ("gPHPEdit/html_question/size", preferences.html_question_size);
	gnome_config_set_bool ("gPHPEdit/html_question/italic", preferences.html_question_italic);
	gnome_config_set_bool ("gPHPEdit/html_question/bold", preferences.html_question_bold);

	gnome_config_set_int ("gPHPEdit/html_value/back", preferences.html_value_back);
	gnome_config_set_int ("gPHPEdit/html_value/fore", preferences.html_value_fore);
	gnome_config_set_string ("gPHPEdit/html_value/font", preferences.html_value_font);
	gnome_config_set_int ("gPHPEdit/html_value/size", preferences.html_value_size);
	gnome_config_set_bool ("gPHPEdit/html_value/italic", preferences.html_value_italic);
	gnome_config_set_bool ("gPHPEdit/html_value/bold", preferences.html_value_bold);

	gnome_config_set_int ("gPHPEdit/javascript_comment/back", preferences.javascript_comment_back);
	gnome_config_set_int ("gPHPEdit/javascript_comment/fore", preferences.javascript_comment_fore);
	gnome_config_set_string ("gPHPEdit/javascript_comment/font", preferences.javascript_comment_font);
	gnome_config_set_int ("gPHPEdit/javascript_comment/size", preferences.javascript_comment_size);
	gnome_config_set_bool ("gPHPEdit/javascript_comment/italic", preferences.javascript_comment_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_comment/bold", preferences.javascript_comment_bold);

	gnome_config_set_int ("gPHPEdit/javascript_comment_line/back", preferences.javascript_comment_line_back);
	gnome_config_set_int ("gPHPEdit/javascript_comment_line/fore", preferences.javascript_comment_line_fore);
	gnome_config_set_string ("gPHPEdit/javascript_comment_line/font", preferences.javascript_comment_line_font);
	gnome_config_set_int ("gPHPEdit/javascript_comment_line/size", preferences.javascript_comment_line_size);
	gnome_config_set_bool ("gPHPEdit/javascript_comment_line/italic", preferences.javascript_comment_line_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_comment_line/bold", preferences.javascript_comment_line_bold);

	gnome_config_set_int ("gPHPEdit/javascript_comment_doc/back", preferences.javascript_comment_doc_back);
	gnome_config_set_int ("gPHPEdit/javascript_comment_doc/fore", preferences.javascript_comment_doc_fore);
	gnome_config_set_string ("gPHPEdit/javascript_comment_doc/font", preferences.javascript_comment_doc_font);
	gnome_config_set_int ("gPHPEdit/javascript_comment_doc/size", preferences.javascript_comment_doc_size);
	gnome_config_set_bool ("gPHPEdit/javascript_comment_doc/italic", preferences.javascript_comment_doc_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_comment_doc/bold", preferences.javascript_comment_doc_bold);

	gnome_config_set_int ("gPHPEdit/javascript_word/back", preferences.javascript_word_back);
	gnome_config_set_int ("gPHPEdit/javascript_word/fore", preferences.javascript_word_fore);
	gnome_config_set_string ("gPHPEdit/javascript_word/font", preferences.javascript_word_font);
	gnome_config_set_int ("gPHPEdit/javascript_word/size", preferences.javascript_word_size);
	gnome_config_set_bool ("gPHPEdit/javascript_word/italic", preferences.javascript_word_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_word/bold", preferences.javascript_word_bold);

	gnome_config_set_int ("gPHPEdit/javascript_keyword/back", preferences.javascript_keyword_back);
	gnome_config_set_int ("gPHPEdit/javascript_keyword/fore", preferences.javascript_keyword_fore);
	gnome_config_set_string ("gPHPEdit/javascript_keyword/font", preferences.javascript_keyword_font);
	gnome_config_set_int ("gPHPEdit/javascript_keyword/size", preferences.javascript_keyword_size);
	gnome_config_set_bool ("gPHPEdit/javascript_keyword/italic", preferences.javascript_keyword_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_keyword/bold", preferences.javascript_keyword_bold);

	gnome_config_set_int ("gPHPEdit/javascript_doublestring/back", preferences.javascript_doublestring_back);
	gnome_config_set_int ("gPHPEdit/javascript_doublestring/fore", preferences.javascript_doublestring_fore);
	gnome_config_set_string ("gPHPEdit/javascript_doublestring/font", preferences.javascript_doublestring_font);
	gnome_config_set_int ("gPHPEdit/javascript_doublestring/size", preferences.javascript_doublestring_size);
	gnome_config_set_bool ("gPHPEdit/javascript_doublestring/italic", preferences.javascript_doublestring_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_doublestring/bold", preferences.javascript_doublestring_bold);

	gnome_config_set_int ("gPHPEdit/javascript_singlestring/back", preferences.javascript_singlestring_back);
	gnome_config_set_int ("gPHPEdit/javascript_singlestring/fore", preferences.javascript_singlestring_fore);
	gnome_config_set_string ("gPHPEdit/javascript_singlestring/font", preferences.javascript_singlestring_font);
	gnome_config_set_int ("gPHPEdit/javascript_singlestring/size", preferences.javascript_singlestring_size);
	gnome_config_set_bool ("gPHPEdit/javascript_singlestring/italic", preferences.javascript_singlestring_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_singlestring/bold", preferences.javascript_singlestring_bold);

	gnome_config_set_int ("gPHPEdit/javascript_symbols/back", preferences.javascript_symbols_back);
	gnome_config_set_int ("gPHPEdit/javascript_symbols/fore", preferences.javascript_symbols_fore);
	gnome_config_set_string ("gPHPEdit/javascript_symbols/font", preferences.javascript_symbols_font);
	gnome_config_set_int ("gPHPEdit/javascript_symbols/size", preferences.javascript_symbols_size);
	gnome_config_set_bool ("gPHPEdit/javascript_symbols/italic", preferences.javascript_symbols_italic);
	gnome_config_set_bool ("gPHPEdit/javascript_symbols/bold", preferences.javascript_symbols_bold);

	gnome_config_set_int ("gPHPEdit/php_default_style/back", preferences.php_default_back);
	gnome_config_set_int ("gPHPEdit/php_default_style/fore", preferences.php_default_fore);
	gnome_config_set_string ("gPHPEdit/php_default_style/font", preferences.php_default_font);
	gnome_config_set_int ("gPHPEdit/php_default_style/size", preferences.php_default_size);
	gnome_config_set_bool ("gPHPEdit/php_default_style/italic", preferences.php_default_italic);
	gnome_config_set_bool ("gPHPEdit/php_default_style/bold", preferences.php_default_bold);

	gnome_config_set_int ("gPHPEdit/php_hstring/fore", preferences.php_hstring_fore);
	gnome_config_set_string ("gPHPEdit/php_hstring/font", preferences.php_hstring_font);
	gnome_config_set_int ("gPHPEdit/php_hstring/back", preferences.php_hstring_back);
	gnome_config_set_int ("gPHPEdit/php_hstring/size", preferences.php_hstring_size);
	gnome_config_set_bool ("gPHPEdit/php_hstring/italic", preferences.php_hstring_italic);
	gnome_config_set_bool ("gPHPEdit/php_hstring/bold", preferences.php_hstring_bold);

	gnome_config_set_int ("gPHPEdit/php_simplestring/fore", preferences.php_simplestring_fore);
	gnome_config_set_string ("gPHPEdit/php_simplestring/font", preferences.php_simplestring_font);
	gnome_config_set_int ("gPHPEdit/php_simplestring/back", preferences.php_simplestring_back);
	gnome_config_set_int ("gPHPEdit/php_simplestring/size", preferences.php_simplestring_size);
	gnome_config_set_bool ("gPHPEdit/php_simplestring/italic", preferences.php_simplestring_italic);
	gnome_config_set_bool ("gPHPEdit/php_simplestring/bold", preferences.php_simplestring_bold);

	gnome_config_set_int ("gPHPEdit/php_word/fore", preferences.php_word_fore);
	gnome_config_set_string ("gPHPEdit/php_word/font", preferences.php_word_font);
	gnome_config_set_int ("gPHPEdit/php_word/back", preferences.php_word_back);
	gnome_config_set_int ("gPHPEdit/php_word/size", preferences.php_word_size);
	gnome_config_set_bool ("gPHPEdit/php_word/italic", preferences.php_word_italic);
	gnome_config_set_bool ("gPHPEdit/php_word/bold", preferences.php_word_bold);

	gnome_config_set_int ("gPHPEdit/php_number/fore", preferences.php_number_fore);
	gnome_config_set_string ("gPHPEdit/php_number/font", preferences.php_number_font);
	gnome_config_set_int ("gPHPEdit/php_number/back", preferences.php_number_back);
	gnome_config_set_int ("gPHPEdit/php_number/size", preferences.php_number_size);
	gnome_config_set_bool ("gPHPEdit/php_number/italic", preferences.php_number_italic);
	gnome_config_set_bool ("gPHPEdit/php_number/bold", preferences.php_number_bold);

	gnome_config_set_int ("gPHPEdit/php_variable/fore", preferences.php_variable_fore);
	gnome_config_set_string ("gPHPEdit/php_variable/font", preferences.php_variable_font);
	gnome_config_set_int ("gPHPEdit/php_variable/back", preferences.php_variable_back);
	gnome_config_set_int ("gPHPEdit/php_variable/size", preferences.php_variable_size);
	gnome_config_set_bool ("gPHPEdit/php_variable/italic", preferences.php_variable_italic);
	gnome_config_set_bool ("gPHPEdit/php_variable/bold", preferences.php_variable_bold);

	gnome_config_set_int ("gPHPEdit/php_comment/fore", preferences.php_comment_fore);
	gnome_config_set_string ("gPHPEdit/php_comment/font", preferences.php_comment_font);
	gnome_config_set_int ("gPHPEdit/php_comment/back", preferences.php_comment_back);
	gnome_config_set_int ("gPHPEdit/php_comment/size", preferences.php_comment_size);
	gnome_config_set_bool ("gPHPEdit/php_comment/italic", preferences.php_comment_italic);
	gnome_config_set_bool ("gPHPEdit/php_comment/bold", preferences.php_comment_bold);

	gnome_config_set_int ("gPHPEdit/php_comment_line/fore", preferences.php_comment_line_fore);
	gnome_config_set_string ("gPHPEdit/php_comment_line/font", preferences.php_comment_line_font);
	gnome_config_set_int ("gPHPEdit/php_comment_line/back", preferences.php_comment_line_back);
	gnome_config_set_int ("gPHPEdit/php_comment_line/size", preferences.php_comment_line_size);
	gnome_config_set_bool ("gPHPEdit/php_comment_line/italic", preferences.php_comment_line_italic);
	gnome_config_set_bool ("gPHPEdit/php_comment_line/bold", preferences.php_comment_line_bold);

	gnome_config_set_int ("gPHPEdit/css_tag/back", preferences.css_tag_back);
	gnome_config_set_string ("gPHPEdit/css_tag/font", preferences.css_tag_font);
	gnome_config_set_int ("gPHPEdit/css_tag/fore", preferences.css_tag_fore);
	gnome_config_set_int ("gPHPEdit/css_tag/size", preferences.css_tag_size);
	gnome_config_set_bool ("gPHPEdit/css_tag/italic", preferences.css_tag_italic);
	gnome_config_set_bool ("gPHPEdit/css_tag/bold", preferences.css_tag_bold);

	gnome_config_set_int ("gPHPEdit/css_class/fore", preferences.css_class_fore);
	gnome_config_set_string ("gPHPEdit/css_class/font", preferences.css_class_font);
	gnome_config_set_int ("gPHPEdit/css_class/back", preferences.css_class_back);
	gnome_config_set_int ("gPHPEdit/css_class/size", preferences.css_class_size);
	gnome_config_set_bool ("gPHPEdit/css_class/italic", preferences.css_class_italic);
	gnome_config_set_bool ("gPHPEdit/css_class/bold", preferences.css_class_bold);

	gnome_config_set_int ("gPHPEdit/css_pseudoclass/back", preferences.css_pseudoclass_back);
	gnome_config_set_string ("gPHPEdit/css_pseudoclass/font", preferences.css_pseudoclass_font);
	gnome_config_set_int ("gPHPEdit/css_pseudoclass/fore", preferences.css_pseudoclass_fore);
	gnome_config_set_int ("gPHPEdit/css_pseudoclass/size", preferences.css_pseudoclass_size);
	gnome_config_set_bool ("gPHPEdit/css_pseudoclass/italic", preferences.css_pseudoclass_italic);
	gnome_config_set_bool ("gPHPEdit/css_pseudoclass/bold", preferences.css_pseudoclass_bold);

	gnome_config_set_int ("gPHPEdit/css_unknown_pseudoclass/fore", preferences.css_unknown_pseudoclass_fore);
	gnome_config_set_string ("gPHPEdit/css_unknown_pseudoclass/font", preferences.css_unknown_pseudoclass_font);
	gnome_config_set_int ("gPHPEdit/css_unknown_pseudoclass/back", preferences.css_unknown_pseudoclass_back);
	gnome_config_set_int ("gPHPEdit/css_unknown_pseudoclass/size", preferences.css_unknown_pseudoclass_size);
	gnome_config_set_bool ("gPHPEdit/css_unknown_pseudoclass/italic", preferences.css_unknown_pseudoclass_italic);
	gnome_config_set_bool ("gPHPEdit/css_unknown_pseudoclass/bold", preferences.css_unknown_pseudoclass_bold);

	gnome_config_set_int ("gPHPEdit/css_operator/fore", preferences.css_operator_fore);
	gnome_config_set_string ("gPHPEdit/css_operator/font", preferences.css_operator_font);
	gnome_config_set_int ("gPHPEdit/css_operator/back", preferences.css_operator_back);
	gnome_config_set_int ("gPHPEdit/css_operator/size", preferences.css_operator_size);
	gnome_config_set_bool ("gPHPEdit/css_operator/italic", preferences.css_operator_italic);
	gnome_config_set_bool ("gPHPEdit/css_operator/bold", preferences.css_operator_bold);

	gnome_config_set_int ("gPHPEdit/css_identifier/fore", preferences.css_identifier_fore);
	gnome_config_set_string ("gPHPEdit/css_identifier/font", preferences.css_identifier_font);
	gnome_config_set_int ("gPHPEdit/css_identifier/back", preferences.css_identifier_back);
	gnome_config_set_int ("gPHPEdit/css_identifier/size", preferences.css_identifier_size);
	gnome_config_set_bool ("gPHPEdit/css_identifier/italic", preferences.css_identifier_italic);
	gnome_config_set_bool ("gPHPEdit/css_identifier/bold", preferences.css_identifier_bold);

	gnome_config_set_int ("gPHPEdit/css_unknown_identifier/fore", preferences.css_unknown_identifier_fore);
	gnome_config_set_string ("gPHPEdit/css_unknown_identifier/font", preferences.css_unknown_identifier_font);
	gnome_config_set_int ("gPHPEdit/css_unknown_identifier/back=16777215", preferences.css_unknown_identifier_back);
	gnome_config_set_int ("gPHPEdit/css_unknown_identifier/size", preferences.css_unknown_identifier_size);
	gnome_config_set_bool ("gPHPEdit/css_unknown_identifier/italic", preferences.css_unknown_identifier_italic);
	gnome_config_set_bool ("gPHPEdit/css_unknown_identifier/bold", preferences.css_unknown_identifier_bold);

	gnome_config_set_int ("gPHPEdit/css_value/fore", preferences.css_value_fore);
	gnome_config_set_string ("gPHPEdit/css_value/font", preferences.css_value_font);
	gnome_config_set_int ("gPHPEdit/css_value/back", preferences.css_value_back);
	gnome_config_set_int ("gPHPEdit/css_value/size", preferences.css_value_size);
	gnome_config_set_bool ("gPHPEdit/css_value/italic", preferences.css_value_italic);
	gnome_config_set_bool ("gPHPEdit/css_value/bold", preferences.css_value_bold);

	gnome_config_set_int ("gPHPEdit/css_comment/fore", preferences.css_comment_fore);
	gnome_config_set_string ("gPHPEdit/css_comment/font", preferences.css_comment_font);
	gnome_config_set_int ("gPHPEdit/css_comment/back", preferences.css_comment_back);
	gnome_config_set_int ("gPHPEdit/css_comment/size", preferences.css_comment_size);
	gnome_config_set_bool ("gPHPEdit/css_comment/italic", preferences.css_comment_italic);
	gnome_config_set_bool ("gPHPEdit/css_comment/bold", preferences.css_comment_bold);

	gnome_config_set_string ("gPHPEdit/css_id/fore", preferences.css_id_font);
	gnome_config_set_int ("gPHPEdit/css_id/font", preferences.css_id_fore);
	gnome_config_set_int ("gPHPEdit/css_id/back", preferences.css_id_back);
	gnome_config_set_int ("gPHPEdit/css_id/size", preferences.css_id_size);
	gnome_config_set_bool ("gPHPEdit/css_id/italic", preferences.css_id_italic);
	gnome_config_set_bool ("gPHPEdit/css_id/bold", preferences.css_id_bold);

	gnome_config_set_string ("gPHPEdit/css_important/fore", preferences.css_important_font);
	gnome_config_set_int ("gPHPEdit/css_important/font", preferences.css_important_fore);
	gnome_config_set_int ("gPHPEdit/css_important/back", preferences.css_important_back);
	gnome_config_set_int ("gPHPEdit/css_important/size", preferences.css_important_size);
	gnome_config_set_bool ("gPHPEdit/css_important/italic", preferences.css_important_italic);
	gnome_config_set_bool ("gPHPEdit/css_important/bold", preferences.css_important_bold);

	gnome_config_set_int ("gPHPEdit/css_directive/fore", preferences.css_directive_fore);
	gnome_config_set_string ("gPHPEdit/css_directive/font", preferences.css_directive_font);
	gnome_config_set_int ("gPHPEdit/css_directive/back", preferences.css_directive_back);
	gnome_config_set_int ("gPHPEdit/css_directive/size", preferences.css_directive_size);
	gnome_config_set_bool ("gPHPEdit/css_directive/italic", preferences.css_directive_italic);
	gnome_config_set_bool ("gPHPEdit/css_directive/bold", preferences.css_directive_bold);

	gnome_config_set_int ("gPHPEdit/sql_word/fore", preferences.sql_word_fore);
	gnome_config_set_string ("gPHPEdit/sql_word/font", preferences.sql_word_font);
	gnome_config_set_int ("gPHPEdit/sql_word/back", preferences.sql_word_back);
	gnome_config_set_int ("gPHPEdit/sql_word/size", preferences.sql_word_size);
	gnome_config_set_bool ("gPHPEdit/sql_word/italic", preferences.sql_word_italic);
	gnome_config_set_bool ("gPHPEdit/sql_word/bold", preferences.sql_word_bold);

	gnome_config_set_int ("gPHPEdit/sql_string/fore", preferences.sql_string_fore);
	gnome_config_set_string ("gPHPEdit/sql_string/font", preferences.sql_string_font);
	gnome_config_set_int ("gPHPEdit/sql_string/back", preferences.sql_string_back);
	gnome_config_set_int ("gPHPEdit/sql_string/size", preferences.sql_string_size);
	gnome_config_set_bool ("gPHPEdit/sql_string/italic", preferences.sql_string_italic);
	gnome_config_set_bool ("gPHPEdit/sql_string/bold", preferences.sql_string_bold);

	gnome_config_set_int ("gPHPEdit/sql_operator/fore", preferences.sql_operator_fore);
	gnome_config_set_string ("gPHPEdit/sql_operator/font", preferences.sql_operator_font);
	gnome_config_set_int ("gPHPEdit/sql_operator/back", preferences.sql_operator_back);
	gnome_config_set_int ("gPHPEdit/sql_operator/size", preferences.sql_operator_size);
	gnome_config_set_bool ("gPHPEdit/sql_operator/italic", preferences.sql_operator_italic);
	gnome_config_set_bool ("gPHPEdit/sql_operator/bold", preferences.sql_operator_bold);

	gnome_config_set_int ("gPHPEdit/sql_comment/fore", preferences.sql_comment_fore);
	gnome_config_set_string ("gPHPEdit/sql_comment/font", preferences.sql_comment_font);
	gnome_config_set_int ("gPHPEdit/sql_comment/back", preferences.sql_comment_back);
	gnome_config_set_int ("gPHPEdit/sql_comment/size", preferences.sql_comment_size);
	gnome_config_set_bool ("gPHPEdit/sql_comment/italic", preferences.sql_comment_italic);
	gnome_config_set_bool ("gPHPEdit/sql_comment/bold", preferences.sql_comment_bold);

	gnome_config_set_int ("gPHPEdit/sql_number/fore", preferences.sql_number_fore);
	gnome_config_set_string ("gPHPEdit/sql_number/font", preferences.sql_number_font);
	gnome_config_set_int ("gPHPEdit/sql_number/back", preferences.sql_number_back);
	gnome_config_set_int ("gPHPEdit/sql_number/size", preferences.sql_number_size);
	gnome_config_set_bool ("gPHPEdit/sql_number/italic", preferences.sql_number_italic);
	gnome_config_set_bool ("gPHPEdit/sql_number/bold", preferences.sql_number_bold);

	gnome_config_set_int ("gPHPEdit/sql_identifier/fore", preferences.sql_identifier_fore);
	gnome_config_set_string ("gPHPEdit/sql_identifier/font", preferences.sql_identifier_font);
	gnome_config_set_int ("gPHPEdit/sql_identifier/back", preferences.sql_identifier_back);
	gnome_config_set_int ("gPHPEdit/sql_identifier/size", preferences.sql_identifier_size);
	gnome_config_set_bool ("gPHPEdit/sql_identifier/italic", preferences.sql_identifier_italic);
	gnome_config_set_bool ("gPHPEdit/sql_identifier/bold", preferences.sql_identifier_bold);

	gnome_config_sync();
}
