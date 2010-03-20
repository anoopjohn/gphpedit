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

#include "preferences.h"
#include "main_window.h"
#include "main.h"
//debug macro
//#define DEBUG

#define DEFAULT_PHP_EXTENSIONS "php,inc,phtml,php3,xml,htm,html"
#define DEFAULT_BACK_COLOR 16777215
Preferences preferences;

gchar *get_string(GConfClient *client,gchar *key,gchar *default_font){
GError *error=NULL;
gchar *temp= gconf_client_get_string(client,key,NULL);
        if (!temp || error!=NULL){
            return default_font;
        }
	return temp;
}

gint get_size(GConfClient *client,gchar *key,gint default_size){
gint temp= gconf_client_get_int (client,key,NULL);
        if (temp==0){
            return 12;
        }
return temp;
}
gint getcolor(GConfClient *client,gchar *key,gchar *subdir,gint default_color){
        gchar *uri= g_strdup_printf("%s/%s/%s",g_get_home_dir(),".gconf/gPHPEdit",subdir);
        #ifdef DEBUG
        g_print("uri:%s \n",uri);
        #endif
       if (!g_file_test (uri,G_FILE_TEST_EXISTS)){
            #ifdef DEBUG
            g_print("key %s don't exist. load default value\n",key);
            #endif
            //load default value
	    g_free(uri);
            return default_color;
        }else {
		    g_free(uri);
          //load key value
          GError *error=NULL;
            gint temp;
            temp = gconf_client_get_int (client,key,&error);
            if (error!=NULL){
                //return default value
                return default_color;
            } else {
                return temp;
            }
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
	preferences.maximized = gconf_client_get_bool (config,"/gPHPEdit/main_window/maximized",NULL);
        
	gtk_window_move(GTK_WINDOW(main_window.window), preferences.left, preferences.top);
	gtk_window_set_default_size(GTK_WINDOW(main_window.window), preferences.width, preferences.height);
 	
	if (preferences.maximized) {
 		gtk_window_maximize(GTK_WINDOW(main_window.window));
	}
	
}

void preferences_load(void)
{
        GConfClient *config=gconf_client_get_default ();
        GError *error = NULL;
        preferences.set_sel_back = getcolor(config,"/gPHPEdit/default_style/selection","default_style",11250603);
        preferences.marker_back = getcolor(config,"/gPHPEdit/default_style/bookmark","default_style",15908608);
        preferences.php_binary_location= get_string(config,"/gPHPEdit/locations/phpbinary","php");
        preferences.shared_source_location = get_string(config,"/gPHPEdit/locations/shared_source","");
        preferences.indentation_size = gconf_client_get_int (config,"/gPHPEdit/defaults/indentationsize",&error);
        if (preferences.indentation_size==0){
            preferences.indentation_size=4;
            error=NULL;
        }
	preferences.tab_size = gconf_client_get_int (config,"/gPHPEdit/defaults/tabsize",&error);
        if (preferences.tab_size==0){
            preferences.tab_size=4;
            error=NULL;
        }
	preferences.auto_complete_delay = gconf_client_get_int (config,"/gPHPEdit/defaults/auto_complete_delay",&error);
        if (preferences.auto_complete_delay==0){
            preferences.auto_complete_delay=500;
            error=NULL;
        }
	preferences.calltip_delay = gconf_client_get_int (config,"/gPHPEdit/defaults/calltip_delay",&error);
        if (preferences.calltip_delay==0){
            preferences.calltip_delay=500;
            error=NULL;
        }
	preferences.show_indentation_guides = gconf_client_get_int (config,"/gPHPEdit/defaults/showindentationguides",NULL);
	preferences.show_folding = true;//gconf_client_get_bool (config,"/gPHPEdit/defaults/showfolding",NULL);
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
	/* font quality */
        preferences.font_quality = gconf_client_get_int (config,"/gPHPEdit/defaults/fontquality",NULL);

	//preferences.auto_indent_after_brace = gnome_config_get_int ("gPHPEdit/defaults/autoindentafterbrace=1");
	preferences.save_session = gconf_client_get_bool (config,"/gPHPEdit/defaults/save_session",NULL);
	preferences.use_tabs_instead_spaces = gconf_client_get_bool(config,"/gPHPEdit/defaults/use_tabs_instead_spaces",&error);
        if (preferences.use_tabs_instead_spaces==0 && error!=NULL){
            preferences.use_tabs_instead_spaces=true;
            error=NULL;
        }
        preferences.single_instance_only = gconf_client_get_bool(config,"/gPHPEdit/defaults/single_instance_only",NULL);
        preferences.php_file_extensions = get_string(config,"/gPHPEdit/defaults/php_file_extensions",DEFAULT_PHP_EXTENSIONS);
        
        preferences.default_font = get_string(config,"/gPHPEdit/default_style/font",DEFAULT_FONT);
	preferences.default_fore = getcolor(config,"/gPHPEdit/default_style/fore","default_style",0);
	preferences.default_back = getcolor(config,"/gPHPEdit/default_style/back","default_style",DEFAULT_BACK_COLOR);
        preferences.default_size = get_size(config,"/gPHPEdit/default_style/size",DEFAULT_FONT_SIZE);
	preferences.default_bold = gconf_client_get_bool(config,"/gPHPEdit/default_style/bold",NULL);
	preferences.default_italic = gconf_client_get_bool(config,"/gPHPEdit/default_style/italic",NULL);
	
	preferences.line_number_font = get_string(config,"/gPHPEdit/line_numbers/font",DEFAULT_FONT);
	preferences.line_number_fore = getcolor(config,"/gPHPEdit/line_numbers/fore","line_numbers",0);
	preferences.line_number_back = getcolor(config,"/gPHPEdit/line_numbers/back","line_numbers",11053224);
        preferences.line_number_size = get_size(config,"/gPHPEdit/line_numbers/size",DEFAULT_FONT_SIZE);
	preferences.line_number_bold = gconf_client_get_bool(config,"/gPHPEdit/line_numbers/bold",NULL);
	preferences.line_number_italic = gconf_client_get_bool(config,"/gPHPEdit/line_numbers/italic",NULL);

	preferences.html_tag_back = getcolor(config,"/gPHPEdit/html_tag/back","html_tag",DEFAULT_BACK_COLOR);
        preferences.html_tag_fore = getcolor(config,"/gPHPEdit/html_tag/fore","html_tag",7553164);
        preferences.html_tag_font = get_string(config,"/gPHPEdit/html_tag/font",DEFAULT_FONT);
	preferences.html_tag_size = get_size(config,"/gPHPEdit/html_tag/size",DEFAULT_FONT_SIZE);
	preferences.html_tag_bold = gconf_client_get_bool(config,"/gPHPEdit/html_tag/bold",NULL);
	preferences.html_tag_italic = gconf_client_get_bool(config,"/gPHPEdit/html_tag/italic",NULL);

	preferences.html_tag_unknown_back = getcolor(config,"/gPHPEdit/html_tag_unknown/back","html_tag_unknown",DEFAULT_BACK_COLOR);
        preferences.html_tag_unknown_fore = getcolor(config,"/gPHPEdit/html_tag_unknown/fore","html_tag_unknown",7553164);
        preferences.html_tag_unknown_font = get_string(config,"/gPHPEdit/html_tag_unknown/font",DEFAULT_FONT);
	preferences.html_tag_unknown_size = get_size(config,"/gPHPEdit/html_tag_unknown/size",DEFAULT_FONT_SIZE);
	preferences.html_tag_unknown_bold = gconf_client_get_bool(config,"/gPHPEdit/html_tag_unknown/bold",NULL);
	preferences.html_tag_unknown_italic = gconf_client_get_bool(config,"/gPHPEdit/html_tag_unknown/italic",NULL);

	preferences.html_attribute_back = getcolor(config,"/gPHPEdit/html_attribute/back","html_attribute",DEFAULT_BACK_COLOR);
        preferences.html_attribute_fore = getcolor(config,"/gPHPEdit/html_attribute/fore","html_attribute",9204544);
        preferences.html_attribute_font = get_string(config,"/gPHPEdit/html_attribute/font",DEFAULT_FONT);
	preferences.html_attribute_size = get_size(config,"/gPHPEdit/html_attribute/size",DEFAULT_FONT_SIZE);
	preferences.html_attribute_bold = gconf_client_get_bool(config,"/gPHPEdit/html_attribute/bold",NULL);
	preferences.html_attribute_italic = gconf_client_get_bool(config,"/gPHPEdit/html_attribute/italic",NULL);

	preferences.html_attribute_unknown_back = getcolor(config,"/gPHPEdit/html_attribute_unknown/back","html_attribute_unknown",DEFAULT_BACK_COLOR);
        preferences.html_attribute_unknown_fore = getcolor(config,"/gPHPEdit/html_attribute_unknown/fore","html_attribute_unknown",7472544);
        preferences.html_attribute_unknown_font = get_string(config,"/gPHPEdit/html_attribute_unknown/font",DEFAULT_FONT);
	preferences.html_attribute_unknown_size = get_size(config,"/gPHPEdit/html_attribute_unknown/size",DEFAULT_FONT_SIZE);
	preferences.html_attribute_unknown_bold = gconf_client_get_bool(config,"/gPHPEdit/html_attribute_unknown/bold",NULL);
	preferences.html_attribute_unknown_italic = gconf_client_get_bool(config,"/gPHPEdit/html_attribute_unknown/italic",NULL);

	preferences.html_number_back = getcolor(config,"/gPHPEdit/html_number/back","html_number",DEFAULT_BACK_COLOR);
        preferences.html_number_fore = getcolor(config,"/gPHPEdit/html_number/fore","html_number",9204544);
        preferences.html_number_font = get_string(config,"/gPHPEdit/html_number/font",DEFAULT_FONT);
	preferences.html_number_size = get_size(config,"/gPHPEdit/html_number/size",DEFAULT_FONT_SIZE);
	preferences.html_number_bold = gconf_client_get_bool(config,"/gPHPEdit/html_number/bold",NULL);
	preferences.html_number_italic = gconf_client_get_bool(config,"/gPHPEdit/html_number/italic",NULL);

	preferences.html_single_string_back = getcolor(config,"/gPHPEdit/html_single_string/back","html_single_string",DEFAULT_BACK_COLOR);
        preferences.html_single_string_fore = getcolor(config,"/gPHPEdit/html_single_string/fore","html_single_string",32768);
        preferences.html_single_string_font = get_string(config,"/gPHPEdit/html_single_string/font",DEFAULT_FONT);
	preferences.html_single_string_size = get_size(config,"/gPHPEdit/html_single_string/size",DEFAULT_FONT_SIZE);
	preferences.html_single_string_bold = gconf_client_get_bool(config,"/gPHPEdit/html_single_string/bold",NULL);
	preferences.html_single_string_italic = gconf_client_get_bool(config,"/gPHPEdit/html_single_string/italic",NULL);

	preferences.html_double_string_back = getcolor(config,"/gPHPEdit/html_double_string/back","html_double_string",DEFAULT_BACK_COLOR);
        preferences.html_double_string_fore = getcolor(config,"/gPHPEdit/html_double_string/fore","html_double_string",32768);
        preferences.html_double_string_font = get_string(config,"/gPHPEdit/html_double_string/font",DEFAULT_FONT);
	preferences.html_double_string_size = get_size(config,"/gPHPEdit/html_double_string/size",DEFAULT_FONT_SIZE);
	preferences.html_double_string_bold = gconf_client_get_bool(config,"/gPHPEdit/html_double_string/bold",NULL);
	preferences.html_double_string_italic = gconf_client_get_bool(config,"/gPHPEdit/html_double_string/italic",NULL);

	preferences.html_comment_back = getcolor(config,"/gPHPEdit/html_comment/back","html_comment",DEFAULT_BACK_COLOR);
        preferences.html_comment_fore = getcolor(config,"/gPHPEdit/html_comment/fore","html_comment",842125504);
        preferences.html_comment_font = get_string(config,"/gPHPEdit/html_comment/font",DEFAULT_FONT);
	preferences.html_comment_size = get_size(config,"/gPHPEdit/html_comment/size",DEFAULT_FONT_SIZE);
	preferences.html_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/html_comment/bold",NULL);
	preferences.html_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/html_comment/italic",NULL);

	preferences.html_entity_back =  getcolor(config,"/gPHPEdit/html_entity/back","html_entity",DEFAULT_BACK_COLOR);
        preferences.html_entity_fore =  getcolor(config,"/gPHPEdit/html_entity/fore","html_entity",8421504);
        preferences.html_entity_font = get_string(config,"/gPHPEdit/html_entity/font",DEFAULT_FONT);
	preferences.html_entity_size = get_size(config,"/gPHPEdit/html_entity/size",DEFAULT_FONT_SIZE);
        preferences.html_entity_bold = gconf_client_get_bool(config,"/gPHPEdit/html_entity/bold",NULL);
	preferences.html_entity_italic = gconf_client_get_bool(config,"/gPHPEdit/html_entity/italic",NULL);

	preferences.html_script_back = getcolor(config,"/gPHPEdit/html_script/back","html_script",DEFAULT_BACK_COLOR);
        preferences.html_script_fore = getcolor(config,"/gPHPEdit/html_script/fore","html_script",7553165);
        preferences.html_script_font = get_string(config,"/gPHPEdit/html_script/font",DEFAULT_FONT);
	preferences.html_script_size = get_size(config,"/gPHPEdit/html_script/size",DEFAULT_FONT_SIZE);
	preferences.html_script_bold = gconf_client_get_bool(config,"/gPHPEdit/html_script/bold",NULL);
	preferences.html_script_italic = gconf_client_get_bool(config,"/gPHPEdit/html_script/italic",NULL);

	preferences.html_question_back = getcolor(config,"/gPHPEdit/html_question/back","html_question",DEFAULT_BACK_COLOR);
        preferences.html_question_fore = getcolor(config,"/gPHPEdit/html_question/fore","html_question",7553165);
	preferences.html_question_font = get_string(config,"/gPHPEdit/html_question/font",DEFAULT_FONT);
	preferences.html_question_size = get_size(config,"/gPHPEdit/html_question/size",DEFAULT_FONT_SIZE);
	preferences.html_question_bold = gconf_client_get_bool(config,"/gPHPEdit/html_question/bold",NULL);
	preferences.html_question_italic = gconf_client_get_bool(config,"/gPHPEdit/html_question/italic",NULL);

	preferences.html_value_back = getcolor(config,"/gPHPEdit/html_value/back","html_value",DEFAULT_BACK_COLOR);
        preferences.html_value_fore = getcolor(config,"/gPHPEdit/html_value/fore","html_value",21632);
        preferences.html_value_font = get_string(config,"/gPHPEdit/html_script/font",DEFAULT_FONT);
	preferences.html_value_size = get_size(config,"/gPHPEdit/html_value/size",DEFAULT_FONT_SIZE);
	preferences.html_value_bold = gconf_client_get_bool(config,"/gPHPEdit/html_value/bold",NULL);
	preferences.html_value_italic = gconf_client_get_bool(config,"/gPHPEdit/html_value/italic",NULL);

	preferences.javascript_comment_back = getcolor(config,"/gPHPEdit/javascript_comment/back","javascript_comment",DEFAULT_BACK_COLOR);
        preferences.javascript_comment_fore = getcolor(config,"/gPHPEdit/javascript_comment/fore","javascript_comment",8421504);
        preferences.javascript_comment_font = get_string(config,"/gPHPEdit/javascript_comment/font",DEFAULT_FONT);
	preferences.javascript_comment_size = get_size(config,"/gPHPEdit/javascript_comment/size",DEFAULT_FONT_SIZE);
	preferences.javascript_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment/bold",NULL);
	preferences.javascript_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments/italic",NULL);

	preferences.javascript_comment_line_back = getcolor(config,"/gPHPEdit/javascript_comment_line/back","javascript_comment_line",DEFAULT_BACK_COLOR);
        preferences.javascript_comment_line_fore = getcolor(config,"/gPHPEdit/javascript_comment_line/fore","javascript_comment_line",8421504);
        preferences.javascript_comment_line_font = get_string(config,"/gPHPEdit/javascript_comment_line/font",DEFAULT_FONT);
	preferences.javascript_comment_line_size = get_size(config,"/gPHPEdit/javascript_comment_line/size",DEFAULT_FONT_SIZE);
	preferences.javascript_comment_line_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment_line/bold",NULL);
	preferences.javascript_comment_line_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comment_line/italic",NULL);

	preferences.javascript_comment_doc_back = getcolor(config,"/gPHPEdit/javascript_comment_doc/back","javascript_comment_doc",DEFAULT_BACK_COLOR);
        preferences.javascript_comment_doc_fore = getcolor(config,"/gPHPEdit/javascript_comment_doc/fore","javascript_comment_doc",8355712);
        preferences.javascript_comment_doc_font = get_string(config,"/gPHPEdit/javascript_comment_doc/font",DEFAULT_FONT);
	preferences.javascript_comment_doc_size = get_size(config,"/gPHPEdit/javascript_comment_doc/size",DEFAULT_FONT_SIZE);
        preferences.javascript_comment_doc_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments_doc/bold",NULL);
	preferences.javascript_comment_doc_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_comments_doc/italic",NULL);

	preferences.javascript_word_back = getcolor(config,"/gPHPEdit/javascript_word/back","javascript_word",DEFAULT_BACK_COLOR);
        preferences.javascript_word_fore = getcolor(config,"/gPHPEdit/javascript_word/fore","javascript_word",9204544);
        preferences.javascript_word_font = get_string(config,"/gPHPEdit/javascript_word/font",DEFAULT_FONT);
	preferences.javascript_word_size = get_size(config,"/gPHPEdit/javascript_word/size",DEFAULT_FONT_SIZE);
	preferences.javascript_word_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_word/bold",NULL);
	preferences.javascript_word_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_word/italic",NULL);

	preferences.javascript_keyword_back = getcolor(config,"/gPHPEdit/javascript_keyword/back","javascript_keyword",DEFAULT_BACK_COLOR);
        preferences.javascript_keyword_fore = getcolor(config,"/gPHPEdit/javascript_keyword/fore","javascript_doublestring",8388608);
        preferences.javascript_keyword_font = get_string(config,"/gPHPEdit/javascript_keyword/font",DEFAULT_FONT);
	preferences.javascript_keyword_size = get_size(config,"/gPHPEdit/javascript_keyword/size",DEFAULT_FONT_SIZE);
        preferences.javascript_keyword_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_keyword/bold",NULL);
	preferences.javascript_keyword_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_keyword/italic",NULL);

	preferences.javascript_doublestring_back = getcolor(config,"/gPHPEdit/javascript_doublestring/back","javascript_doublestring",DEFAULT_BACK_COLOR);
        preferences.javascript_doublestring_fore = getcolor(config,"/gPHPEdit/javascript_doublestring/fore","javascript_doublestring",8388608);
	preferences.javascript_doublestring_font = get_string(config,"/gPHPEdit/javascript_doublestring/font",DEFAULT_FONT);
	preferences.javascript_doublestring_size = get_size(config,"/gPHPEdit/javascript_doublestring/size",DEFAULT_FONT_SIZE);
	preferences.javascript_doublestring_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_doublestring/bold",NULL);
	preferences.javascript_doublestring_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_doublestring/italic",NULL);

	preferences.javascript_singlestring_back = getcolor(config,"/gPHPEdit/javascript_singlestring/back","javascript_singlestring",DEFAULT_BACK_COLOR);
        preferences.javascript_singlestring_fore = getcolor(config,"/gPHPEdit/javascript_singlestring/fore","javascript_singlestring",8388608);
        preferences.javascript_singlestring_font = get_string(config,"/gPHPEdit/javascript_singlestring/font",DEFAULT_FONT);
	preferences.javascript_singlestring_size = get_size(config,"/gPHPEdit/javascript_singlestring/size",DEFAULT_FONT_SIZE);
	preferences.javascript_singlestring_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_singlestring/bold",NULL);
	preferences.javascript_singlestring_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_singlestring/bold",NULL);

	preferences.javascript_symbols_back = getcolor(config,"/gPHPEdit/javascript_symbols/back","javascript_symbols",DEFAULT_BACK_COLOR);
        preferences.javascript_symbols_fore = getcolor(config,"/gPHPEdit/javascript_symbols/fore","javascript_symbols",8355712);
        preferences.javascript_symbols_font = get_string(config,"/gPHPEdit/javascript_symbols/font",DEFAULT_FONT);
	preferences.javascript_symbols_size = get_size(config,"/gPHPEdit/javascript_symbols/size",DEFAULT_FONT_SIZE);
	preferences.javascript_symbols_bold = gconf_client_get_bool(config,"/gPHPEdit/javascript_symbols/bold",NULL);
	preferences.javascript_symbols_italic = gconf_client_get_bool(config,"/gPHPEdit/javascript_symbols/italic",NULL);

	preferences.php_default_font = get_string(config,"/gPHPEdit/php_default_style/font",DEFAULT_FONT);
	preferences.php_default_fore = getcolor(config,"/gPHPEdit/php_default_style/fore","php_default_style",1052688);
        preferences.php_default_back = getcolor(config,"/gPHPEdit/php_default_style/back","php_default_style",DEFAULT_BACK_COLOR);
        preferences.php_default_size = get_size(config,"/gPHPEdit/php_default_style/size",DEFAULT_FONT_SIZE);
	preferences.php_default_bold = gconf_client_get_bool(config,"/gPHPEdit/php_default_style/bold",NULL);
	preferences.php_default_italic = gconf_client_get_bool(config,"/gPHPEdit/php_default_style/italic",NULL);

	preferences.php_hstring_fore = getcolor(config,"/gPHPEdit/php_hstring/fore","php_hstring",8388736);
        preferences.php_hstring_font = get_string(config,"/gPHPEdit/php_hstring/font",DEFAULT_FONT);
	preferences.php_hstring_back = getcolor(config,"/gPHPEdit/php_hstring/back","php_hstring",DEFAULT_BACK_COLOR);
        preferences.php_hstring_size = get_size(config,"/gPHPEdit/php_hstring/size",DEFAULT_FONT_SIZE);
	preferences.php_hstring_bold = gconf_client_get_bool(config,"/gPHPEdit/php_hstring/bold",NULL);
	preferences.php_hstring_italic = gconf_client_get_bool(config,"/gPHPEdit/php_hstring/italic",NULL);

	preferences.php_simplestring_fore = getcolor(config,"/gPHPEdit/php_simplestring/fore","php_simplestring",8388736);
        preferences.php_simplestring_font = get_string(config,"/gPHPEdit/php_simplestring/font",DEFAULT_FONT);
	preferences.php_simplestring_back = getcolor(config,"/gPHPEdit/php_simplestring/back","php_simplestring",DEFAULT_BACK_COLOR);
        preferences.php_simplestring_size = get_size(config,"/gPHPEdit/php_simplestring/size",DEFAULT_FONT_SIZE);
	preferences.php_simplestring_bold = gconf_client_get_bool(config,"/gPHPEdit/php_simplestring/bold",NULL);
	preferences.php_simplestring_italic = gconf_client_get_bool(config,"/gPHPEdit/php_simplestring/italic",NULL);

	preferences.php_word_fore = getcolor(config,"/gPHPEdit/php_word/fore","php_word",0);
	preferences.php_word_font = get_string(config,"/gPHPEdit/php_word/font",DEFAULT_FONT);
	preferences.php_word_back = getcolor(config,"/gPHPEdit/php_word/back","php_word",DEFAULT_BACK_COLOR);
        preferences.php_word_size = get_size(config,"/gPHPEdit/php_word/size",DEFAULT_FONT_SIZE);
	preferences.php_word_bold = gconf_client_get_bool(config,"/gPHPEdit/php_word/bold",NULL);
	preferences.php_word_italic = gconf_client_get_bool(config,"/gPHPEdit/php_word/italic",NULL);

	preferences.php_number_fore = getcolor(config,"/gPHPEdit/php_number/fore","php_number",9204544);
        preferences.php_number_font = get_string(config,"/gPHPEdit/php_number/font",DEFAULT_FONT);
	preferences.php_number_back = getcolor(config,"/gPHPEdit/php_number/back","php_number",DEFAULT_BACK_COLOR);
        preferences.php_number_size = get_size(config,"/gPHPEdit/php_number/size",DEFAULT_FONT_SIZE);
	preferences.php_number_bold = gconf_client_get_bool(config,"/gPHPEdit/php_number/bold",NULL);
	preferences.php_number_italic = gconf_client_get_bool(config,"/gPHPEdit/php_number/italic",NULL);

	preferences.php_variable_fore = getcolor(config,"/gPHPEdit/php_variable/fore","php_variable",16746496);
        preferences.php_variable_font = get_string(config,"/gPHPEdit/php_variable/font",DEFAULT_FONT);
	preferences.php_variable_back = getcolor(config,"/gPHPEdit/php_variable/back","php_variable",DEFAULT_BACK_COLOR);
        preferences.php_variable_size = get_size(config,"/gPHPEdit/php_variable/size",DEFAULT_FONT_SIZE);
	preferences.php_variable_bold = gconf_client_get_bool(config,"/gPHPEdit/php_variable/bold",NULL);
	preferences.php_variable_italic = gconf_client_get_bool(config,"/gPHPEdit/php_variable/italic",NULL);

	preferences.php_comment_fore = getcolor(config,"/gPHPEdit/php_comment/fore","php_comment",8421594);
        preferences.php_comment_font = get_string(config,"/gPHPEdit/php_comment/font",DEFAULT_FONT);
	preferences.php_comment_back = getcolor(config,"/gPHPEdit/php_comment/back","php_comment",DEFAULT_BACK_COLOR);
        preferences.php_comment_size = get_size(config,"/gPHPEdit/php_comment/size",DEFAULT_FONT_SIZE);
	preferences.php_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/php_comment/bold",NULL);
	preferences.php_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/php_comment/italic",NULL);

	preferences.php_comment_line_fore = getcolor(config,"/gPHPEdit/php_comment_line/fore","php_comment_line",8421504);
        preferences.php_comment_line_font = get_string(config,"/gPHPEdit/php_comment_line/font",DEFAULT_FONT);
	preferences.php_comment_line_back = getcolor(config,"/gPHPEdit/php_comment_line/back","php_comment_line",DEFAULT_BACK_COLOR);
        preferences.php_comment_line_size = get_size(config,"/gPHPEdit/php_comment_line/size",DEFAULT_FONT_SIZE);
	preferences.php_comment_line_bold = gconf_client_get_bool(config,"/gPHPEdit/php_comment_line/bold",NULL);
	preferences.php_comment_line_italic = gconf_client_get_bool(config,"/gPHPEdit/php_comment_line/italic",NULL);

	preferences.css_tag_back = getcolor(config,"/gPHPEdit/css_tag/back","css_tag",DEFAULT_BACK_COLOR);
        preferences.css_tag_font = get_string(config,"/gPHPEdit/css_tag/font",DEFAULT_FONT);
	preferences.css_tag_fore = getcolor(config,"/gPHPEdit/css_tag/fore","css_tag",8388608);
        preferences.css_tag_size = get_size(config,"/gPHPEdit/css_tag/size",DEFAULT_FONT_SIZE);
	preferences.css_tag_bold = gconf_client_get_bool(config,"/gPHPEdit/css_tag/bold",NULL);
	preferences.css_tag_italic = gconf_client_get_bool(config,"/gPHPEdit/css_tag/italic",NULL);
	
	preferences.css_class_fore = getcolor(config,"/gPHPEdit/css_class/fore","css_class",8388608);
        preferences.css_class_font = get_string(config,"/gPHPEdit/css_class/font",DEFAULT_FONT);
	preferences.css_class_back = getcolor(config,"/gPHPEdit/css_class/back","css_class",DEFAULT_BACK_COLOR);
        preferences.css_class_size = get_size(config,"/gPHPEdit/css_class/size",DEFAULT_FONT_SIZE);
	preferences.css_class_bold = gconf_client_get_bool(config,"/gPHPEdit/css_class/bold",NULL);
	preferences.css_class_italic = gconf_client_get_bool(config,"/gPHPEdit/css_class/italic",NULL);
	
	preferences.css_pseudoclass_back = getcolor(config,"/gPHPEdit/css_pseudoclass/back","css_pseudoclass",DEFAULT_BACK_COLOR);
        preferences.css_pseudoclass_font = get_string(config,"/gPHPEdit/css_pseudoclass/font",DEFAULT_FONT);
	preferences.css_pseudoclass_fore = getcolor(config,"/gPHPEdit/css_pseudoclass/fore","css_pseudoclass",8388608);
        preferences.css_pseudoclass_size = get_size(config,"/gPHPEdit/css_pseudoclass/size",DEFAULT_FONT_SIZE);
	preferences.css_pseudoclass_bold = gconf_client_get_bool(config,"/gPHPEdit/css_pseudoclass/bold",NULL);
	preferences.css_pseudoclass_italic = gconf_client_get_bool(config,"/gPHPEdit/css_pseudoclass/italic",NULL);
	
	preferences.css_unknown_pseudoclass_fore = getcolor(config,"/gPHPEdit/css_unknown_pseudoclass/fore","css_unknown_pseudoclass",16711680);
        preferences.css_unknown_pseudoclass_font = get_string(config,"/gPHPEdit/css_unknown_pseudoclass/font",DEFAULT_FONT);
	preferences.css_unknown_pseudoclass_back = getcolor(config,"/gPHPEdit/css_unknown_pseudoclass/back","css_unknown_pseudoclass",DEFAULT_BACK_COLOR);
        preferences.css_unknown_pseudoclass_size = get_size(config,"/gPHPEdit/css_unknown_pseudoclass/size",DEFAULT_FONT_SIZE);
	preferences.css_unknown_pseudoclass_bold = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_pseudoclass/bold",NULL);
	preferences.css_unknown_pseudoclass_italic = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_pseudoclass/italic",NULL);
	
	preferences.css_operator_fore = getcolor(config,"/gPHPEdit/css_operator/fore","css_operator",128);
        preferences.css_operator_font = get_string(config,"/gPHPEdit/css_operator/font",DEFAULT_FONT);
	preferences.css_operator_back = getcolor(config,"/gPHPEdit/css_operator/back","css_operator",DEFAULT_BACK_COLOR);
        preferences.css_operator_size = get_size(config,"/gPHPEdit/css_operator/size",DEFAULT_FONT_SIZE);
	preferences.css_operator_bold = gconf_client_get_bool(config,"/gPHPEdit/css_operator/bold",NULL);
	preferences.css_operator_italic = gconf_client_get_bool(config,"/gPHPEdit/css_operator/italic",NULL);
	
	preferences.css_identifier_fore = gconf_client_get_int (config,"/gPHPEdit/css_identifier/fore",NULL);
	preferences.css_identifier_font = get_string(config,"/gPHPEdit/css_identifier/font",DEFAULT_FONT);
	preferences.css_identifier_back = getcolor(config,"/gPHPEdit/css_identifier/back","css_identifier",DEFAULT_BACK_COLOR);
        preferences.css_identifier_size = get_size(config,"/gPHPEdit/css_identifier/size",DEFAULT_FONT_SIZE);
	preferences.css_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/css_identifier/bold",NULL);
	preferences.css_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/css_identifier/italic",NULL);
	
	preferences.css_unknown_identifier_fore = getcolor(config,"/gPHPEdit/css_unknown_identifier/fore","css_unknown_identifier",16711680);
        preferences.css_unknown_identifier_font = get_string(config,"/gPHPEdit/css_unknown_identifier/font",DEFAULT_FONT);
	preferences.css_unknown_identifier_back = getcolor(config,"/gPHPEdit/css_unknown_identifier/back","css_unknown_identifier",DEFAULT_BACK_COLOR);
        preferences.css_unknown_identifier_size = get_size(config,"/gPHPEdit/css_unknown_identifier/size",DEFAULT_FONT_SIZE);
	preferences.css_unknown_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_identifier/bold",NULL);
	preferences.css_unknown_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/css_unknown_identifier/italic",NULL);
	
	preferences.css_value_fore = getcolor(config,"/gPHPEdit/css_value/fore","css_value",8388736);
        preferences.css_value_font = get_string(config,"/gPHPEdit/css_value/font",DEFAULT_FONT);
	preferences.css_value_back = getcolor(config,"/gPHPEdit/css_value/back","css_value",DEFAULT_BACK_COLOR);
        preferences.css_value_size = get_size(config,"/gPHPEdit/css_value/size",DEFAULT_FONT_SIZE);
	preferences.css_value_bold = gconf_client_get_bool(config,"/gPHPEdit/css_value/bold",NULL);
	preferences.css_value_italic = gconf_client_get_bool(config,"/gPHPEdit/css_value/italic",NULL);
	
	preferences.css_comment_fore = getcolor(config,"/gPHPEdit/css_comment/fore","css_comment",84215504);
        preferences.css_comment_font = get_string(config,"/gPHPEdit/css_comment/font",DEFAULT_FONT);
	preferences.css_comment_back = getcolor(config,"/gPHPEdit/css_comment/back","css_comment",DEFAULT_BACK_COLOR);
        preferences.css_comment_size = get_size(config,"/gPHPEdit/css_comment/size",DEFAULT_FONT_SIZE);
	preferences.css_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/css_comment/bold",NULL);
	preferences.css_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/css_comment/italic",NULL);
	
	preferences.css_id_font = get_string(config,"/gPHPEdit/css_id/font",DEFAULT_FONT);
	preferences.css_id_fore = getcolor(config,"/gPHPEdit/css_id/fore","css_id",8388608);
        preferences.css_id_back = getcolor(config,"/gPHPEdit/css_id/back","css_id",DEFAULT_BACK_COLOR);
        preferences.css_id_size = get_size(config,"/gPHPEdit/css_id/size",DEFAULT_FONT_SIZE);
	preferences.css_id_bold = gconf_client_get_bool(config,"/gPHPEdit/css_id/bold",NULL);
	preferences.css_id_italic = gconf_client_get_bool(config,"/gPHPEdit/css_id/italic",NULL);
	
	preferences.css_important_font = get_string(config,"/gPHPEdit/css_important/font",DEFAULT_FONT);
	preferences.css_important_fore = getcolor(config,"/gPHPEdit/css_important/fore","css_important",255);
        preferences.css_important_back = getcolor(config,"/gPHPEdit/css_important/back","css_important",DEFAULT_BACK_COLOR);
        preferences.css_important_size = get_size(config,"/gPHPEdit/css_important/size",DEFAULT_FONT_SIZE);
	preferences.css_important_bold = gconf_client_get_bool(config,"/gPHPEdit/css_important/bold",NULL);
	preferences.css_important_italic = gconf_client_get_bool(config,"/gPHPEdit/css_important/italic",NULL);
	
	preferences.css_directive_fore = getcolor(config,"/gPHPEdit/css_directive/fore","css_directive",32768);
        preferences.css_directive_font = get_string(config,"/gPHPEdit/css_directive/font",DEFAULT_FONT);
	preferences.css_directive_back = getcolor(config,"/gPHPEdit/css_directive/back","css_directive",DEFAULT_BACK_COLOR);
        preferences.css_directive_size = get_size(config,"/gPHPEdit/css_directive/size",DEFAULT_FONT_SIZE);
	preferences.css_directive_bold = gconf_client_get_bool(config,"/gPHPEdit/css_directive/bold",NULL);
	preferences.css_directive_italic = gconf_client_get_bool(config,"/gPHPEdit/css_directive/italic",NULL);
        
	preferences.sql_word_fore =  getcolor(config,"/gPHPEdit/sql_word/fore","sql_word",0);
        preferences.sql_word_font = get_string(config,"/gPHPEdit/sql_word/font",DEFAULT_FONT);
        preferences.sql_word_back = getcolor(config,"/gPHPEdit/sql_word/back","sql_word",DEFAULT_BACK_COLOR);
        preferences.sql_word_size = get_size(config,"/gPHPEdit/sql_word/size",DEFAULT_FONT_SIZE);
	preferences.sql_word_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_word/bold",NULL);
	preferences.sql_word_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_word/italic",NULL);
        
	preferences.sql_string_fore = getcolor(config,"/gPHPEdit/sql_string/fore","sql_string",8388736);
        preferences.sql_string_font = get_string(config,"/gPHPEdit/sql_string/font",DEFAULT_FONT);
	preferences.sql_string_back = getcolor(config,"/gPHPEdit/sql_string/back","sql_string",DEFAULT_BACK_COLOR);
        preferences.sql_string_size = get_size(config,"/gPHPEdit/sql_string/size",DEFAULT_FONT_SIZE);
	preferences.sql_string_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_string/bold",NULL);
	preferences.sql_string_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_string/italic",NULL);
	
	preferences.sql_operator_fore = getcolor(config,"/gPHPEdit/sql_operator/fore","sql_operator",0);
	preferences.sql_operator_font = get_string(config,"/gPHPEdit/sql_operator/font",DEFAULT_FONT);
	preferences.sql_operator_back = getcolor(config,"/gPHPEdit/sql_operator/back","sql_operator",DEFAULT_BACK_COLOR);
        preferences.sql_operator_size = get_size(config,"/gPHPEdit/sql_operator/size",DEFAULT_FONT_SIZE);
	preferences.sql_operator_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_operator/bold",NULL);
	preferences.sql_operator_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_operator/italic",NULL);
	
	preferences.sql_comment_fore = getcolor(config,"/gPHPEdit/sql_comment/fore","sql_comment",8421504);
        preferences.sql_comment_font = get_string(config,"/gPHPEdit/sql_comment/font",DEFAULT_FONT);
	preferences.sql_comment_back = getcolor(config,"/gPHPEdit/sql_comment/back","sql_comment",DEFAULT_BACK_COLOR);
        preferences.sql_comment_size = get_size(config,"/gPHPEdit/sql_comment/size",DEFAULT_FONT_SIZE);
	preferences.sql_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_comment/bold",NULL);
	preferences.sql_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_comment/italic",NULL);
	
	preferences.sql_number_fore = getcolor(config,"/gPHPEdit/sql_number/fore","sql_number",9204544);
        preferences.sql_number_font = get_string(config,"/gPHPEdit/sql_number/font",DEFAULT_FONT);
	preferences.sql_number_back = getcolor(config,"/gPHPEdit/sql_number/back","sql_number",DEFAULT_BACK_COLOR);
        preferences.sql_number_size = get_size(config,"/gPHPEdit/sql_number/size",DEFAULT_FONT_SIZE);
	preferences.sql_number_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_number/bold",NULL);
	preferences.sql_number_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_number/italic",NULL);
	
	preferences.sql_identifier_fore = getcolor(config,"/gPHPEdit/sql_identifier/fore","sql_identifier",16746496);
	preferences.sql_identifier_font = get_string(config,"/gPHPEdit/sql_identifier/font",DEFAULT_FONT);
	preferences.sql_identifier_back = getcolor(config,"/gPHPEdit/sql_identifier/back","sql_identifier",DEFAULT_BACK_COLOR);
        preferences.sql_identifier_size = get_size(config,"/gPHPEdit/sql_identifier/size",DEFAULT_FONT_SIZE);
	preferences.sql_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/sql_identifier/bold",NULL);
	preferences.sql_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/sql_identifier/italic",NULL);
        
	preferences.c_default_fore = getcolor(config,"/gPHPEdit/c_default/fore","c_default",1052688);
        preferences.c_default_font = get_string(config,"/gPHPEdit/c_default/font",DEFAULT_FONT);
	preferences.c_default_back = getcolor(config,"/gPHPEdit/c_default/back","c_default",DEFAULT_BACK_COLOR);
        preferences.c_default_size = get_size(config,"/gPHPEdit/c_default/size",DEFAULT_FONT_SIZE);
	preferences.c_default_bold = gconf_client_get_bool(config,"/gPHPEdit/c_default/bold",NULL);
	preferences.c_default_italic = gconf_client_get_bool(config,"/gPHPEdit/c_default/italic",NULL);

	preferences.c_string_fore = getcolor(config,"/gPHPEdit/c_string/fore","c_string",8388736);
        preferences.c_string_font = get_string(config,"/gPHPEdit/c_string/font",DEFAULT_FONT);
	preferences.c_string_back = getcolor(config,"/gPHPEdit/c_string/back","c_string",DEFAULT_BACK_COLOR);
	preferences.c_string_size = get_size(config,"/gPHPEdit/c_string/size",DEFAULT_FONT_SIZE);
	preferences.c_string_bold = gconf_client_get_bool(config,"/gPHPEdit/c_string/bold",NULL);
	preferences.c_string_italic = gconf_client_get_bool(config,"/gPHPEdit/c_string/italic",NULL);

        preferences.c_character_fore = getcolor(config,"/gPHPEdit/c_character/fore","c_character",8388736);
        preferences.c_character_font = get_string(config,"/gPHPEdit/c_character/font",DEFAULT_FONT);
	preferences.c_character_back = getcolor(config,"/gPHPEdit/c_character/back","c_character",DEFAULT_BACK_COLOR);
	preferences.c_character_size = get_size(config,"/gPHPEdit/c_character/size",DEFAULT_FONT_SIZE);
	preferences.c_character_bold = gconf_client_get_bool(config,"/gPHPEdit/c_character/bold",NULL);
	preferences.c_character_italic = gconf_client_get_bool(config,"/gPHPEdit/c_character/italic",NULL);

        preferences.c_word_fore = getcolor(config,"/gPHPEdit/c_word/fore","c_word",16746496);
        preferences.c_word_font = get_string(config,"/gPHPEdit/c_word/font",DEFAULT_FONT);
	preferences.c_word_back = getcolor(config,"/gPHPEdit/c_word/back","c_word",DEFAULT_BACK_COLOR);
	preferences.c_word_size = get_size(config,"/gPHPEdit/c_word/size",DEFAULT_FONT_SIZE);
	preferences.c_word_bold = gconf_client_get_bool(config,"/gPHPEdit/c_word/bold",NULL);
	preferences.c_word_italic = gconf_client_get_bool(config,"/gPHPEdit/c_word/italic",NULL);

        preferences.c_commentline_fore = getcolor(config,"/gPHPEdit/c_commentline/fore","c_commentline",8421504);
        preferences.c_commentline_font = get_string(config,"/gPHPEdit/c_commentline/font",DEFAULT_FONT);
	preferences.c_commentline_back = getcolor(config,"/gPHPEdit/c_commentline/back","c_commentline",DEFAULT_BACK_COLOR);
	preferences.c_commentline_size = get_size(config,"/gPHPEdit/c_commentline/size",DEFAULT_FONT_SIZE);
	preferences.c_commentline_bold = gconf_client_get_bool(config,"/gPHPEdit/c_commentline/bold",NULL);
	preferences.c_commentline_italic = gconf_client_get_bool(config,"/gPHPEdit/c_commentline/italic",NULL);

        preferences.c_number_fore = getcolor(config,"/gPHPEdit/c_number/fore","c_number",9204544);
        preferences.c_number_font = get_string(config,"/gPHPEdit/c_number/font",DEFAULT_FONT);
	preferences.c_number_back = getcolor(config,"/gPHPEdit/c_number/back","c_number",DEFAULT_BACK_COLOR);
	preferences.c_number_size = get_size(config,"/gPHPEdit/c_number/size",DEFAULT_FONT_SIZE);
	preferences.c_number_bold = gconf_client_get_bool(config,"/gPHPEdit/c_number/bold",NULL);
	preferences.c_number_italic = gconf_client_get_bool(config,"/gPHPEdit/c_number/italic",NULL);

        preferences.c_identifier_fore = getcolor(config,"/gPHPEdit/c_identifier/fore","c_identifier",1052688);
        preferences.c_identifier_font = get_string(config,"/gPHPEdit/c_identifier/font",DEFAULT_FONT);
	preferences.c_identifier_back = getcolor(config,"/gPHPEdit/c_identifier/back","c_identifier",DEFAULT_BACK_COLOR);
	preferences.c_identifier_size = get_size(config,"/gPHPEdit/c_identifier/size",DEFAULT_FONT_SIZE);
	preferences.c_identifier_bold = gconf_client_get_bool(config,"/gPHPEdit/c_identifier/bold",NULL);
	preferences.c_identifier_italic = gconf_client_get_bool(config,"/gPHPEdit/c_identifier/italic",NULL);

        preferences.c_comment_fore = getcolor(config,"/gPHPEdit/c_comment/fore","c_comment",8421594);
        preferences.c_comment_font = get_string(config,"/gPHPEdit/c_comment/font",DEFAULT_FONT);
	preferences.c_comment_back = getcolor(config,"/gPHPEdit/c_comment/back","c_comment",DEFAULT_BACK_COLOR);
	preferences.c_comment_size = get_size(config,"/gPHPEdit/c_comment/size",DEFAULT_FONT_SIZE);
	preferences.c_comment_bold = gconf_client_get_bool(config,"/gPHPEdit/c_comment/bold",NULL);
	preferences.c_comment_italic = gconf_client_get_bool(config,"/gPHPEdit/c_comment/italic",NULL);

        preferences.c_preprocesor_fore = getcolor(config,"/gPHPEdit/c_preprocesor/fore","c_preprocesor",7553165);
        preferences.c_preprocesor_font = get_string(config,"/gPHPEdit/c_preprocesor/font",DEFAULT_FONT);
	preferences.c_preprocesor_back = getcolor(config,"/gPHPEdit/c_preprocesor/back","c_preprocesor",DEFAULT_BACK_COLOR);
	preferences.c_preprocesor_size = get_size(config,"/gPHPEdit/c_preprocesor/size",DEFAULT_FONT_SIZE);
	preferences.c_preprocesor_bold = gconf_client_get_bool(config,"/gPHPEdit/c_preprocesor/bold",NULL);
	preferences.c_preprocesor_italic = gconf_client_get_bool(config,"/gPHPEdit/c_preprocesor/italic",NULL);

        preferences.c_operator_fore = getcolor(config,"/gPHPEdit/c_operator/fore","c_operator",128);
        preferences.c_operator_font = get_string(config,"/gPHPEdit/c_operator/font",DEFAULT_FONT);
	preferences.c_operator_back = getcolor(config,"/gPHPEdit/c_operator/back","c_operator",DEFAULT_BACK_COLOR);
	preferences.c_operator_size = get_size(config,"/gPHPEdit/c_operator/size",DEFAULT_FONT_SIZE);
	preferences.c_operator_bold = gconf_client_get_bool(config,"/gPHPEdit/c_operator/bold",NULL);
	preferences.c_operator_italic = gconf_client_get_bool(config,"/gPHPEdit/c_operator/italic",NULL);

        preferences.c_regex_fore = getcolor(config,"/gPHPEdit/c_regex/fore","c_regex",8388608);
        preferences.c_regex_font = get_string(config,"/gPHPEdit/c_regex/font",DEFAULT_FONT);
	preferences.c_regex_back = getcolor(config,"/gPHPEdit/c_regex/back","c_regex",DEFAULT_BACK_COLOR);
	preferences.c_regex_size = get_size(config,"/gPHPEdit/c_regex/size",DEFAULT_FONT_SIZE);
	preferences.c_regex_bold = gconf_client_get_bool(config,"/gPHPEdit/c_regex/bold",NULL);
	preferences.c_regex_italic = gconf_client_get_bool(config,"/gPHPEdit/c_regex/italic",NULL);

        preferences.c_uuid_fore = getcolor(config,"/gPHPEdit/c_uuid/fore","c_uuid",8388608);
        preferences.c_uuid_font = get_string(config,"/gPHPEdit/c_uuid/font",DEFAULT_FONT);
	preferences.c_uuid_back = getcolor(config,"/gPHPEdit/c_uuid/back","c_uuid",DEFAULT_BACK_COLOR);
	preferences.c_uuid_size = get_size(config,"/gPHPEdit/c_uuid/size",DEFAULT_FONT_SIZE);
	preferences.c_uuid_bold = gconf_client_get_bool(config,"/gPHPEdit/c_uuid/bold",NULL);
	preferences.c_uuid_italic = gconf_client_get_bool(config,"/gPHPEdit/c_uuid/italic",NULL);

        preferences.c_verbatim_fore = getcolor(config,"/gPHPEdit/c_verbatim/fore","c_verbatim",255);
        preferences.c_verbatim_font = get_string(config,"/gPHPEdit/c_verbatim/font",DEFAULT_FONT);
	preferences.c_verbatim_back = getcolor(config,"/gPHPEdit/c_verbatim/back","c_verbatim",DEFAULT_BACK_COLOR);
	preferences.c_verbatim_size = get_size(config,"/gPHPEdit/c_verbatim/size",DEFAULT_FONT_SIZE);
	preferences.c_verbatim_bold = gconf_client_get_bool(config,"/gPHPEdit/c_verbatim/bold",NULL);
	preferences.c_verbatim_italic = gconf_client_get_bool(config,"/gPHPEdit/c_verbatim/italic",NULL);
        
        preferences.c_globalclass_fore = getcolor(config,"/gPHPEdit/c_globalclass/fore","c_globalclass",8388608);
        preferences.c_globalclass_font = get_string(config,"/gPHPEdit/c_globalclass/font",DEFAULT_FONT);
	preferences.c_globalclass_back = getcolor(config,"/gPHPEdit/c_globalclass/back","c_globalclass",DEFAULT_BACK_COLOR);
	preferences.c_globalclass_size = get_size(config,"/gPHPEdit/c_globalclass/size",DEFAULT_FONT_SIZE);
	preferences.c_globalclass_bold = gconf_client_get_bool(config,"/gPHPEdit/c_globalclass/bold",NULL);
	preferences.c_globalclass_italic = gconf_client_get_bool(config,"/gPHPEdit/c_globalclass/italic",NULL);

	gconf_client_clear_cache(config);
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
	gconf_client_set_int (config,"/gPHPEdit/defaults/fontquality", preferences.font_quality,NULL);
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

	gconf_client_set_string (config,"/gPHPEdit/css_id/font", preferences.css_id_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/fore", preferences.css_id_fore,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/back", preferences.css_id_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_id/size", preferences.css_id_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_id/italic", preferences.css_id_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/css_id/bold", preferences.css_id_bold,NULL);

	gconf_client_set_string (config,"/gPHPEdit/css_important/font", preferences.css_important_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/css_important/fore", preferences.css_important_fore,NULL);
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

        gconf_client_set_int (config,"/gPHPEdit/c_default/fore", preferences.c_default_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_default/font", preferences.c_default_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_default/back", preferences.c_default_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_default/size", preferences.c_default_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_default/italic", preferences.c_default_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_default/bold", preferences.c_default_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_string/fore", preferences.c_string_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_string/font", preferences.c_string_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_string/back", preferences.c_string_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_string/size", preferences.c_string_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_string/italic", preferences.c_string_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_string/bold", preferences.c_string_bold,NULL);

       	gconf_client_set_int (config,"/gPHPEdit/c_character/fore", preferences.c_character_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_character/font", preferences.c_character_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_character/back", preferences.c_character_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_character/size", preferences.c_character_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_character/italic", preferences.c_character_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_character/bold", preferences.c_character_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_word/fore", preferences.c_word_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_word/font", preferences.c_word_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_word/back", preferences.c_word_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_word/size", preferences.c_word_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_word/italic", preferences.c_word_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_word/bold", preferences.c_word_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_number/fore", preferences.c_number_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_number/font", preferences.c_number_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_number/back", preferences.c_number_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_number/size", preferences.c_number_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_number/italic", preferences.c_number_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_number/bold", preferences.c_number_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_identifier/fore", preferences.c_identifier_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_identifier/font", preferences.c_identifier_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_identifier/back", preferences.c_identifier_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_identifier/size", preferences.c_identifier_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_identifier/italic", preferences.c_identifier_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_identifier/bold", preferences.c_identifier_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_comment/fore", preferences.c_comment_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_comment/font", preferences.c_comment_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_comment/back", preferences.c_comment_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_comment/size", preferences.c_comment_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_comment/italic", preferences.c_comment_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_comment/bold", preferences.c_comment_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_preprocesor/fore", preferences.c_preprocesor_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_preprocesor/font", preferences.c_preprocesor_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_preprocesor/back", preferences.c_preprocesor_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_preprocesor/size", preferences.c_preprocesor_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_preprocesor/italic", preferences.c_preprocesor_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_preprocesor/bold", preferences.c_preprocesor_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_operator/fore", preferences.c_operator_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_operator/font", preferences.c_operator_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_operator/back", preferences.c_operator_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_operator/size", preferences.c_operator_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_operator/italic", preferences.c_operator_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_operator/bold", preferences.c_operator_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_regex/fore", preferences.c_regex_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_regex/font", preferences.c_regex_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_regex/back", preferences.c_regex_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_regex/size", preferences.c_regex_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_regex/italic", preferences.c_regex_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_regex/bold", preferences.c_regex_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_uuid/fore", preferences.c_uuid_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_uuid/font", preferences.c_uuid_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_uuid/back", preferences.c_uuid_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_uuid/size", preferences.c_uuid_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_uuid/italic", preferences.c_uuid_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_uuid/bold", preferences.c_uuid_bold,NULL);


        gconf_client_set_int (config,"/gPHPEdit/c_verbatim/fore", preferences.c_verbatim_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_verbatim/font", preferences.c_verbatim_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_verbatim/back", preferences.c_verbatim_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_verbatim/size", preferences.c_verbatim_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_verbatim/italic", preferences.c_verbatim_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_verbatim/bold", preferences.c_verbatim_bold,NULL);

        gconf_client_set_int (config,"/gPHPEdit/c_globalclass/fore", preferences.c_globalclass_fore,NULL);
	gconf_client_set_string (config,"/gPHPEdit/c_globalclass/font", preferences.c_globalclass_font,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_globalclass/back", preferences.c_globalclass_back,NULL);
	gconf_client_set_int (config,"/gPHPEdit/c_globalclass/size", preferences.c_globalclass_size,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_globalclass/italic", preferences.c_globalclass_italic,NULL);
	gconf_client_set_bool (config,"/gPHPEdit/c_globalclass/bold", preferences.c_globalclass_bold,NULL);

        gconf_client_suggest_sync (config,NULL);
        g_string_free (uri, TRUE);
        gconf_client_clear_cache(config);
}
