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
		// been set using gconf
		
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
        GConfClient *config;
        config=gconf_client_get_default ();
        GError *error = NULL;
	preferences.left = gconf_client_get_int (config,"/gPHPEdit/main_window/x",&error);
        if (preferences.left ==0 && error!=NULL){
            preferences.left=20;
            error=NULL;
        }
	preferences.top = gconf_client_get_int (config,"/gPHPEdit/main_window/y",&error);
        if (preferences.top ==0 && error!=NULL){
            preferences.top=20;
            error=NULL;
        }
	preferences.width  = gconf_client_get_int (config,"/gPHPEdit/main_window/width",&error);
        if (preferences.width ==0 && error!=NULL){
            preferences.width=400;
            error=NULL;
        }
	preferences.height = gconf_client_get_int (config,"/gPHPEdit/main_window/height",&error);
        if (preferences.height ==0 && error!=NULL){
            preferences.height=400;
            error=NULL;
        }
	preferences.maximized = gconf_client_get_int (config,"/gPHPEdit/main_window/maximized",&error);
        
	gtk_window_move(GTK_WINDOW(main_window.window), preferences.left, preferences.top);
	gtk_window_set_default_size(GTK_WINDOW(main_window.window), preferences.width, preferences.height);
 	
	if (preferences.maximized) {
 		gtk_window_maximize(GTK_WINDOW(main_window.window));
	}
}

void preferences_load(void)
{
        GConfClient *config;
        config=gconf_client_get_default ();
        GError *error = NULL;
        gconf_client_get_int (config,"/gPHPEdit/main_window/x",&error);
        preferences.set_sel_back = gconf_client_get_int (config,"/gPHPEdit/default_style/selection",&error);
        if (preferences.set_sel_back==0 && error!=NULL){
            preferences.set_sel_back=11250603;
            error=NULL;
        }
        preferences.marker_back = gconf_client_get_int (config,"/gPHPEdit/default_style/bookmark",&error);
        if (preferences.marker_back==0 && error!=NULL){
            preferences.marker_back=15908608;
            error=NULL;
        }
        preferences.php_binary_location= gconf_client_get_string(config,"/gPHPEdit/locations/phpbinary",&error);
        if (!preferences.php_binary_location){
            preferences.php_binary_location="php";
            error=NULL;
        }
        preferences.shared_source_location = gconf_client_get_string(config,"/gPHPEdit/locations/shared_source",&error);
        if (!preferences.shared_source_location){
            preferences.shared_source_location="";
            error=NULL;
        }
	preferences.indentation_size = gconf_client_get_int (config,"/gPHPEdit/defaults/indentationsize",&error);
        if (preferences.indentation_size==0 && error!=NULL){
            preferences.indentation_size=4;
            error=NULL;
        }
	preferences.tab_size = gconf_client_get_int (config,"/gPHPEdit/defaults/tabsize",&error);
        if (preferences.tab_size==0 && error!=NULL){
            preferences.tab_size=4;
            error=NULL;
        }
	preferences.auto_complete_delay = gconf_client_get_int (config,"/gPHPEdit/defaults/auto_complete_delay",&error);
        if (preferences.auto_complete_delay==0 && error!=NULL){
            preferences.auto_complete_delay=500;
            error=NULL;
        }
	preferences.calltip_delay = gconf_client_get_int (config,"/gPHPEdit/defaults/calltip_delay",&error);
        if (preferences.calltip_delay==0 && error!=NULL){
            preferences.calltip_delay=500;
            error=NULL;
        }
	preferences.show_indentation_guides = gconf_client_get_int (config,"/gPHPEdit/defaults/showindentationguides",NULL);
	preferences.show_folding = gconf_client_get_int (config,"/gPHPEdit/defaults/showfolding",NULL);
	preferences.edge_mode = gconf_client_get_int (config,"/gPHPEdit/defaults/edgemode",NULL);
	preferences.edge_column = gconf_client_get_int (config,"/gPHPEdit/defaults/edgecolumn",&error);
        if (preferences.edge_column==0 && error!=NULL){
            preferences.edge_column=80;
            error=NULL;
        }
	preferences.edge_colour = gconf_client_get_int (config,"/gPHPEdit/defaults/edgecolour",&error);
        if (preferences.edge_colour==0 && error!=NULL){
            preferences.edge_colour=8355712;
            error=NULL;
        }
	preferences.line_wrapping = gconf_client_get_int (config,"/gPHPEdit/defaults/linewrapping",&error);
        if (preferences.line_wrapping==0 && error!=NULL){
            preferences.line_wrapping=1;
            error=NULL;
        }
	//preferences.auto_indent_after_brace = gnome_config_get_int ("gPHPEdit/defaults/autoindentafterbrace=1");
	preferences.save_session = gconf_client_get_int (config,"/gPHPEdit/defaults/save_session",&error);
        if (preferences.save_session==0 && error!=NULL){
            preferences.save_session=true;
            error=NULL;
        }
	preferences.use_tabs_instead_spaces = gconf_client_get_bool(config,"/gPHPEdit/defaults/use_tabs_instead_spaces",&error);
        if (preferences.use_tabs_instead_spaces==0 && error!=NULL){
            preferences.use_tabs_instead_spaces=true;
            error=NULL;
        }
        preferences.single_instance_only = gconf_client_get_bool(config,"/gPHPEdit/defaults/single_instance_only",NULL);
        preferences.php_file_extensions = gconf_client_get_string(config,"/gPHPEdit/defaults/php_file_extensions",&error);
        if (!preferences.php_file_extensions){
            preferences.php_file_extensions="php,inc,phtml,php3,xml,htm,html";
            error=NULL;
        }
	preferences.default_font = gconf_client_get_string(config,"/gPHPEdit/default_style/font",&error);
        if (!preferences.default_font){
            preferences.default_font="!Sans";
            error=NULL;
        }
	preferences.default_fore = gconf_client_get_int (config,"/gPHPEdit/default_style/fore",NULL);
	preferences.default_back = gconf_client_get_int (config,"/gPHPEdit/default_style/back",&error);
        if (preferences.default_back==0 && error!=NULL){
            preferences.default_back=16777215;
            error=NULL;
        }
	preferences.default_size = gconf_client_get_int (config,"/gPHPEdit/default_style/size",&error);
        if (preferences.default_size==0 && error!=NULL){
            preferences.default_size=12;
            error=NULL;
        }
	preferences.default_bold = gconf_client_get_bool(config,"/gPHPEdit/default_style/bold",NULL);
	preferences.default_italic = gconf_client_get_bool(config,"/gPHPEdit/default_style/italic",NULL);
	
	preferences.line_number_font = gconf_client_get_string(config,"/gPHPEdit/line_numbers/font",&error);
        if (!preferences.line_number_font){
            preferences.line_number_font="!Sans";
            error=NULL;
        }
	preferences.line_number_fore = gconf_client_get_int (config,"/gPHPEdit/line_numbers/fore",NULL);
	preferences.line_number_back = gconf_client_get_int (config,"/gPHPEdit/line_numbers/back",&error);
         if (preferences.line_number_back==0 && error!=NULL){
            preferences.line_number_back=11053224;
            error=NULL;
        }
	preferences.line_number_size = gconf_client_get_int (config,"/gPHPEdit/line_numbers/size",&error);
         if (preferences.line_number_size==0 && error!=NULL){
            preferences.line_number_size=12;
            error=NULL;
        }
	preferences.line_number_bold = gconf_client_get_bool(config,"/gPHPEdit/line_numbers/bold",NULL);
	preferences.line_number_italic = gconf_client_get_bool(config,"/gPHPEdit/line_numbers/italic",NULL);

	preferences.html_tag_back = gconf_client_get_int (config,"/gPHPEdit/html_tag/back",&error);
        if (preferences.html_tag_back==0 && error!=NULL){
            preferences.html_tag_back=16777215;
            error=NULL;
        }
	preferences.html_tag_fore = gconf_client_get_int (config,"/gPHPEdit/html_tag/fore",&error);
        if (preferences.html_tag_fore==0 && error!=NULL){
            preferences.html_tag_fore=7553164;
            error=NULL;
        }
	preferences.html_tag_font = gconf_client_get_string(config,"/gPHPEdit/html_tag/font",&error);
        if (!preferences.html_tag_font){
            preferences.html_tag_font="!Sans";
            error=NULL;
        }
	preferences.html_tag_size = gconf_client_get_int (config,"/gPHPEdit/html_tag/size",&error);
         if (preferences.html_tag_size==0 && error!=NULL){
            preferences.html_tag_size=12;
            error=NULL;
        }
	preferences.html_tag_bold = gconf_client_get_bool(config,"/gPHPEdit/html_tag/bold",NULL);
	preferences.html_tag_italic = gconf_client_get_bool(config,"/gPHPEdit/html_tag/italic",NULL);

	preferences.html_tag_unknown_back = gconf_client_get_int (config,"/gPHPEdit/html_tag_unknown/back",&error);
         if (preferences.html_tag_unknown_back==0 && error!=NULL){
            preferences.html_tag_unknown_back=16777215;
            error=NULL;
        }
	preferences.html_tag_unknown_fore = gconf_client_get_int (config,"/gPHPEdit/html_tag_unknown/fore",&error);
        if (preferences.html_tag_unknown_fore==0 && error!=NULL){
            preferences.html_tag_unknown_fore=7553164;
            error=NULL;
        }
	preferences.html_tag_unknown_font = gconf_client_get_string(config,"/gPHPEdit/html_tag_unknown/font",&error);
        if (!preferences.html_tag_unknown_font){
            preferences.html_tag_unknown_font="!Sans";
            error=NULL;
        }
	preferences.html_tag_unknown_size = gconf_client_get_int (config,"/gPHPEdit/html_tag_unknown/size",&error);
         if (preferences.html_tag_unknown_size==0 && error!=NULL){
            preferences.html_tag_unknown_size=12;
            error=NULL;
        }
	preferences.html_tag_unknown_bold = gconf_client_get_bool(config,"/gPHPEdit/html_tag_unknown/bold",NULL);
	preferences.html_tag_unknown_italic = gconf_client_get_bool(config,"/gPHPEdit/html_tag_unknown/italic",NULL);

	preferences.html_attribute_back = gconf_client_get_int (config,"/gPHPEdit/html_attribute/back",&error);
        if (preferences.html_attribute_back==0 && error!=NULL){
            preferences.html_attribute_back=16777215;
            error=NULL;
        }
	preferences.html_attribute_fore = gconf_client_get_int (config,"/gPHPEdit/html_attribute/fore",&error);
        if (preferences.html_attribute_fore==0 && error!=NULL){
            preferences.html_attribute_fore=9204544;
            error=NULL;
        }
	preferences.html_attribute_font = gconf_client_get_string(config,"/gPHPEdit/html_attribute/font",&error);
        if (!preferences.html_attribute_font){
            preferences.html_attribute_font="!Sans";
            error=NULL;
        }
	preferences.html_attribute_size = gconf_client_get_int (config,"/gPHPEdit/html_attribute/size",&error);
         if (preferences.html_attribute_size==0 && error!=NULL){
            preferences.html_attribute_size=12;
            error=NULL;
        }
	preferences.html_attribute_bold = gconf_client_get_bool(config,"/gPHPEdit/html_attribute/bold",NULL);
	preferences.html_attribute_italic = gconf_client_get_bool(config,"/gPHPEdit/html_attribute/italic",NULL);

	preferences.html_attribute_unknown_back = gconf_client_get_int (config,"/gPHPEdit/html_attribute_unknown/back",&error);
        if (preferences.html_attribute_unknown_back==0 && error!=NULL){
            preferences.html_attribute_unknown_back=16777215;
            error=NULL;
        }
	preferences.html_attribute_unknown_fore = gconf_client_get_int (config,"/gPHPEdit/html_attribute_unknown/fore",&error);
        if (preferences.html_attribute_unknown_fore==0 && error!=NULL){
            preferences.html_attribute_unknown_fore=7472544;
            error=NULL;
        }
	preferences.html_attribute_unknown_font = gconf_client_get_string(config,"/gPHPEdit/html_attribute_unknown/font",&error);
        if (!preferences.html_attribute_unknown_font){
            preferences.html_attribute_unknown_font="!Sans";
            error=NULL;
        }
	preferences.html_attribute_unknown_size = gconf_client_get_int (config,"/gPHPEdit/html_attribute_unknown/size",&error);
         if (preferences.html_attribute_unknown_size==0 && error!=NULL){
            preferences.html_attribute_unknown_size=12;
            error=NULL;
        }
	preferences.html_attribute_unknown_bold = gconf_client_get_bool(config,"/gPHPEdit/html_attribute_unknown/bold",NULL);
	preferences.html_attribute_unknown_italic = gconf_client_get_bool(config,"/gPHPEdit/html_attribute_unknown/italic",NULL);

	preferences.html_number_back = gconf_client_get_int (config,"/gPHPEdit/html_number/back",&error);
        if (preferences.html_number_back==0 && error!=NULL){
            preferences.html_number_back=16777215;
            error=NULL;
        }
	preferences.html_number_fore = gconf_client_get_int (config,"/gPHPEdit/html_number/fore",&error);
        if (preferences.html_number_fore==0 && error!=NULL){
            preferences.html_number_fore=9204544;
            error=NULL;
        }
	preferences.html_number_font = gconf_client_get_string(config,"/gPHPEdit/html_number/font",&error);
        if (!preferences.html_number_font){
            preferences.html_number_font="!Sans";
            error=NULL;
        }
	preferences.html_number_size = gconf_client_get_int (config,"/gPHPEdit/html_number/size",&error);
        if (preferences.html_number_size==0 && error!=NULL){
            preferences.html_number_size=12;
            error=NULL;
        }
	preferences.html_number_bold = gconf_client_get_bool(config,"/gPHPEdit/html_number/bold",NULL);
	preferences.html_number_italic = gconf_client_get_bool(config,"/gPHPEdit/html_number/italic",NULL);

	preferences.html_single_string_back = gconf_client_get_int (config,"/gPHPEdit/html_single_string/back",&error);
        if (preferences.html_single_string_back==0 && error!=NULL){
            preferences.html_single_string_back=16777215;
            error=NULL;
        }
	preferences.html_single_string_fore = gconf_client_get_int (config,"/gPHPEdit/html_single_string/fore",&error);
        if (preferences.html_single_string_back==0 && error!=NULL){
            preferences.html_single_string_fore=32768;
            error=NULL;
        }
	preferences.html_single_string_font = gconf_client_get_string(config,"/gPHPEdit/html_single_string/font",&error);
        if (!preferences.html_single_string_font){
            preferences.html_single_string_font="!Sans";
            error=NULL;
        }
	preferences.html_single_string_size = gconf_client_get_int (config,"/gPHPEdit/html_single_string/size",&error);
        if (preferences.html_single_string_size==0 && error!=NULL){
            preferences.html_single_string_size=12;
            error=NULL;
        }
	preferences.html_single_string_bold = gconf_client_get_bool(config,"/gPHPEdit/html_single_string/bold",NULL);
	preferences.html_single_string_italic = gconf_client_get_bool(config,"/gPHPEdit/html_single_string/italic",NULL);

	preferences.html_double_string_back = gconf_client_get_int (config,"/gPHPEdit/html_double_string/back",&error);
        if (preferences.html_double_string_back==0 && error!=NULL){
            preferences.html_double_string_back=16777215;
            error=NULL;
        }
	preferences.html_double_string_fore = gconf_client_get_int (config,"/gPHPEdit/html_double_string/fore",&error);
        if (preferences.html_double_string_fore==0 && error!=NULL){
            preferences.html_double_string_fore=32768;
            error=NULL;
        }
	preferences.html_double_string_font = gconf_client_get_string(config,"/gPHPEdit/html_double_string/font",&error);
        if (!preferences.html_double_string_font){
            preferences.html_double_string_font="!Sans";
            error=NULL;
        }
	preferences.html_double_string_size = gconf_client_get_int (config,"/gPHPEdit/html_double_string/size",&error);
        if (preferences.html_double_string_size==0 && error!=NULL){
            preferences.html_double_string_size=12;
            error=NULL;
        }
	preferences.html_double_string_bold = gconf_client_get_bool(config,"/gPHPEdit/html_double_string/bold",NULL);
	preferences.html_double_string_italic = gconf_client_get_bool(config,"/gPHPEdit/html_double_string/italic",NULL);

	preferences.html_comment_back = gconf_client_get_int (config,"/gPHPEdit/html_comment/back",&error);
        if (preferences.html_comment_back==0 && error!=NULL){
            preferences.html_comment_back=16777215;
            error=NULL;
        }
	preferences.html_comment_fore = gconf_client_get_int (config,"/gPHPEdit/html_comment/fore",&error);
        if (preferences.html_comment_fore==0 && error!=NULL){
            preferences.html_comment_fore=842125504;
            error=NULL;
        }
        preferences.html_comment_font = gconf_client_get_string(config,"/gPHPEdit/html_comment/font",&error);
        if (!preferences.html_comment_font){
            preferences.html_comment_font="!Sans";
            error=NULL;
        }
	preferences.html_comment_size = gconf_client_get_int (config,"/gPHPEdit/html_comment/back",&error);
        if (preferences.html_comment_size==0 && error!=NULL){
            preferences.html_comment_size=12;
            error=NULL;
        }
	preferences.html_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/html_comment/bold",NULL);
	preferences.html_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/html_comment/italic",NULL);

	preferences.html_entity_back =  gconf_client_get_int (config,"/gPHPEdit/html_entity/back",&error);
        if (preferences.html_entity_back==0 && error!=NULL){
            preferences.html_entity_back=16777215;
            error=NULL;
        }
	preferences.html_entity_fore =  gconf_client_get_int (config,"/gPHPEdit/html_entity/fore",&error);
        if (preferences.html_entity_fore==0 && error!=NULL){
            preferences.html_entity_fore=8421504;
            error=NULL;
        }
	preferences.html_entity_font = gconf_client_get_string(config,"/gPHPEdit/html_entity/font",&error);
        if (!preferences.html_entity_font){
            preferences.html_entity_font="!Sans";
            error=NULL;
        }
	preferences.html_entity_size =  gconf_client_get_int (config,"/gPHPEdit/html_entity/size",&error);
        if (preferences.html_entity_size==0 && error!=NULL){
            preferences.html_entity_size=12;
            error=NULL;
        }
        preferences.html_entity_bold = gconf_client_get_bool(config,"/gPHPEdit/html_entity/bold",NULL);
	preferences.html_entity_italic = gconf_client_get_bool(config,"/gPHPEdit/html_entity/italic",NULL);

	preferences.html_script_back = gconf_client_get_int (config,"/gPHPEdit/html_script/back",&error);
        if (preferences.html_script_back==0 && error!=NULL){
            preferences.html_script_back=16777215;
            error=NULL;
        }
	preferences.html_script_fore = gconf_client_get_int (config,"/gPHPEdit/html_script/fore",&error);
        if (preferences.html_script_fore==0 && error!=NULL){
            preferences.html_script_fore=7553165;
            error=NULL;
        }
	preferences.html_script_font = gconf_client_get_string(config,"/gPHPEdit/html_script/font",&error);
        if (!preferences.html_script_font){
            preferences.html_script_font="!Sans";
            error=NULL;
        }
	preferences.html_script_size = gconf_client_get_int (config,"/gPHPEdit/html_script/size",&error);
        if (preferences.html_script_size==0 && error!=NULL){
            preferences.html_script_size=12;
            error=NULL;
        }
	preferences.html_script_bold = gconf_client_get_bool(config,"/gPHPEdit/html_script/bold",NULL);
	preferences.html_script_italic = gconf_client_get_bool(config,"/gPHPEdit/html_script/italic",NULL);

	preferences.html_question_back = gconf_client_get_int (config,"/gPHPEdit/html_question/back",&error);
        if (preferences.html_question_back==0 && error!=NULL){
            preferences.html_question_back=16777215;
            error=NULL;
        }
	preferences.html_question_fore = gconf_client_get_int (config,"/gPHPEdit/html_question/fore",&error);
        if (error!=NULL){
            preferences.html_question_fore=7553165;
            error=NULL;
        }
	preferences.html_question_font = gconf_client_get_string(config,"/gPHPEdit/html_question/font",&error);
        if (!preferences.html_question_font){
            preferences.html_question_font="!Sans";
            error=NULL;
        }
	preferences.html_question_size = gconf_client_get_int (config,"/gPHPEdit/html_question/size",&error);
        if (preferences.html_question_size==0 && error!=NULL){
            preferences.html_question_size=12;
            error=NULL;
        }
	preferences.html_question_bold = gconf_client_get_bool(config,"/gPHPEdit/html_question/bold",NULL);
	preferences.html_question_italic = gconf_client_get_bool(config,"/gPHPEdit/html_question/italic",NULL);

	preferences.html_value_back = gconf_client_get_int (config,"/gPHPEdit/html_value/back",&error);
        if (preferences.html_value_back==0 && error!=NULL){
            preferences.html_value_back=16777215;
            error=NULL;
        }
	preferences.html_value_fore = gconf_client_get_int (config,"/gPHPEdit/html_script/fore",&error);
        if (preferences.html_value_back==0 && error!=NULL){
            preferences.html_value_fore=21632;
            error=NULL;
        }
	preferences.html_value_font = gconf_client_get_string(config,"/gPHPEdit/html_script/font",&error);
        if (!preferences.html_value_font){
            preferences.html_value_font="!Sans";
            error=NULL;
        }
	preferences.html_value_size = gconf_client_get_int (config,"/gPHPEdit/html_value/size",&error);
        if (preferences.html_value_back==0 && error!=NULL){
            preferences.html_value_size=12;
            error=NULL;
        }
	preferences.html_value_bold = gconf_client_get_bool(config,"/gPHPEdit/html_value/bold",NULL);
	preferences.html_value_italic = gconf_client_get_bool(config,"/gPHPEdit/html_value/italic",NULL);

	preferences.javascript_comment_back = gconf_client_get_int (config,"/gPHPEdit/javascript_comment/back",&error);
        if (preferences.javascript_comment_back==0 && error!=NULL){
            preferences.javascript_comment_back=16777215;
            error=NULL;
        }
	preferences.javascript_comment_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_comment/fore",&error);
       if (preferences.javascript_comment_fore==0 && error!=NULL){
            preferences.javascript_comment_fore=8421504;
            error=NULL;
        }
	preferences.javascript_comment_font = gconf_client_get_string(config,"/gPHPEdit/javascript_comment/font",&error);
        if (!preferences.javascript_comment_font){
            preferences.javascript_comment_font="!Sans";
            error=NULL;
        }
	preferences.javascript_comment_size = gconf_client_get_int (config,"/gPHPEdit/javascript_comment/size",&error);
        if (preferences.javascript_comment_size==0 && error!=NULL){
            preferences.javascript_comment_size=12;
            error=NULL;
        }
	preferences.javascript_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment/bold",NULL);
	preferences.javascript_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments/italic",NULL);

	preferences.javascript_comment_line_back = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_line/back",&error);
        if (preferences.javascript_comment_line_back==0 && error!=NULL){
            preferences.javascript_comment_line_back=16777215;
            error=NULL;
        }
	preferences.javascript_comment_line_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_line/fore",&error);
        if (preferences.javascript_comment_line_back==0 && error!=NULL){
            preferences.javascript_comment_line_back=8421504;
            error=NULL;
        }
	preferences.javascript_comment_line_font = gconf_client_get_string(config,"/gPHPEdit/javascript_comment_line/font",&error);
        if (!preferences.javascript_comment_line_font){
            preferences.javascript_comment_line_font="!Sans";
            error=NULL;
        }
	preferences.javascript_comment_line_size = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_line/size",&error);
        if (preferences.javascript_comment_line_size==0 && error!=NULL){
            preferences.javascript_comment_line_size=12;
            error=NULL;
        }
	preferences.javascript_comment_line_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment_line/bold",NULL);
	preferences.javascript_comment_line_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment_line/italic",NULL);

	preferences.javascript_comment_doc_back = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_doc/back",&error);
        if (preferences.javascript_comment_doc_back==0 && error!=NULL){
            preferences.javascript_comment_doc_back=16777215;
            error=NULL;
        }
	preferences.javascript_comment_doc_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_doc/fore",&error);
        if (preferences.javascript_comment_doc_fore==0 && error!=NULL){
            preferences.javascript_comment_line_fore=8355712;
            error=NULL;
        }
	preferences.javascript_comment_doc_font = gconf_client_get_string(config,"/gPHPEdit/javascript_comment_doc/font",&error);
        if (!preferences.javascript_comment_doc_font){
            preferences.javascript_comment_doc_font="!Sans";
            error=NULL;
        }
	preferences.javascript_comment_doc_size = gconf_client_get_int (config,"/gPHPEdit/javascript_comment_doc/size",&error);
        if (preferences.javascript_comment_doc_size==0 && error!=NULL){
            preferences.javascript_comment_doc_size=12;
            error=NULL;
        }
        preferences.javascript_comment_doc_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments_doc/bold",NULL);
	preferences.javascript_comment_doc_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments_doc/italic",NULL);

	preferences.javascript_word_back = gconf_client_get_int (config,"/gPHPEdit/javascript_word/back",&error);
        if (preferences.javascript_word_back==0 && error!=NULL){
            preferences.javascript_word_back=16777215;
            error=NULL;
        }
	preferences.javascript_word_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_word/fore",&error);
        if (preferences.javascript_word_fore==0 && error!=NULL){
            preferences.javascript_word_fore=9204544;
            error=NULL;
        }
	preferences.javascript_word_font = gconf_client_get_string(config,"/gPHPEdit/javascript_word/font",&error);
        if (!preferences.javascript_word_font){
            preferences.javascript_word_font="!Sans";
            error=NULL;
        }
	preferences.javascript_word_size = gconf_client_get_int (config,"/gPHPEdit/javascript_word/size",&error);
        if (preferences.javascript_word_size==0 && error!=NULL){
            preferences.javascript_word_size=12;
            error=NULL;
        }
	preferences.javascript_word_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_word/bold",NULL);
	preferences.javascript_word_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_word/italic",NULL);

	preferences.javascript_keyword_back = gconf_client_get_int (config,"/gPHPEdit/javascript_keyword/back",&error);
        if (preferences.javascript_keyword_back==0 && error!=NULL){
            preferences.javascript_keyword_back=16777215;
            error=NULL;
        }
	preferences.javascript_keyword_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_keyword/fore",&error);
        if (preferences.javascript_keyword_fore==0 && error!=NULL){
            preferences.javascript_keyword_fore=8388608;
            error=NULL;
        }
	preferences.javascript_keyword_font = gconf_client_get_string(config,"/gPHPEdit/javascript_keyword/font",&error);
        if (!preferences.javascript_keyword_font){
            preferences.javascript_keyword_font="!Sans";
            error=NULL;
        }
	preferences.javascript_keyword_size = gconf_client_get_int (config,"/gPHPEdit/javascript_keyword/size",&error);
        if (preferences.javascript_keyword_size==0 && error!=NULL){
            preferences.javascript_keyword_size=12;
            error=NULL;
        }
	preferences.javascript_keyword_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_keyword/bold",NULL);
	preferences.javascript_keyword_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_keyword/italic",NULL);

	preferences.javascript_doublestring_back = gconf_client_get_int (config,"/gPHPEdit/javascript_doublestring/back",&error);
        if (preferences.javascript_doublestring_back==0 && error!=NULL){
            preferences.javascript_doublestring_back=16777215;
            error=NULL;
        }
	preferences.javascript_doublestring_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_doublestring/fore",&error);
        if (preferences.javascript_doublestring_fore==0 && error!=NULL){
            preferences.javascript_doublestring_fore=8388608;
            error=NULL;
        }
	preferences.javascript_doublestring_font = gconf_client_get_string(config,"/gPHPEdit/javascript_doublestring/font",&error);
        if (!preferences.javascript_doublestring_font){
            preferences.javascript_doublestring_font="!Sans";
            error=NULL;
        }
	preferences.javascript_doublestring_size = gconf_client_get_int (config,"/gPHPEdit/javascript_doublestring/size",&error);
        if (preferences.javascript_doublestring_size==0 && error!=NULL){
            preferences.javascript_doublestring_size=12;
            error=NULL;
        }
	preferences.javascript_doublestring_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_doublestring/bold",NULL);
	preferences.javascript_doublestring_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_doublestring/italic",NULL);

	preferences.javascript_singlestring_back = gconf_client_get_int (config,"/gPHPEdit/javascript_singlestring/back",&error);
        if (preferences.javascript_singlestring_back==0 && error!=NULL){
            preferences.javascript_singlestring_back=16777215;
            error=NULL;
        }
	preferences.javascript_singlestring_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_singlestring/fore",&error);
         if (preferences.javascript_singlestring_fore==0 && error!=NULL){
            preferences.javascript_singlestring_fore=8388608;
            error=NULL;
        }
	preferences.javascript_singlestring_font = gconf_client_get_string(config,"/gPHPEdit/javascript_singlestring/font",&error);
        if (!preferences.javascript_singlestring_font){
            preferences.javascript_singlestring_font="!Sans";
            error=NULL;
        }
	preferences.javascript_singlestring_size = gconf_client_get_int (config,"/gPHPEdit/javascript_singlestring/size",&error);
        if (preferences.javascript_singlestring_size==0 && error!=NULL){
            preferences.javascript_singlestring_size=12;
            error=NULL;
        }
	preferences.javascript_singlestring_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_singlestring/bold",NULL);
	preferences.javascript_singlestring_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_singlestring/bold",NULL);

	preferences.javascript_symbols_back = gconf_client_get_int (config,"/gPHPEdit/javascript_symbols/back",&error);
        if (preferences.javascript_symbols_back==0 && error!=NULL){
            preferences.javascript_symbols_back=16777215;
            error=NULL;
        }
	preferences.javascript_symbols_fore = gconf_client_get_int (config,"/gPHPEdit/javascript_symbols/fore",&error);
         if (preferences.javascript_symbols_fore==0 && error!=NULL){
            preferences.javascript_symbols_fore=8355712;
            error=NULL;
        }
	preferences.javascript_symbols_font = gconf_client_get_string(config,"/gPHPEdit/javascript_symbols/font",&error);
        if (!preferences.javascript_symbols_font){
            preferences.javascript_symbols_font="!Sans";
            error=NULL;
        }
	preferences.javascript_symbols_size = gconf_client_get_int (config,"/gPHPEdit/javascript_symbols/size",&error);
        if (preferences.javascript_symbols_size==0 && error!=NULL){
            preferences.javascript_symbols_size=12;
            error=NULL;
        }
	preferences.javascript_symbols_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_symbols/bold",NULL);
	preferences.javascript_symbols_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_symbols/italic",NULL);

	preferences.php_default_font = gconf_client_get_string(config,"/gPHPEdit/php_default_style/font",&error);
        if (!preferences.php_default_font){
            preferences.php_default_font="!Sans";
            error=NULL;
        }
	preferences.php_default_fore = gconf_client_get_int (config,"/gPHPEdit/php_default_style/fore",&error);
        if (preferences.php_default_fore==0 && error!=NULL){
            preferences.php_default_fore=1052688;
            error=NULL;
        }
	preferences.php_default_back = gconf_client_get_int (config,"/gPHPEdit/php_default_style/back",&error);
        if (preferences.php_default_back==0 && error!=NULL){
            preferences.php_default_back=16777215;
            error=NULL;
        }
	preferences.php_default_size = gconf_client_get_int (config,"/gPHPEdit/php_default_style/size",&error);
        if (preferences.php_default_size==0 && error!=NULL){
            preferences.php_default_size=12;
            error=NULL;
        }
	preferences.php_default_bold = gconf_client_get_bool(config,"/gPHPEdit/php_default_style/bold",NULL);
	preferences.php_default_italic = gconf_client_get_bool(config,"/gPHPEdit/php_default_style/italic",NULL);

	preferences.php_hstring_fore = gconf_client_get_int (config,"/gPHPEdit/php_hstring/fore",&error);
         if (preferences.php_hstring_fore==0 && error!=NULL){
            preferences.php_hstring_fore=8388736;
            error=NULL;
        }
	preferences.php_hstring_font = gconf_client_get_string(config,"/gPHPEdit/php_hstring/font",&error);
        if (!preferences.php_hstring_font){
            preferences.php_hstring_font="!Sans";
            error=NULL;
        }
	preferences.php_hstring_back = gconf_client_get_int (config,"/gPHPEdit/php_hstring/back",&error);
        if (preferences.php_hstring_back==0 && error!=NULL){
            preferences.php_hstring_back=16777215;
            error=NULL;
        }
	preferences.php_hstring_size = gconf_client_get_int (config,"/gPHPEdit/php_hstring/size",&error);
        if (preferences.php_hstring_size==0 && error!=NULL){
            preferences.php_hstring_size=12;
            error=NULL;
        }
	preferences.php_hstring_bold = gconf_client_get_bool(config,"/gPHPEdit/php_hstring/bold",NULL);
	preferences.php_hstring_italic = gconf_client_get_bool(config,"/gPHPEdit/php_hstring/italic",NULL);

	preferences.php_simplestring_fore = gconf_client_get_int (config,"/gPHPEdit/php_simplestring/fore",&error);
        if (preferences.php_simplestring_fore==0 && error!=NULL){
            preferences.php_simplestring_fore=8388736;
            error=NULL;
        }
	preferences.php_simplestring_font = gconf_client_get_string(config,"/gPHPEdit/php_simplestring/font",&error);
        if (!preferences.php_simplestring_font){
            preferences.php_simplestring_font="!Sans";
            error=NULL;
        }
	preferences.php_simplestring_back = gconf_client_get_int (config,"/gPHPEdit/php_simplestring/back",&error);
        if (preferences.php_simplestring_back==0 && error!=NULL){
            preferences.php_simplestring_back=16777215;
            error=NULL;
        }
	preferences.php_simplestring_size = gconf_client_get_int (config,"/gPHPEdit/php_simplestring/size",&error);
        if (preferences.php_simplestring_size==0 && error!=NULL){
            preferences.php_simplestring_size=12;
            error=NULL;
        }
	preferences.php_simplestring_bold = gconf_client_get_bool(config,"/gPHPEdit/php_simplestring/bold",NULL);
	preferences.php_simplestring_italic = gconf_client_get_bool(config,"/gPHPEdit/php_simplestring/italic",NULL);

	preferences.php_word_fore = gconf_client_get_int (config,"/gPHPEdit/php_word/fore",NULL);
	preferences.php_word_font = gconf_client_get_string(config,"/gPHPEdit/php_simplestring/font",&error);
        if (!preferences.php_word_font){
            preferences.php_word_font="!Sans";
            error=NULL;
        }
	preferences.php_word_back = gconf_client_get_int (config,"/gPHPEdit/php_word/back",&error);
        if (preferences.php_word_back==0 && error!=NULL){
            preferences.php_word_back=16777215;
            error=NULL;
        }
	preferences.php_word_size = gconf_client_get_int (config,"/gPHPEdit/php_word/size",&error);
        if (preferences.php_word_size==0 && error!=NULL){
            preferences.php_word_size=12;
            error=NULL;
        }
	preferences.php_word_bold = gconf_client_get_bool(config,"/gPHPEdit/php_word/bold",NULL);
	preferences.php_word_italic = gconf_client_get_bool(config,"/gPHPEdit/php_word/italic",NULL);

	preferences.php_number_fore = gconf_client_get_int (config,"/gPHPEdit/php_number/fore",&error);
        if (preferences.php_number_fore==0 && error!=NULL){
            preferences.php_number_fore=9204544;
            error=NULL;
        }
	preferences.php_number_font = gconf_client_get_string(config,"/gPHPEdit/php_number/font",&error);
        if (!preferences.php_number_font){
            preferences.php_number_font="!Sans";
            error=NULL;
        }
	preferences.php_number_back = gconf_client_get_int (config,"/gPHPEdit/php_number/back",&error);
        if (preferences.php_number_back==0 && error!=NULL){
            preferences.php_number_back=16777215;
            error=NULL;
        }
	preferences.php_number_size = gconf_client_get_int (config,"/gPHPEdit/php_number/size",&error);
        if (preferences.php_number_size==0 && error!=NULL){
            preferences.php_number_size=12;
            error=NULL;
        }
	preferences.php_number_bold = gconf_client_get_bool(config,"/gPHPEdit/php_number/bold",NULL);
	preferences.php_number_italic = gconf_client_get_bool(config,"/gPHPEdit/php_number/italic",NULL);

	preferences.php_variable_fore = gconf_client_get_int (config,"/gPHPEdit/php_variable/fore",&error);
        if (preferences.php_variable_fore==0 && error!=NULL){
            preferences.php_variable_fore=16746496;
            error=NULL;
        }
	preferences.php_variable_font = gconf_client_get_string(config,"/gPHPEdit/php_variable/font",&error);
        if (!preferences.php_variable_font){
            preferences.php_variable_font="!Sans";
            error=NULL;
        }
	preferences.php_variable_back = gconf_client_get_int (config,"/gPHPEdit/php_variable/back",&error);
        if (preferences.php_variable_back==0 && error!=NULL){
            preferences.php_variable_back=16777215;
            error=NULL;
        }
	preferences.php_variable_size = gconf_client_get_int (config,"/gPHPEdit/php_variable/size",&error);
        if (preferences.php_variable_size==0 && error!=NULL){
            preferences.php_variable_size=12;
            error=NULL;
        }
	preferences.php_variable_bold = gconf_client_get_bool(config,"/gPHPEdit/php_variable/bold",NULL);
	preferences.php_variable_italic = gconf_client_get_bool(config,"/gPHPEdit/php_variable/italic",NULL);

	preferences.php_comment_fore = gconf_client_get_int (config,"/gPHPEdit/php_comment/fore",&error);
        if (preferences.php_comment_fore==0 && error!=NULL){
            preferences.php_comment_fore=8421594;
            error=NULL;
        }
	preferences.php_comment_font = gconf_client_get_string(config,"/gPHPEdit/php_comment/font",&error);
        if (!preferences.php_comment_font){
            preferences.php_comment_font="!Sans";
            error=NULL;
        }
	preferences.php_comment_back = gconf_client_get_int (config,"/gPHPEdit/php_comment/back",&error);
        if (preferences.php_comment_back==0 && error!=NULL){
            preferences.php_comment_back=16777215;
            error=NULL;
        }
	preferences.php_comment_size = gconf_client_get_int (config,"/gPHPEdit/php_comment/size",&error);
        if (preferences.php_comment_size==0 && error!=NULL){
            preferences.php_comment_size=12;
            error=NULL;
        }
	preferences.php_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/php_comment/bold",NULL);
	preferences.php_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/php_comment/italic",NULL);

	preferences.php_comment_line_fore = gconf_client_get_int (config,"/gPHPEdit/php_comment_line/fore",&error);
        if (preferences.php_comment_line_fore==0 && error!=NULL){
            preferences.php_comment_line_fore=8421504;
            error=NULL;
        }
	preferences.php_comment_line_font = gconf_client_get_string(config,"/gPHPEdit/php_comment_line/font",&error);
        if (!preferences.php_comment_line_font){
            preferences.php_comment_line_font="!Sans";
            error=NULL;
        }
	preferences.php_comment_line_back = gconf_client_get_int (config,"/gPHPEdit/php_comment_line/back",&error);
        if (preferences.php_comment_line_back==0 && error!=NULL){
            preferences.php_comment_line_back=16777215;
            error=NULL;
        }
	preferences.php_comment_line_size = gconf_client_get_int (config,"/gPHPEdit/php_comment_line/back",&error);
        if (preferences.php_comment_line_size==0 && error!=NULL){
            preferences.php_comment_line_size=12;
            error=NULL;
        }
	preferences.php_comment_line_bold = gconf_client_get_bool(config,"/gPHPEdit/php_comment_line/bold",NULL);
	preferences.php_comment_line_italic = gconf_client_get_bool(config,"/gPHPEdit/php_comment_line/italic",NULL);

	preferences.css_tag_back = gconf_client_get_int (config,"/gPHPEdit/css_tag/back",&error);
         if (preferences.css_tag_back==0 && error!=NULL){
            preferences.css_tag_back=16777215;
            error=NULL;
        }
	preferences.css_tag_font = gconf_client_get_string(config,"/gPHPEdit/css_tag/font",&error);
        if (!preferences.css_tag_font){
            preferences.css_tag_font="!Sans";
            error=NULL;
        }
	preferences.css_tag_fore = gconf_client_get_int (config,"/gPHPEdit/css_tag/fore",&error);
        if (preferences.css_tag_fore==0 && error!=NULL){
            preferences.css_tag_fore=8388608;
            error=NULL;
        }
	preferences.css_tag_size = gconf_client_get_int (config,"/gPHPEdit/css_tag/size",&error);
        if (preferences.css_tag_size==0 && error!=NULL){
            preferences.css_tag_size=12;
            error=NULL;
        }
	preferences.css_tag_bold = gconf_client_get_bool(config,"/gPHPEdit/css_tag/bold",NULL);
	preferences.css_tag_italic = gconf_client_get_bool(config,"/gPHPEdit/css_tag/italic",NULL);
	
	preferences.css_class_fore = gconf_client_get_int (config,"/gPHPEdit/css_class/fore",&error);
        if (preferences.css_class_fore==0 && error!=NULL){
            preferences.css_class_fore=8388608;
            error=NULL;
        }
	preferences.css_class_font = gconf_client_get_string(config,"/gPHPEdit/css_class/font",&error);
        if (!preferences.css_class_font){
            preferences.css_class_font="!Sans";
            error=NULL;
        }
	preferences.css_class_back = gconf_client_get_int (config,"/gPHPEdit/css_class/back",&error);
        if (preferences.css_class_back==0 && error!=NULL){
            preferences.css_class_back=16777215;
            error=NULL;
        }
	preferences.css_class_size = gconf_client_get_int (config,"/gPHPEdit/css_class/size",&error);
        if (preferences.css_class_size==0 && error!=NULL){
            preferences.css_class_size=12;
            error=NULL;
        }
	preferences.css_class_bold = gconf_client_get_bool(config,"/gPHPEdit/css_class/bold",NULL);
	preferences.css_class_italic = gconf_client_get_bool(config,"/gPHPEdit/css_class/italic",NULL);
	
	preferences.css_pseudoclass_back = gconf_client_get_int (config,"/gPHPEdit/css_pseudoclass/back",&error);
        if (preferences.css_pseudoclass_back==0 && error!=NULL){
            preferences.css_pseudoclass_back=16777215;
            error=NULL;
        }
	preferences.css_pseudoclass_font = gconf_client_get_string(config,"/gPHPEdit/css_pseudoclass/font",&error);
        if (!preferences.css_pseudoclass_font){
            preferences.css_pseudoclass_font="!Sans";
            error=NULL;
        }
	preferences.css_pseudoclass_fore = gconf_client_get_int (config,"/gPHPEdit/css_pseudoclass/fore",&error);
        if (preferences.css_pseudoclass_fore==0 && error!=NULL){
            preferences.css_pseudoclass_fore=8388608;
            error=NULL;
        }
	preferences.css_pseudoclass_size = gconf_client_get_int (config,"/gPHPEdit/css_pseudoclass/size",&error);
        if (preferences.css_pseudoclass_size==0 && error!=NULL){
            preferences.css_pseudoclass_size=12;
            error=NULL;
        }
	preferences.css_pseudoclass_bold = gconf_client_get_bool(config,"/gPHPEdit/css_pseudoclass/bold",NULL);
	preferences.css_pseudoclass_italic = gconf_client_get_bool(config,"/gPHPEdit/css_pseudoclass/italic",NULL);
	
	preferences.css_unknown_pseudoclass_fore = gconf_client_get_int (config,"/gPHPEdit/css_unknown_line/fore",&error);
        if (preferences.css_unknown_pseudoclass_fore==0 && error!=NULL){
            preferences.css_unknown_pseudoclass_fore=16711680;
            error=NULL;
        }
	preferences.css_unknown_pseudoclass_font = gconf_client_get_string(config,"/gPHPEdit/css_unknown_line/font",&error);
        if (!preferences.css_unknown_pseudoclass_font){
            preferences.css_unknown_pseudoclass_font="!Sans";
            error=NULL;
        }
	preferences.css_unknown_pseudoclass_back = gconf_client_get_int (config,"/gPHPEdit/css_unknown_line/back",&error);
        if (preferences.css_unknown_pseudoclass_back==0 && error!=NULL){
            preferences.css_unknown_pseudoclass_back=16777215;
            error=NULL;
        }
	preferences.css_unknown_pseudoclass_size = gconf_client_get_int (config,"/gPHPEdit/css_unknown_line/size",&error);
        if (preferences.css_pseudoclass_size==0 && error!=NULL){
            preferences.css_unknown_pseudoclass_size=12;
            error=NULL;
        }
	preferences.css_unknown_pseudoclass_bold = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_line/bold",NULL);
	preferences.css_unknown_pseudoclass_italic = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_line/italic",NULL);
	
	preferences.css_operator_fore = gconf_client_get_int (config,"/gPHPEdit/css_operator/fore",&error);
        if (preferences.css_operator_fore==0 && error!=NULL){
            preferences.css_operator_fore=128;
            error=NULL;
        }
	preferences.css_operator_font = gconf_client_get_string(config,"/gPHPEdit/css_operator/font",&error);
        if (!preferences.css_operator_font){
            preferences.css_operator_font="!Sans";
            error=NULL;
        }
	preferences.css_operator_back = gconf_client_get_int (config,"/gPHPEdit/css_operator/back",&error);
        if (preferences.css_operator_back==0 && error!=NULL){
            preferences.css_operator_back=16777215;
            error=NULL;
        }
	preferences.css_operator_size = gconf_client_get_int (config,"/gPHPEdit/css_operator/size",&error);
        if (preferences.css_operator_size==0 && error!=NULL){
            preferences.css_operator_size=12;
            error=NULL;
        }
	preferences.css_operator_bold = gconf_client_get_bool(config,"/gPHPEdit/css_operator/bold",NULL);
	preferences.css_operator_italic = gconf_client_get_bool(config,"/gPHPEdit/css_operator/italic",NULL);
	
	preferences.css_identifier_fore = gconf_client_get_int (config,"/gPHPEdit/css_identifier/fore",NULL);
	preferences.css_identifier_font = gconf_client_get_string(config,"/gPHPEdit/css_identifier/font",&error);
        if (!preferences.css_identifier_font){
            preferences.css_identifier_font="!Sans";
            error=NULL;
        }
	preferences.css_identifier_back = gconf_client_get_int (config,"/gPHPEdit/css_identifier/back",&error);
        if (preferences.css_identifier_back==0 && error!=NULL){
            preferences.css_identifier_back=16777215;
            error=NULL;
        }
	preferences.css_identifier_size = gconf_client_get_int (config,"/gPHPEdit/css_identifier/size",&error);
        if (preferences.css_identifier_size==0 && error!=NULL){
            preferences.css_identifier_size=12;
            error=NULL;
        }
	preferences.css_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/css_identifier/bold",NULL);
	preferences.css_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/css_identifier/italic",NULL);
	
	preferences.css_unknown_identifier_fore = gconf_client_get_int (config,"/gPHPEdit/css_unknown_identifier/fore",&error);
        if (preferences.css_unknown_identifier_fore==0 && error!=NULL){
            preferences.css_unknown_identifier_fore=16711680;
            error=NULL;
        }
	preferences.css_unknown_identifier_font = gconf_client_get_string(config,"/gPHPEdit/css_unknown_identifier/font",&error);
        if (!preferences.css_unknown_identifier_font){
            preferences.css_unknown_identifier_font="!Sans";
            error=NULL;
        }
	preferences.css_unknown_identifier_back = gconf_client_get_int (config,"/gPHPEdit/css_unknown_identifier/back",&error);
        if (preferences.css_unknown_identifier_back==0 && error!=NULL){
            preferences.css_unknown_identifier_back=16777215;
            error=NULL;
        }
	preferences.css_unknown_identifier_size = gconf_client_get_int (config,"/gPHPEdit/css_unknown_identifier/size",&error);
        if (preferences.css_unknown_identifier_size==0 && error!=NULL){
            preferences.css_unknown_identifier_size=12;
            error=NULL;
        }
	preferences.css_unknown_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_identifier/bold",NULL);
	preferences.css_unknown_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_identifier/italic",NULL);
	
	preferences.css_value_fore = gconf_client_get_int (config,"/gPHPEdit/css_value/fore",&error);
        if (preferences.css_value_fore==0 && error!=NULL){
            preferences.css_value_fore=8388736;
            error=NULL;
        }
	preferences.css_value_font = gconf_client_get_string(config,"/gPHPEdit/css_value/font",&error);
        if (!preferences.css_value_font){
            preferences.css_value_font="!Sans";
            error=NULL;
        }
	preferences.css_value_back = gconf_client_get_int (config,"/gPHPEdit/css_value/back",&error);
        if (preferences.css_value_back==0 && error!=NULL){
            preferences.css_value_back=16777215;
            error=NULL;
        }
	preferences.css_value_size = gconf_client_get_int (config,"/gPHPEdit/css_value/size",&error);
        if (preferences.css_value_size==0 && error!=NULL){
            preferences.css_value_size=12;
            error=NULL;
        }
	preferences.css_value_bold = gconf_client_get_bool(config,"/gPHPEdit/css_value/bold",NULL);
	preferences.css_value_italic = gconf_client_get_bool(config,"/gPHPEdit/css_value/italic",NULL);
	
	preferences.css_comment_fore = gconf_client_get_int (config,"/gPHPEdit/css_comment/fore",&error);
        if (preferences.css_comment_fore==0 && error!=NULL){
            preferences.css_comment_fore=84215504;
            error=NULL;
        }
	preferences.css_comment_font = gconf_client_get_string(config,"/gPHPEdit/css_comment/font",&error);
        if (!preferences.css_comment_font){
            preferences.css_comment_font="!Sans";
            error=NULL;
        }
	preferences.css_comment_back = gconf_client_get_int (config,"/gPHPEdit/css_comment/back",&error);
        if (preferences.css_comment_back==0 && error!=NULL){
            preferences.css_comment_back=16777215;
            error=NULL;
        }
	preferences.css_comment_size = gconf_client_get_int (config,"/gPHPEdit/css_comment/size",&error);
        if (preferences.css_comment_size==0 && error!=NULL){
            preferences.css_comment_size=12;
            error=NULL;
        }
	preferences.css_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/css_comment/bold",NULL);
	preferences.css_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/css_comment/italic",NULL);
	
	preferences.css_id_font = gconf_client_get_string(config,"/gPHPEdit/css_id/font",&error);
        if (!preferences.css_id_font){
            preferences.css_id_font="!Sans";
            error=NULL;
        }
	preferences.css_id_fore = gconf_client_get_int (config,"/gPHPEdit/css_id/fore",&error);
        if (preferences.css_id_fore==0 && error!=NULL){
            preferences.css_id_fore=8388608;
            error=NULL;
        }
	preferences.css_id_back = gconf_client_get_int (config,"/gPHPEdit/css_id/back",&error);
        if (preferences.css_id_back==0 && error!=NULL){
            preferences.css_id_back=16777215;
            error=NULL;
        }
	preferences.css_id_size = gconf_client_get_int (config,"/gPHPEdit/css_id/size",&error);
        if (preferences.css_id_size==0 && error!=NULL){
            preferences.css_id_size=12;
            error=NULL;
        }
	preferences.css_id_bold = gconf_client_get_bool(config,"/gPHPEdit/css_id/bold",NULL);
	preferences.css_id_italic = gconf_client_get_bool(config,"/gPHPEdit/css_id/italic",NULL);
	
	preferences.css_important_font = gconf_client_get_string(config,"/gPHPEdit/css_important/font",&error);
        if (!preferences.css_important_font){
            preferences.css_important_font="!Sans";
            error=NULL;
        }
	preferences.css_important_fore = gconf_client_get_int (config,"/gPHPEdit/css_important/fore",&error);
        if (preferences.css_important_fore==0 && error!=NULL){
            preferences.css_important_fore=255;
            error=NULL;
        }
	preferences.css_important_back = gconf_client_get_int (config,"/gPHPEdit/css_important/back",&error);
        if (preferences.css_important_back==0 && error!=NULL){
            preferences.css_important_back=16777215;
            error=NULL;
        }
	preferences.css_important_size = gconf_client_get_int (config,"/gPHPEdit/css_important/size",&error);
        if (preferences.css_important_size==0 && error!=NULL){
            preferences.css_important_size=12;
            error=NULL;
        }
	preferences.css_important_bold = gconf_client_get_bool(config,"/gPHPEdit/css_important/bold",NULL);
	preferences.css_important_italic = gconf_client_get_bool(config,"/gPHPEdit/css_important/italic",NULL);
	
	preferences.css_directive_fore = gconf_client_get_int (config,"/gPHPEdit/css_directive/fore",&error);
        if (preferences.css_directive_fore==0 && error!=NULL){
            preferences.css_directive_fore=32768;
            error=NULL;
        }
	preferences.css_directive_font = gconf_client_get_string(config,"/gPHPEdit/css_directive/font",&error);
        if (!preferences.css_directive_font){
            preferences.css_directive_font="!Sans";
            error=NULL;
        }
	preferences.css_directive_back = gconf_client_get_int (config,"/gPHPEdit/css_directive/back",&error);
        if (preferences.css_directive_back==0 && error!=NULL){
            preferences.css_directive_back=16777215;
            error=NULL;
        }
	preferences.css_directive_size = gconf_client_get_int (config,"/gPHPEdit/css_directive/size",&error);
        if (preferences.css_directive_size==0 && error!=NULL){
            preferences.css_directive_size=12;
            error=NULL;
        }
	preferences.css_directive_bold = gconf_client_get_bool(config,"/gPHPEdit/css_directive/bold",NULL);
	preferences.css_directive_italic = gconf_client_get_bool(config,"/gPHPEdit/css_directive/italic",NULL);

        
	preferences.sql_word_fore = gconf_client_get_int (config,"/gPHPEdit/sql_word/fore",NULL);
        preferences.sql_word_font = gconf_client_get_string(config,"/gPHPEdit/sql_word/font",&error);
        if (!preferences.sql_word_font){
            preferences.sql_word_font="!Sans";
            error=NULL;
        }
        preferences.sql_word_back = gconf_client_get_int (config,"/gPHPEdit/sql_word/back",&error);
        if (preferences.sql_word_back==0 && error!=NULL){
            preferences.sql_word_back=16777215;
            error=NULL;
        }
	preferences.sql_word_size = gconf_client_get_int (config,"/gPHPEdit/sql_word/size",&error);
        if (preferences.sql_word_size==0 && error!=NULL){
            preferences.sql_word_size=12;
            error=NULL;
        }
	preferences.sql_word_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_word/bold",NULL);
	preferences.sql_word_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_word/italic",NULL);
        
	preferences.sql_string_fore = gconf_client_get_int (config,"/gPHPEdit/sql_string/fore",&error);
        if (preferences.sql_string_fore==0 && error!=NULL){
            preferences.sql_string_fore=8388736;
            error=NULL;
        }
	preferences.sql_string_font = gconf_client_get_string(config,"/gPHPEdit/sql_string/font",&error);
        if (!preferences.sql_string_font){
            preferences.sql_string_font="!Sans";
            error=NULL;
        }
	preferences.sql_string_back = gconf_client_get_int (config,"/gPHPEdit/sql_string/back",&error);
        if (preferences.sql_string_back==0 && error!=NULL){
            preferences.sql_string_back=16777215;
            error=NULL;
        }
	preferences.sql_string_size = gconf_client_get_int (config,"/gPHPEdit/sql_string/size",&error);
        if (preferences.sql_string_size==0 && error!=NULL){
            preferences.sql_string_size=12;
            error=NULL;
        }
	preferences.sql_string_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_string/bold",NULL);
	preferences.sql_string_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_string/italic",NULL);
	
	preferences.sql_operator_fore = gconf_client_get_int (config,"/gPHPEdit/sql_operator/fore",NULL);
	preferences.sql_operator_font = gconf_client_get_string(config,"/gPHPEdit/sql_operator/font",&error);
        if (!preferences.sql_operator_font){
            preferences.sql_operator_font="!Sans";
            error=NULL;
        }
	preferences.sql_operator_back = gconf_client_get_int (config,"/gPHPEdit/sql_operator/back",&error);
        if (preferences.sql_operator_back==0 && error!=NULL){
            preferences.sql_operator_fore=16777215;
            error=NULL;
        }
	preferences.sql_operator_size = gconf_client_get_int (config,"/gPHPEdit/sql_operator/size",&error);
        if (preferences.sql_operator_size==0 && error!=NULL){
            preferences.sql_operator_size=12;
            error=NULL;
        }
	preferences.sql_operator_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_operator/bold",NULL);
	preferences.sql_operator_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_operator/italic",NULL);
	
	preferences.sql_comment_fore = gconf_client_get_int (config,"/gPHPEdit/sql_comment/fore",&error);
        if (preferences.sql_comment_fore==0 && error!=NULL){
            preferences.sql_comment_fore=8421504;
            error=NULL;
        }
	preferences.sql_comment_font = gconf_client_get_string(config,"/gPHPEdit/sql_comment/font",&error);
        if (!preferences.sql_comment_font){
            preferences.sql_comment_font="!Sans";
            error=NULL;
        }
	preferences.sql_comment_back = gconf_client_get_int (config,"/gPHPEdit/sql_comment/back",&error);
        if (preferences.sql_comment_back==0 && error!=NULL){
            preferences.sql_comment_back=16777215;
            error=NULL;
        }
	preferences.sql_comment_size = gconf_client_get_int (config,"/gPHPEdit/sql_comment/size",&error);
        if (preferences.sql_comment_size==0 && error!=NULL){
            preferences.sql_comment_size=12;
            error=NULL;
        }
	preferences.sql_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_comment/bold",NULL);
	preferences.sql_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_comment/italic",NULL);
	
	preferences.sql_number_fore = gconf_client_get_int (config,"/gPHPEdit/sql_number/fore",&error);
        if (preferences.sql_number_fore==0 && error!=NULL){
            preferences.sql_number_fore=9204544;
            error=NULL;
        }
	preferences.sql_number_font = gconf_client_get_string(config,"/gPHPEdit/sql_number/font",&error);
        if (!preferences.sql_number_font){
            preferences.sql_number_font="!Sans";
            error=NULL;
        }
	preferences.sql_number_back = gconf_client_get_int (config,"/gPHPEdit/sql_number/back",&error);
        if (preferences.sql_number_back==0 && error!=NULL){
            preferences.sql_number_back=16777215;
            error=NULL;
        }
	preferences.sql_number_size = gconf_client_get_int (config,"/gPHPEdit/sql_number/size",&error);
        if (preferences.sql_number_size==0 &&error!=NULL){
            preferences.sql_number_size=12;
            error=NULL;
        }
	preferences.sql_number_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_number/bold",NULL);
	preferences.sql_number_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_number/italic",NULL);
	
	preferences.sql_identifier_fore = gconf_client_get_int (config,"/gPHPEdit/sql_identifier/fore",&error);
        if (preferences.sql_identifier_fore==0 &&error!=NULL){
            preferences.sql_identifier_fore=16746496;
            error=NULL;
        }
	preferences.sql_identifier_font = gconf_client_get_string(config,"/gPHPEdit/sql_identifier/font",&error);
        if (!preferences.sql_identifier_font){
            preferences.sql_identifier_font="!Sans";
            error=NULL;
        }
	preferences.sql_identifier_back = gconf_client_get_int (config,"/gPHPEdit/sql_identifier/back",&error);
        if (preferences.sql_identifier_back==0 && error!=NULL){
            preferences.sql_identifier_fore=16777215;
            error=NULL;
        }
	preferences.sql_identifier_size = gconf_client_get_int (config,"/gPHPEdit/sql_identifier/size",&error);
        if (preferences.sql_identifier_size==0 && error!=NULL){
            preferences.sql_identifier_size=12;
            error=NULL;
        }
	preferences.sql_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_identifier/bold",NULL);
	preferences.sql_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_identifier/italic",NULL);

        check_for_pango_fonts();
}


void move_classbrowser_position(void)
{
    GConfClient *config;
    config=gconf_client_get_default ();
    GError *error = NULL;
    gint pos;
    pos = gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_size",&error);
        if (error!=NULL){
            pos=100;
        }
    gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane),pos);
}

void save_classbrowser_position(void)
{
    
    if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)) != 0) {
        GConfClient *config;
        config=gconf_client_get_default ();
         gconf_client_set_int (config,"/gPHPEdit/main_window/classbrowser_size", gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)),NULL);
	}
 }

void main_window_size_save_details()
{
        GError *error = NULL;
        GConfClient *config;
        config=gconf_client_get_default ();
        preferences.maximized = gconf_client_get_bool(config,"/gPHPEdit/main_window/maximized",NULL);
	
	if (!preferences.maximized) {
		gtk_window_get_position(GTK_WINDOW(main_window.window), &preferences.left, &preferences.top);
		gtk_window_get_size(GTK_WINDOW(main_window.window), &preferences.width, &preferences.height);

                gconf_client_set_int (config,"/gPHPEdit/main_window/x", preferences.left,NULL);
                gconf_client_set_int (config,"/gPHPEdit/main_window/y", preferences.top,NULL);
                gconf_client_set_int (config,"/gPHPEdit/main_window/width", preferences.width,NULL);
		gconf_client_set_int (config,"/gPHPEdit/main_window/height", preferences.height,NULL);
	}
}

void preferences_save()
{
        GString *uri;
        uri = g_string_new ("");
        g_string_printf(uri,"%s/%s",g_get_home_dir(),".gphpedit/");
        GConfClient *config;
        config=gconf_client_get_default ();
        //gconf_client_add_dir(config,CONFIG_ROOT,GCONF_CLIENT_PRELOAD_NONE,NULL);

        gconf_client_set_string(config, "/gPHPEdit/default_style/font",preferences.default_font,NULL);
	gconf_client_set_int (config, "/gPHPEdit/default_style/fore", preferences.default_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/default_style/back", preferences.default_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/default_style/size", preferences.default_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/default_style/italic", preferences.default_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/default_style/bold", preferences.default_bold,NULL);

	gconf_client_set_string (config,"/gPHPEdit/line_numbers/font", preferences.line_number_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/line_numbers/fore", preferences.line_number_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/line_numbers/back", preferences.line_number_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/line_numbers/size", preferences.line_number_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/line_numbers/italic", preferences.line_number_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/line_numbers/bold", preferences.line_number_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/default_style/selection", preferences.set_sel_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/default_style/bookmark", preferences.marker_back,NULL);
	gconf_client_set_string (config,"/gPHPEdit/locations/php_binary", preferences.php_binary_location,NULL);
	gconf_client_set_string (config,"/gPHPEdit/locations/shared_source", preferences.shared_source_location,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/indentationsize", preferences.indentation_size,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/tabsize", preferences.tab_size,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/linewrapping", preferences.line_wrapping,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/showindentationguides", preferences.show_indentation_guides,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/showfolding", preferences.show_folding,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/edgemode", preferences.edge_mode,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/edgecolumn", preferences.edge_column,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/edgecolour", preferences.edge_colour,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/auto_complete_delay", preferences.auto_complete_delay,NULL);
	gconf_client_set_int (config,"/gPHPEdit/defaults/calltip_delay", preferences.calltip_delay,NULL);
	//gconf_client_set_int (config,"/gPHPEdit//defaults/autoindentafterbrace"), preferences.auto_indent_after_brace,NULL);
	gconf_client_set_string (config,"/gPHPEdit/defaults/php_file_extensions",preferences.php_file_extensions,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/defaults/save_session", preferences.save_session,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/defaults/use_tabs_instead_spaces", preferences.use_tabs_instead_spaces,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/defaults/single_instance_only", preferences.single_instance_only,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_tag/back", preferences.html_tag_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_tag/fore", preferences.html_tag_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_tag/font", preferences.html_tag_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_tag/size", preferences.html_tag_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_tag/italic", preferences.html_tag_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_tag/bold", preferences.html_tag_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_tag_unknown/back", preferences.html_tag_unknown_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_tag_unknown/fore", preferences.html_tag_unknown_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_tag_unknown/font", preferences.html_tag_unknown_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_tag_unknown/size", preferences.html_tag_unknown_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_tag_unknown/italic", preferences.html_tag_unknown_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_tag_unknown/bold", preferences.html_tag_unknown_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_attribute/back", preferences.html_attribute_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_attribute/fore", preferences.html_attribute_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_attribute/font", preferences.html_attribute_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_attribute/size", preferences.html_attribute_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_attribute/italic", preferences.html_attribute_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_attribute/bold", preferences.html_attribute_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_attribute_unknown/back", preferences.html_attribute_unknown_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_attribute_unknown/fore", preferences.html_attribute_unknown_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_attribute_unknown/font", preferences.html_attribute_unknown_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_attribute_unknown/size", preferences.html_attribute_unknown_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_attribute_unknown/italic", preferences.html_attribute_unknown_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_attribute_unknown/bold", preferences.html_attribute_unknown_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_number/back", preferences.html_number_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_number/fore", preferences.html_number_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_number/font", preferences.html_number_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_number/size", preferences.html_number_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_number/italic", preferences.html_number_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_number/bold", preferences.html_number_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_single_string/back", preferences.html_single_string_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_single_string/fore", preferences.html_single_string_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_single_string/font", preferences.html_single_string_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_single_string/size", preferences.html_single_string_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_single_string/italic", preferences.html_single_string_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_single_string/bold", preferences.html_single_string_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_double_string/back", preferences.html_double_string_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_double_string/fore", preferences.html_double_string_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_double_string/font", preferences.html_double_string_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_double_string/size", preferences.html_double_string_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_double_string/italic", preferences.html_double_string_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_double_string/bold", preferences.html_double_string_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_comment/back", preferences.html_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_comment/fore", preferences.html_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_comment/font", preferences.html_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_comment/size", preferences.html_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_comment/italic", preferences.html_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_comment/bold", preferences.html_comment_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_entity/back", preferences.html_entity_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_entity/fore", preferences.html_entity_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_entity/font", preferences.html_entity_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_entity/size", preferences.html_entity_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_entity/italic", preferences.html_entity_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_entity/bold", preferences.html_entity_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_script/back", preferences.html_script_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_script/fore", preferences.html_script_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_script/font", preferences.html_script_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_script/size", preferences.html_script_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_script/italic", preferences.html_script_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_script/bold", preferences.html_script_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_question/back", preferences.html_question_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_question/fore", preferences.html_question_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_question/font", preferences.html_question_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_question/size", preferences.html_question_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_question/italic", preferences.html_question_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_question/bold", preferences.html_question_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/html_value/back", preferences.html_value_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_value/fore", preferences.html_value_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/html_value/font", preferences.html_value_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/html_value/size", preferences.html_value_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_value/italic", preferences.html_value_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/html_value/bold", preferences.html_value_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_comment/back", preferences.javascript_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment/fore", preferences.javascript_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_comment/font", preferences.javascript_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment/size", preferences.javascript_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment/italic", preferences.javascript_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment/bold", preferences.javascript_comment_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_line/back", preferences.javascript_comment_line_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_line/fore", preferences.javascript_comment_line_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_comment_line/font", preferences.javascript_comment_line_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_line/size", preferences.javascript_comment_line_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment_line/italic", preferences.javascript_comment_line_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment_line/bold", preferences.javascript_comment_line_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_doc/back", preferences.javascript_comment_doc_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_doc/fore", preferences.javascript_comment_doc_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_comment_doc/font", preferences.javascript_comment_doc_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_comment_doc/size", preferences.javascript_comment_doc_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment_doc/italic", preferences.javascript_comment_doc_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_comment_doc/bold", preferences.javascript_comment_doc_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_word/back", preferences.javascript_word_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_word/fore", preferences.javascript_word_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_word/font", preferences.javascript_word_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_word/size", preferences.javascript_word_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_word/italic", preferences.javascript_word_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_word/bold", preferences.javascript_word_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_keyword/back", preferences.javascript_keyword_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_keyword/fore", preferences.javascript_keyword_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_keyword/font", preferences.javascript_keyword_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_keyword/size", preferences.javascript_keyword_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_keyword/italic", preferences.javascript_keyword_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_keyword/bold", preferences.javascript_keyword_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_doublestring/back", preferences.javascript_doublestring_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_doublestring/fore", preferences.javascript_doublestring_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_doublestring/font", preferences.javascript_doublestring_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_doublestring/size", preferences.javascript_doublestring_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_doublestring/italic", preferences.javascript_doublestring_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_doublestring/bold", preferences.javascript_doublestring_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_singlestring/back", preferences.javascript_singlestring_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_singlestring/fore", preferences.javascript_singlestring_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_singlestring/font", preferences.javascript_singlestring_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_singlestring/size", preferences.javascript_singlestring_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_singlestring/italic", preferences.javascript_singlestring_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_singlestring/bold", preferences.javascript_singlestring_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/javascript_symbols/back", preferences.javascript_symbols_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_symbols/fore", preferences.javascript_symbols_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/javascript_symbols/font", preferences.javascript_symbols_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/javascript_symbols/size", preferences.javascript_symbols_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_symbols/italic", preferences.javascript_symbols_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/javascript_symbols/bold", preferences.javascript_symbols_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_default_style/back", preferences.php_default_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_default_style/fore", preferences.php_default_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_default_style/font", preferences.php_default_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_default_style/size", preferences.php_default_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_default_style/italic", preferences.php_default_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_default_style/bold", preferences.php_default_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_hstring/fore", preferences.php_hstring_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_hstring/font", preferences.php_hstring_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_hstring/back", preferences.php_hstring_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_hstring/size", preferences.php_hstring_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_hstring/italic", preferences.php_hstring_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_hstring/bold", preferences.php_hstring_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_simplestring/fore", preferences.php_simplestring_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_simplestring/font", preferences.php_simplestring_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_simplestring/back", preferences.php_simplestring_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_simplestring/size", preferences.php_simplestring_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_simplestring/italic", preferences.php_simplestring_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_simplestring/bold", preferences.php_simplestring_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_word/fore", preferences.php_word_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_word/font", preferences.php_word_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_word/back", preferences.php_word_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_word/size", preferences.php_word_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_word/italic", preferences.php_word_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_word/bold", preferences.php_word_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_number/fore", preferences.php_number_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_number/font", preferences.php_number_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_number/back", preferences.php_number_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_number/size", preferences.php_number_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_number/italic", preferences.php_number_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_number/bold", preferences.php_number_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_variable/fore", preferences.php_variable_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_variable/font", preferences.php_variable_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_variable/back", preferences.php_variable_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_variable/size", preferences.php_variable_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_variable/italic", preferences.php_variable_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_variable/bold", preferences.php_variable_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_comment/fore", preferences.php_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_comment/font", preferences.php_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_comment/back", preferences.php_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_comment/size", preferences.php_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_comment/italic", preferences.php_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_comment/bold", preferences.php_comment_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/php_comment_line/fore", preferences.php_comment_line_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/php_comment_line/font", preferences.php_comment_line_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_comment_line/back", preferences.php_comment_line_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/php_comment_line/size", preferences.php_comment_line_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_comment_line/italic", preferences.php_comment_line_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/php_comment_line/bold", preferences.php_comment_line_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_tag/back", preferences.css_tag_back,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_tag/font", preferences.css_tag_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_tag/fore", preferences.css_tag_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_tag/size", preferences.css_tag_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_tag/italic", preferences.css_tag_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_tag/bold", preferences.css_tag_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_class/fore", preferences.css_class_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_class/font", preferences.css_class_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_class/back", preferences.css_class_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_class/size", preferences.css_class_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_class/italic", preferences.css_class_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_class/bold", preferences.css_class_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_pseudoclass/back", preferences.css_pseudoclass_back,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_pseudoclass/font", preferences.css_pseudoclass_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_pseudoclass/fore", preferences.css_pseudoclass_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_pseudoclass/size", preferences.css_pseudoclass_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_pseudoclass/italic", preferences.css_pseudoclass_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_pseudoclass/bold", preferences.css_pseudoclass_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_unknown_pseudoclass/fore", preferences.css_unknown_pseudoclass_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_unknown_pseudoclass/font", preferences.css_unknown_pseudoclass_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_unknown_pseudoclass/back", preferences.css_unknown_pseudoclass_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_unknown_pseudoclass/size", preferences.css_unknown_pseudoclass_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_unknown_pseudoclass/italic", preferences.css_unknown_pseudoclass_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_unknown_pseudoclass/bold", preferences.css_unknown_pseudoclass_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_operator/fore", preferences.css_operator_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_operator/font", preferences.css_operator_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_operator/back", preferences.css_operator_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_operator/size", preferences.css_operator_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_operator/italic", preferences.css_operator_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_operator/bold", preferences.css_operator_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_identifier/fore", preferences.css_identifier_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_identifier/font", preferences.css_identifier_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_identifier/back", preferences.css_identifier_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_identifier/size", preferences.css_identifier_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_identifier/italic", preferences.css_identifier_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_identifier/bold", preferences.css_identifier_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_unknown_identifier/fore", preferences.css_unknown_identifier_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_unknown_identifier/font", preferences.css_unknown_identifier_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_unknown_identifier/back", preferences.css_unknown_identifier_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_unknown_identifier/size", preferences.css_unknown_identifier_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_unknown_identifier/italic", preferences.css_unknown_identifier_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_unknown_identifier/bold", preferences.css_unknown_identifier_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_value/fore", preferences.css_value_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_value/font", preferences.css_value_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_value/back", preferences.css_value_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_value/size", preferences.css_value_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_value/italic", preferences.css_value_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_value/bold", preferences.css_value_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_comment/fore", preferences.css_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_comment/font", preferences.css_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_comment/back", preferences.css_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_comment/size", preferences.css_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_comment/italic", preferences.css_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_comment/bold", preferences.css_comment_bold,NULL);

	gconf_client_set_string (config,"/gPHPEdit/css_id/fore", preferences.css_id_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/font", preferences.css_id_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/back", preferences.css_id_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/size", preferences.css_id_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_id/italic", preferences.css_id_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_id/bold", preferences.css_id_bold,NULL);

	gconf_client_set_string (config,"/gPHPEdit/css_important/fore", preferences.css_important_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_important/font", preferences.css_important_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_important/back", preferences.css_important_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_important/size", preferences.css_important_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_important/italic", preferences.css_important_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_important/bold", preferences.css_important_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/css_directive/fore", preferences.css_directive_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/css_directive/font", preferences.css_directive_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_directive/back", preferences.css_directive_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_directive/size", preferences.css_directive_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_directive/italic", preferences.css_directive_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_directive/bold", preferences.css_directive_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_word/fore", preferences.sql_word_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_word/font", preferences.sql_word_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_word/back", preferences.sql_word_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_word/size", preferences.sql_word_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_word/italic", preferences.sql_word_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_word/bold", preferences.sql_word_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_string/fore", preferences.sql_string_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_string/font", preferences.sql_string_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_string/back", preferences.sql_string_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_string/size", preferences.sql_string_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_string/italic", preferences.sql_string_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_string/bold", preferences.sql_string_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_operator/fore", preferences.sql_operator_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_operator/font", preferences.sql_operator_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_operator/back", preferences.sql_operator_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_operator/size", preferences.sql_operator_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_operator/italic", preferences.sql_operator_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_operator/bold", preferences.sql_operator_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_comment/fore", preferences.sql_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_comment/font", preferences.sql_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_comment/back", preferences.sql_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_comment/size", preferences.sql_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_comment/italic", preferences.sql_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_comment/bold", preferences.sql_comment_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_number/fore", preferences.sql_number_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_number/font", preferences.sql_number_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_number/back", preferences.sql_number_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_number/size", preferences.sql_number_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_number/italic", preferences.sql_number_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_number/bold", preferences.sql_number_bold,NULL);

	gconf_client_set_int (config,"/gPHPEdit/sql_identifier/fore", preferences.sql_identifier_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/sql_identifier/font", preferences.sql_identifier_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_identifier/back", preferences.sql_identifier_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/sql_identifier/size", preferences.sql_identifier_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_identifier/italic", preferences.sql_identifier_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/sql_identifier/bold", preferences.sql_identifier_bold,NULL);
        g_string_free (uri, TRUE);
}
