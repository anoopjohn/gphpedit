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
#include <stdlib.h>
#include "preferences_dialog.h"
#include "main_window.h"
#include "tab_php.h"
#include "tab.h"
#include "tab_util.h"
#include "templates.h"
#include "edit_template.h"
#include <gtkscintilla.h>

#define IS_FONT_NAME(name1, name2) g_str_has_prefix(name1, name2)
PreferencesDialog preferences_dialog;
gchar *current_highlighting_element = NULL;
gchar *current_key = NULL;

static gint cmp_families (gconstpointer a, gconstpointer b, gpointer user_data)
{
  const char *a_name = pango_font_family_get_name (*(PangoFontFamily **)a);
  const char *b_name = pango_font_family_get_name (*(PangoFontFamily **)b);
  
  return g_utf8_collate (a_name, b_name);
}

static GList * get_font_names()
{
  PangoFontFamily **families;
  gint n_families, i;
  GList *fonts = NULL;
  
  pango_context_list_families (gtk_widget_get_pango_context (GTK_WIDGET (main_window.window)), &families, &n_families);
  g_qsort_with_data (families, n_families, sizeof (PangoFontFamily *), cmp_families, NULL);
  for (i=0; i<n_families; i++) {
    const gchar *name = pango_font_family_get_name (families[i]);
    /* From glib docs. Prepend and reverse list it's more eficient */
    //fonts = g_list_append(fonts, (gchar *)name);
    fonts = g_list_prepend(fonts, (gchar *)name);
  }
  fonts= g_list_reverse (fonts);
  g_free(families);
  return fonts;
}

#define CANT_SIZES 25
/* These are what we use as the standard font sizes, for the size list. */
static const gchar *font_sizes[CANT_SIZES] = {
  "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "20", "22", "24", "26", "28",
  "32", "36", "40", "48", "56", "64", "72"};

static GList *get_font_qualities()
{
  GList *qualities = NULL;
  
/*
+#define SC_EFF_QUALITY_DEFAULT 0
+#define SC_EFF_QUALITY_NON_ANTIALIASED 1
+#define SC_EFF_QUALITY_ANTIALIASED 2
+#define SC_EFF_QUALITY_LCD_OPTIMIZED 3

*/  
  qualities = g_list_prepend(qualities, "Non Antialiased");
  qualities = g_list_prepend(qualities, "LCD Optimized");  
  qualities = g_list_prepend(qualities, "Default");
  return qualities;
}
gchar sample_text[]= "<?php\n\n/* A class to implement a car\n   by Mr Somebody */\n\nclass Car extends Vehicle\n  implements EAccident\n{\n  private $PetrolTankFull = true;\n  protected $Name = \"betty\";\n  public $YearMade = 1999;\n\n  function __construct()\n  {\n    parent::__construct();\n    print \"Made $Name!\";\n  }\n  \n  private function go()\n  {\n    // Just go for it!\n    print 'Go!!!';\n  }\n}\n\n?>\n\n<html>\n <head>\n  <title>My test page</title>\n </head>\n\n <body>\n  <h1 class='winter'>Test</h1>\n </body>\n</html>  \n";
#if 0
GString *create_sample_text()
{
  GString *ret;
  
  ret = g_string_new("<?php\n\n");

  ret = g_string_append(ret, "/* A class to implement a car\n");
  ret = g_string_append(ret, "   by Mr Somebody */\n\n");
  ret = g_string_append(ret, "class Car extends Vehicle\n");
  ret = g_string_append(ret, "  implements EAccident\n");
  ret = g_string_append(ret, "{\n");
  ret = g_string_append(ret, "  private $PetrolTankFull = true;\n");
  ret = g_string_append(ret, "  protected $Name = \"betty\";\n");
  ret = g_string_append(ret, "  public $YearMade = 1999;\n\n");
  
  ret = g_string_append(ret, "  function __construct()\n");
  ret = g_string_append(ret, "  {\n");
  ret = g_string_append(ret, "    parent::__construct();\n");
  ret = g_string_append(ret, "    print \"Made $Name!\";\n");
  ret = g_string_append(ret, "  }\n");
  ret = g_string_append(ret, "  \n");
  ret = g_string_append(ret, "  private function go()\n");
  ret = g_string_append(ret, "  {\n");
  ret = g_string_append(ret, "    // Just go for it!\n");
  ret = g_string_append(ret, "    print 'Go!!!';\n");
  ret = g_string_append(ret, "  }\n");
  ret = g_string_append(ret, "}\n\n");

  ret = g_string_append(ret, "?>\n\n");

  ret = g_string_append(ret, "<html>\n");
  ret = g_string_append(ret, " <head>\n");
  ret = g_string_append(ret, "  <title>My test page</title>\n");
  ret = g_string_append(ret, " </head>\n\n");
 
  ret = g_string_append(ret, " <body>\n");
  ret = g_string_append(ret, "  <h1 class='winter'>Test</h1>\n");
  ret = g_string_append(ret, " </body>\n");
  ret = g_string_append(ret, "</html>  \n");
  return ret;
}
#endif
void get_current_preferences(void)
{
  GList *highlighting_list = NULL;

  // Font drop down elements : General
  highlighting_list = g_list_prepend(highlighting_list, _("Default"));
  highlighting_list = g_list_prepend(highlighting_list, _("Line Number (margin)"));

  // Font drop down elements : HTML
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Tag"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Unknown Tag"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Attribute"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Unknown Attribute"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Number"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Single-quoted String"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Double-quoted String"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Entity"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Script"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Question"));
  highlighting_list = g_list_prepend(highlighting_list, _("HTML Value"));

  // Font drop down elements : JavaScript
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Multiple-line Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Single-line Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Document Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Word"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Keyword"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Single-quoted String"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Double-quoted String"));
  highlighting_list = g_list_prepend(highlighting_list, _("JavaScript Symbol"));

  // Font drop down elements : PHP
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Default"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP 'HString'"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Simple String"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Word"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Number"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Variable"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Single-line Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("PHP Multiple-line Comment"));

  // Font drop down elements : CSS
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Tag"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Class"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Psuedoclass"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Unknown Pseudoclass"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Operator"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Identifier"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Unknown Identifier"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Value"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS ID"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Important"));
  highlighting_list = g_list_prepend(highlighting_list, _("CSS Directive"));

  // Font drop down elements : SQL
  highlighting_list = g_list_prepend(highlighting_list, _("SQL Word"));
  highlighting_list = g_list_prepend(highlighting_list, _("SQL Identifier"));
  highlighting_list = g_list_prepend(highlighting_list, _("SQL Number"));
  highlighting_list = g_list_prepend(highlighting_list, _("SQL String"));
  highlighting_list = g_list_prepend(highlighting_list, _("SQL Operator"));
  highlighting_list = g_list_prepend(highlighting_list, _("SQL Comment"));

  highlighting_list = g_list_prepend(highlighting_list, _("C Default"));
  highlighting_list = g_list_prepend(highlighting_list, _("C String"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Character"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Word"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Number"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Identifier"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Comment"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Commentline"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Preprocessor"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Operator"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Regex"));
  highlighting_list = g_list_prepend(highlighting_list, _("C UUID"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Verbatim"));
  highlighting_list = g_list_prepend(highlighting_list, _("C Globalclass"));

  highlighting_list= g_list_reverse (highlighting_list);  

  preferences_dialog.highlighting_elements = highlighting_list;
}

void set_controls_to_highlight(gchar *setting_name, gchar *fontname, gint fontsize, gboolean bold, gboolean italic, gint fore, gint back)
{
  gchar *sfontsize;
  gint row;
  
  // Debug print for preferences being set
//  g_print("Getting %s: %s %d %d %d %d %d\n", setting_name, fontname, fontsize, bold, italic, fore, back);
  
  preferences_dialog.changing_highlight_element=TRUE;
  
  fontname++; // Ignore the initial ! for Pango rendering
  
  row = GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (preferences_dialog.font_combo), g_quark_from_string (fontname)));
  gtk_combo_box_set_active (GTK_COMBO_BOX(preferences_dialog.font_combo), row);
  sfontsize = g_strdup_printf("%d", fontsize);

  row = GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (preferences_dialog.size_combo), g_quark_from_string (sfontsize)));
  gtk_combo_box_set_active (GTK_COMBO_BOX(preferences_dialog.size_combo), row);
  g_free(sfontsize);
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button), bold);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button), italic);

  GdkColor color;

  color.red = (fore & 0xff) << 8;
  color.green = ((fore & 0xff00) >> 8) << 8;
  color.blue = ((fore & 0xff0000) >> 16) << 8;

  gtk_color_button_set_color (GTK_COLOR_BUTTON(preferences_dialog.foreground_colour), &color);

  color.red = (back & 0xff) << 8;
  color.green = ((back & 0xff00) >> 8) << 8;
  color.blue = ((back & 0xff0000) >> 16) << 8;
  gtk_color_button_set_color (GTK_COLOR_BUTTON(preferences_dialog.background_colour), &color);

  preferences_dialog.changing_highlight_element=FALSE;
}

void get_current_highlighting_settings(gchar *name)
{
  if (!name) return;
  if (strlen(name)==0) return;
  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  if (IS_FONT_NAME(current_highlighting_element, _("Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "default_style", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("Line Number (margin)"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "line_numbers", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("Line Number (margin)"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_tag", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Tag"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_tag_unknown", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Unknown Tag"), font, size, bold, italic, fore, back);
   }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_attribute", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Attribute"), font, size, bold, italic, fore, back);
 }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_attribute_unknown", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Unknown Attribute"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_single_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Single-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_double_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Double-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_entity", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Entity"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_script", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Script"), font, size, bold, italic, fore, back); 
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_question", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Question"), font, size, bold, italic, fore, back);
 }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_value", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("HTML Value"), font, size, bold, italic, fore, back);  
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Multiple-line Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_line", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Single-line Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_doc", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Document Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_keyword", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Keyword"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_singlestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Single-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_doublestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Double-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_symbols", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("JavaScript Symbol"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_hstring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP 'HString'"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Simple String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_variable", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Variable"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Single-line Comment"), font, size, bold, italic, fore, back);  
}
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "php_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("PHP Multiple-line Comment"), font, size, bold, italic, fore, back);  
}
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_tag", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS_Tag"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_class", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Class"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Pseudoclass"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Pseudoclass"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_pseudoclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Unknown Pseudoclass"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Unknown Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_value", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Value"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_id", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Id"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_important", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Important"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "css_directive", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("CSS Directive"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("SQL Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_default", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Character"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_character", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Character"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Identifier"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Commentline"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_commentline", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Commentline"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Preprocesor"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Preprocesor"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Operator"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Regex"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_regex", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Regex"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C UUID"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_uuid", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C UUID"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Verbatim"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Verbatim"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Globalclass"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_globalclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(_("C Globalclass"), font, size, bold, italic, fore, back);
  }
}

void on_element_entry_changed(GtkComboBox *widget, gpointer     user_data)
{
  current_highlighting_element =  gtk_combo_box_get_active_text (widget);
  get_current_highlighting_settings(current_highlighting_element);
  g_free(current_highlighting_element);
}

void apply_preferences(GtkButton *button, gpointer data)
{
  GSList *walk;

  for (walk = editors; walk!=NULL; walk = g_slist_next(walk)) {
    Document *document = walk->data;
    document_refresh_properties(document);
    tab_check_php_file(document);
    tab_check_css_file(document);
    tab_check_cxx_file(document);
    tab_check_perl_file(document);
    tab_check_cobol_file(document);
    tab_check_python_file(document);
    tab_check_sql_file(document);
  }
}

void response_preferences (GtkDialog *dialog, gint response_id, gpointer   user_data)
{
  if (response_id == GTK_RESPONSE_DELETE_EVENT){
    preferences_manager_restore_data(main_window.prefmg);
  } else if (response_id == GTK_RESPONSE_ACCEPT){
      apply_preferences(NULL, NULL);      
      // Save the preferences definitely
      preferences_manager_save_data_full(main_window.prefmg);
  } else { //GTK_RESPONSE_REJECT
    preferences_manager_restore_data(main_window.prefmg);  
    apply_preferences(NULL, NULL);
  }
}
void change_font_global_callback(gint reply, gpointer data)
{
  gchar *fontname;

  //g_print("change_font_global: %d, YES=%d, NO=%d\n", reply, GTK_RESPONSE_YES, GTK_RESPONSE_NO);
  if (reply==GTK_RESPONSE_YES) {
    fontname = g_strdup((gchar *)data); 
    set_preferences_manager_style_settings(main_window.prefmg, "default_style", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "line_numbers", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_tag", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_tag_unknown", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_attribute", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_attribute_unknown", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_number", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_single_string", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_double_string", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_entity", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_script", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_question", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_value", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_line", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_doc", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_word", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_keyword", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_doublestring", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_singlestring", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_symbols", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_default", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_hstring", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_word", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_number", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_variable", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_tag", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_class", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_unknown_pseudoclass", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_operator", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_identifier", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_unknown_identifier", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_value", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_id", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_important", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_directive", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_word", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_string", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_operator", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_number", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_identifier", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_default", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_string", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_character", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_word", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_commentline", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_number", fontname , NULL, NULL, NULL, NULL, NULL);
	  set_preferences_manager_style_settings(main_window.prefmg, "c_identifier", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_comment", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_operator", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_regex", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_uuid", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", fontname , NULL, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_globalclass", fontname , NULL, NULL, NULL, NULL, NULL);
  }
  
    set_document_to_php(preferences_dialog.highlighting_document);
}


void change_size_global_callback(gint reply,gpointer data)
{
  gint fontsize;

  //g_print("change_size_global: %d\n", reply);
  if (reply==GTK_RESPONSE_YES) {
    fontsize =  GPOINTER_TO_INT (data);
    
    set_preferences_manager_style_settings(main_window.prefmg, "default_style", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "line_numbers", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_tag", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_tag_unknown", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_attribute", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_attribute_unknown", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_number", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_single_string", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_double_string", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_entity", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_script", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_question", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "html_value", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_line", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_doc", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_word", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_keyword", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_doublestring", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_singlestring", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "javascript_symbols", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_default", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_hstring", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_word", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_number", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_variable", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_tag", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_class", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_unknown_pseudoclass", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_operator", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_identifier", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_unknown_identifier", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_value", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_id", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_important", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "css_directive", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_word", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_string", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_operator", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_number", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "sql_identifier", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_default", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_string", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_character", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_word", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_commentline", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_number", NULL, &fontsize, NULL, NULL, NULL, NULL);
	  set_preferences_manager_style_settings(main_window.prefmg, "c_identifier", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_comment", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_operator", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_regex", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_uuid", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", NULL, &fontsize, NULL, NULL, NULL, NULL);
    set_preferences_manager_style_settings(main_window.prefmg, "c_globalclass", NULL, &fontsize, NULL, NULL, NULL, NULL);
  }
  set_document_to_php(preferences_dialog.highlighting_document);
}


void get_control_values_to_highlight(gchar *setting_name)
{
  GString *tempfontname;
  GString *message;
  gchar *fontname;
  gboolean bold, italic;
  gint fore, back, fontsize;

  gint newfontsize;
  gint result;

  get_preferences_manager_style_settings(main_window.prefmg, setting_name, &fontname , &fontsize, NULL, NULL, NULL, NULL);

  if (preferences_dialog.changing_highlight_element) return;
  tempfontname = g_string_new(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.font_combo)));
  tempfontname = g_string_prepend(tempfontname, "!");
  
  if (!g_str_has_prefix(fontname, tempfontname->str)) {

    message = g_string_new(NULL);
    g_string_printf(message, _("You have just changed the font to %s\n\nWould you like to use this font as the default for every element?"), gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.font_combo)));
    result = yes_no_dialog (_("gPHPEdit"), message->str);
    change_font_global_callback(result, tempfontname->str);    
    g_string_free(message, TRUE);
  }
  
  newfontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.size_combo)));
  if (fontsize != newfontsize &&
    newfontsize != 0) {
    message = g_string_new(NULL);
    g_string_printf(message, _("You have just changed the font size to %dpt\n\nWould you like to use this font size as the default for every element?"), newfontsize);
    result = yes_no_dialog (_("gPHPEdit"), message->str);
    change_size_global_callback(result, GINT_TO_POINTER(newfontsize));    
    g_string_free(message, TRUE);
    fontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.size_combo)));
  }
  bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button));
  italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button));
  GdkColor color;
  gtk_color_button_get_color (GTK_COLOR_BUTTON(preferences_dialog.foreground_colour),&color);
  fore = scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8));

  gtk_color_button_get_color (GTK_COLOR_BUTTON(preferences_dialog.background_colour),&color);
  back = scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8));
  // Debug print for preferences being set
  //g_print("Setting %s: %s %d %d %d %d %d\n", setting_name, *fontname, *fontsize, *bold, *italic, *fore, *back);  
  set_preferences_manager_style_settings(main_window.prefmg, setting_name, tempfontname->str , &newfontsize, &fore, &back, &italic, &bold);
  g_string_free(tempfontname, FALSE);
}

void set_current_highlighting_font()
{
  current_highlighting_element = gtk_combo_box_get_active_text (GTK_COMBO_BOX(preferences_dialog.element_combo));
  if (IS_FONT_NAME(current_highlighting_element, _("Default"))) {
    get_control_values_to_highlight("default_style");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("Line Number (margin)"))) {
    get_control_values_to_highlight("line_numbers");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
    get_control_values_to_highlight("html_tag");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
    get_control_values_to_highlight("html_tag_unknown");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
    get_control_values_to_highlight("html_attribute");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
    get_control_values_to_highlight("html_attribute_unknown");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
    get_control_values_to_highlight("html_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
    get_control_values_to_highlight("html_single_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
    get_control_values_to_highlight("html_double_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
    get_control_values_to_highlight("html_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
    get_control_values_to_highlight("html_entity");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
    get_control_values_to_highlight("html_script");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
    get_control_values_to_highlight("html_question");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
    get_control_values_to_highlight("html_value");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
    get_control_values_to_highlight("javascript_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
    get_control_values_to_highlight("javascript_comment_line");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
    get_control_values_to_highlight("javascript_comment_doc");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
    get_control_values_to_highlight("javascript_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
    get_control_values_to_highlight("javascript_keyword");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
    get_control_values_to_highlight("javascript_singlestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
    get_control_values_to_highlight("javascript_doublestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
    get_control_values_to_highlight("javascript_symbols");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
    get_control_values_to_highlight("php_default");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
    get_control_values_to_highlight("php_hstring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
    get_control_values_to_highlight("php_simplestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
    get_control_values_to_highlight("php_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
    get_control_values_to_highlight("php_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
    get_control_values_to_highlight("php_variable");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
    get_control_values_to_highlight("php_comment_line");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
    get_control_values_to_highlight("php_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
    get_control_values_to_highlight("css_tag");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
    get_control_values_to_highlight("css_class");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Psuedoclass"))) {
    get_control_values_to_highlight("css_pseudoclass");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
    get_control_values_to_highlight("css_unknown_pseudoclass");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
    get_control_values_to_highlight("css_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
    get_control_values_to_highlight("css_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
    get_control_values_to_highlight("css_unknown_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
    get_control_values_to_highlight("css_value");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
    get_control_values_to_highlight("css_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
    get_control_values_to_highlight("css_id");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
    get_control_values_to_highlight("css_important");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
    get_control_values_to_highlight("css_directive");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
    get_control_values_to_highlight("sql_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
    get_control_values_to_highlight("sql_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
    get_control_values_to_highlight("sql_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
    get_control_values_to_highlight("sql_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
    get_control_values_to_highlight("sql_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
    get_control_values_to_highlight("sql_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Default"))) {
    get_control_values_to_highlight("c_default");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C String"))) {
    get_control_values_to_highlight("c_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Character"))) {
    get_control_values_to_highlight("c_character");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Word"))) {
    get_control_values_to_highlight("c_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Number"))) {
    get_control_values_to_highlight("c_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Identifier"))) {
    get_control_values_to_highlight("c_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Comment"))) {
    get_control_values_to_highlight("c_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Commentline"))) {
    get_control_values_to_highlight("c_commentline");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Preprocesor"))) {
    get_control_values_to_highlight("c_preprocesor");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Operator"))) {
    get_control_values_to_highlight("c_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Regex"))) {
    get_control_values_to_highlight("c_regex");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C UUID"))) {
    get_control_values_to_highlight("c_uuid");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Verbatim"))) {
    get_control_values_to_highlight("c_verbatim");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Globalclass"))) {
    get_control_values_to_highlight("c_globalclass");
  }
  tab_check_php_file(preferences_dialog.highlighting_document);
  g_free(current_highlighting_element);
}

void on_bold_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_current_highlighting_font();
}

void on_italic_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_current_highlighting_font();
}

void on_fontname_entry_changed(GtkEntry *Entry, gpointer data)
{
  set_current_highlighting_font();
}

void on_fontsize_entry_changed(GtkEntry *Entry, gpointer data)
{
  set_current_highlighting_font();
}

void preferences_window_destroyed(GtkWidget *widget, gpointer data)
{
  gtk_grab_remove(preferences_dialog.window);
  preferences_dialog.window = NULL;
}

void on_fore_changed(GtkColorButton *widget, gpointer user_data)
{
  set_current_highlighting_font();
}

void on_back_changed(GtkColorButton *widget, gpointer user_data)
{
  set_current_highlighting_font();
}

/**
 * Callback registered for setting the edge color from 
 * preferences dialog.
 */
void on_edge_colour_changed(GtkColorButton *widget, gpointer user_data)
{
  GdkColor color;
  gtk_color_button_get_color (widget,&color);
  set_preferences_manager_edge_colour(main_window.prefmg, scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8)));
}

void on_fontqualities_entry_changed(GtkEntry *Entry, gpointer data)
{
  gchar *texttemp=gtk_combo_box_get_active_text(GTK_COMBO_BOX(preferences_dialog.fontstyle));

  if(g_strcmp0(texttemp,"Non Antialiased")==0){
    set_preferences_manager_font_quality(main_window.prefmg, SC_EFF_QUALITY_NON_ANTIALIASED);    
  } else if(g_strcmp0(texttemp,"LCD Optimized")==0){
    set_preferences_manager_font_quality(main_window.prefmg, SC_EFF_QUALITY_LCD_OPTIMIZED);
  } else {
    /* set default */
    set_preferences_manager_font_quality(main_window.prefmg, SC_EFF_QUALITY_DEFAULT);
  }
  g_free(texttemp);
}

/**
 * Callback registered for setting the selection background color from 
 * preferences dialog.
 */
void on_sel_back_changed(GtkColorButton *widget, gpointer user_data)
{
  GdkColor color;
  gtk_color_button_get_color (widget, &color);
  set_preferences_manager_set_sel_back(main_window.prefmg, scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8)));
}

void on_caretline_back_changed(GtkColorButton *widget, gpointer user_data)
{
  GdkColor color;
  gtk_color_button_get_color (widget, &color);

  set_preferences_manager_higthlight_caret_line_color(main_window.prefmg, scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8)));
}

void on_tab_size_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  set_preferences_manager_tab_size(main_window.prefmg, gtk_spin_button_get_value_as_int(spinbutton));
  set_preferences_manager_indentation_size(main_window.prefmg, gtk_spin_button_get_value_as_int(spinbutton));
}

void on_calltip_delay_changed(GtkRange *range, gpointer user_data)
{
  set_preferences_manager_calltip_delay(main_window.prefmg, (int) gtk_range_get_value(range));
  set_preferences_manager_auto_complete_delay(main_window.prefmg, (int) gtk_range_get_value(range));
}

void on_edge_column_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  set_preferences_manager_edge_column(main_window.prefmg, gtk_spin_button_get_value_as_int(spinbutton));
}

void on_show_indentation_guides_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_show_indentation_guides(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));  
}

void on_edge_mode_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_edge_mode(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));  
}

void on_line_wrapping_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_line_wrapping(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_use_tabs_instead_spaces_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_use_tabs_instead_spaces(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_save_session_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
 set_preferences_manager_saved_session(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_save_folderbrowser_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_show_filebrowser(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}
void on_save_autobrace_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_auto_complete_braces(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}
void on_save_higthlightcaretline_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_higthlight_caret_line(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_single_instance_only_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_single_instance_only(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_php_binary_location_changed (GtkEntry *entry, gpointer user_data)
{
  set_preferences_manager_php_binary_location(main_window.prefmg, g_strdup(gtk_entry_get_text(entry)));
}

void on_php_file_extensions_changed (GtkEntry *entry, gpointer user_data)
{
  set_preferences_manager_php_file_extensions(main_window.prefmg, g_strdup(gtk_entry_get_text(entry)));
}

void on_shared_source_changed (GtkEntry *entry, gpointer user_data)
{
  set_preferences_manager_shared_source_location(main_window.prefmg, g_strdup(gtk_entry_get_text(entry)));
}

void update_template_display(gchar *template)
{
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (preferences_dialog.template_sample));

  gtk_text_buffer_set_text (buffer, template, -1);  
}

static void add_to_template_list(gpointer key, gpointer value, gpointer user_data)
{
  GtkTreeIter iter;

  gtk_list_store_append (preferences_dialog.template_store, &iter);
  gtk_list_store_set (preferences_dialog.template_store, &iter,0, (gchar *)key, -1);
}

void add_templates_to_store(void)
{
  g_hash_table_foreach(templates, add_to_template_list, NULL);
}

void add_template_clicked(GtkButton *button, gpointer data)
{
  gchar *name;
  gchar *template;
  GtkTreeIter iter;
  GtkTextBuffer *buffer;
  GtkTextIter begin, end;
  gchar *content;

  // create dialog
  create_edit_template_dialog();

  // Run and wait for OK
  if (gtk_dialog_run(GTK_DIALOG(edit_template_dialog.window1)) == GTK_RESPONSE_ACCEPT) {
    // convert content to template format
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit_template_dialog.textview1));
    gtk_text_buffer_get_start_iter (buffer, &begin);
    gtk_text_buffer_get_end_iter (buffer, &end);
    content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
    
    template = template_convert_to_template(content);

    // add to templates
    name = (gchar *)gtk_entry_get_text (GTK_ENTRY(edit_template_dialog.entry1));
    template_delete(current_key); // Just in case you accidentally type the name of an existing template
    template_replace(name, template);
    
    // add to treeview
    gtk_list_store_append (preferences_dialog.template_store, &iter);
    gtk_list_store_set (preferences_dialog.template_store, &iter,0, (gchar *)name, -1);
  }
  
  // destroy/null dialog
  gtk_widget_destroy(edit_template_dialog.window1);
  edit_template_dialog.window1 = NULL;
}

void edit_template_clicked(GtkButton *button, gpointer data)
{
  gchar *name;
  gchar *template;
  GtkTextBuffer *buffer;
  GtkTextIter begin, end;
  gchar *content;
  GtkTreeIter iter;

  // create dialog
  create_edit_template_dialog();

  // fill in existing content
  gtk_entry_set_text(GTK_ENTRY(edit_template_dialog.entry1), current_key);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (preferences_dialog.template_sample));
  gtk_text_buffer_get_start_iter (buffer, &begin);
  gtk_text_buffer_get_end_iter (buffer, &end);
  content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit_template_dialog.textview1));
  gtk_text_buffer_set_text(buffer, content, -1);  
  
  // Run and wait for OK
  if (gtk_dialog_run(GTK_DIALOG(edit_template_dialog.window1)) == GTK_RESPONSE_ACCEPT) {
    // convert content to template format
    buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (edit_template_dialog.textview1));
    gtk_text_buffer_get_start_iter (buffer, &begin);
    gtk_text_buffer_get_end_iter (buffer, &end);
    content = gtk_text_buffer_get_text(buffer, &begin, &end, TRUE);
    
    template = template_convert_to_template(content);

    // add to templates
    name = (gchar *)gtk_entry_get_text (GTK_ENTRY(edit_template_dialog.entry1));
    template_delete(current_key);
    template_replace(name, template);
    
    // replace in treeview
    gtk_tree_selection_get_selected (preferences_dialog.template_selection, NULL, &iter);
    gtk_list_store_set (preferences_dialog.template_store, &iter,0, (gchar *)name, -1);
    
    // replace in display
    update_template_display(content);
  }
  
  // destroy/null dialog
  gtk_widget_destroy(edit_template_dialog.window1);
  edit_template_dialog.window1 = NULL;
}

void delete_template_clicked(GtkButton *button, gpointer data)
{
  GtkWidget *confirm_dialog;
  GtkTreeIter iter;

  confirm_dialog = gtk_message_dialog_new(GTK_WINDOW(main_window.window),GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
    GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        _("Are you sure you want to delete template %s?"),current_key);
  // confirm deletion with dialog
  if (gtk_dialog_run(GTK_DIALOG(confirm_dialog)) == GTK_RESPONSE_YES) {
    // delete from templates
    template_delete(current_key);

    // delete from treeview
    gtk_tree_selection_get_selected (preferences_dialog.template_selection, NULL, &iter);
    gtk_list_store_remove (preferences_dialog.template_store, &iter);
    current_key = NULL;
    /*
    gtk_tree_selection_unselect_all (GTK_TREE_SELECTION(preferences_dialog.template_selection));
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(preferences_dialog.Templates));
    g_object_ref(model);
    gtk_tree_view_set_model(GTK_TREE_VIEW(preferences_dialog.Templates), NULL);
    //gtk_list_store_remove (preferences_dialog.template_store, &current_template_iter);
    gtk_list_store_clear(preferences_dialog.template_store);
    gtk_tree_view_set_model(GTK_TREE_VIEW(preferences_dialog.Templates), model);
    g_object_unref(model);
    add_templates_to_store();*/
  }
  
  // destroy dialog
  gtk_widget_destroy(confirm_dialog);
}

void template_row_activated(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeModel *model;
  gchar *content, *template;
  GtkTreeIter iter;

  if (current_key) {
    g_free(current_key);
  }
  //g_print("a\n");
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, 0, &current_key, -1);

    // display template content
    template = template_find(current_key);
    if (template) {
      content = template_convert_to_display(template);
      update_template_display(content);
      g_free(content);
    }
  }
}

static void templates_treeview_add_column(void)
{  
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* column for description */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name/Shortcut"), renderer, "text", 0, NULL);
  gtk_tree_view_column_set_sort_column_id (column, 0);
  gtk_tree_view_append_column (GTK_TREE_VIEW(preferences_dialog.Templates), column);
}

void preferences_dialog_create (void)
{

  GList *comboitems = NULL;
  GList *items = NULL;
//  GString *sample_text;
  
  get_current_preferences();

  preferences_dialog.window = gtk_dialog_new_with_buttons(_("Preferences"), GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL |  GTK_DIALOG_DESTROY_WITH_PARENT,
GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  
  preferences_dialog.notebook1 = gtk_notebook_new ();
  gtk_widget_show (preferences_dialog.notebook1);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(preferences_dialog.window))),preferences_dialog.notebook1);
  
  preferences_dialog.prinbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (preferences_dialog.prinbox);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.prinbox), 8);

  /* current session */  
  preferences_dialog.lblseccion=gtk_frame_new (_("Session:"));
  gtk_widget_show (preferences_dialog.lblseccion);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinbox), preferences_dialog.lblseccion, FALSE, FALSE, 0);

  preferences_dialog.sessionbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (preferences_dialog.sessionbox);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.lblseccion),  preferences_dialog.sessionbox);

  preferences_dialog.save_session = gtk_check_button_new_with_mnemonic (_("Save session (including open files) on exit"));
  gtk_widget_show (preferences_dialog.save_session);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.sessionbox), preferences_dialog.save_session, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.save_session), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.save_session), get_preferences_manager_saved_session(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.save_session)), "toggled", G_CALLBACK(on_save_session_toggle), NULL);

  preferences_dialog.single_instance_only = gtk_check_button_new_with_mnemonic (_("Only ever run 1 copy of gPHPEdit at a time"));
  gtk_widget_show (preferences_dialog.single_instance_only);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.sessionbox), preferences_dialog.single_instance_only, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.single_instance_only), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.single_instance_only), get_preferences_manager_single_instance_only(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.single_instance_only)), "toggled", G_CALLBACK(on_single_instance_only_toggle), NULL);
  /*end session*/

/*edge part*/

  preferences_dialog.edge = gtk_frame_new (_("Hand edge guide:"));
  gtk_widget_show (preferences_dialog.edge);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinbox), preferences_dialog.edge, FALSE, FALSE, 0);

  preferences_dialog.edgebox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (preferences_dialog.edgebox);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.edge), preferences_dialog.edgebox);

  preferences_dialog.edge_mode = gtk_check_button_new_with_mnemonic (_("Show right hand edge guide"));
  gtk_widget_show (preferences_dialog.edge_mode);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.edgebox), preferences_dialog.edge_mode);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.edge_mode), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.edge_mode), get_preferences_manager_edge_mode(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.edge_mode)), "toggled", G_CALLBACK(on_edge_mode_toggle), NULL);
  
  /*Begin: Right Hand Edge Color*/
  preferences_dialog.hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox15);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.edgebox), preferences_dialog.hbox15);
  
  preferences_dialog.label33 = gtk_label_new (_("Right hand edge colour:"));
  gtk_widget_show (preferences_dialog.label33);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox15), preferences_dialog.label33, FALSE, FALSE, 8);

  GdkColor color;
  color.red = (get_preferences_manager_edge_colour(main_window.prefmg) & 0xff) << 8;
  color.green = ((get_preferences_manager_edge_colour(main_window.prefmg) & 0xff00) >> 8) << 8;
  color.blue = ((get_preferences_manager_edge_colour(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  preferences_dialog.edge_colour = gtk_color_button_new_with_color (&color);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(preferences_dialog.edge_colour),&color);

  gtk_widget_show (preferences_dialog.edge_colour);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox15), preferences_dialog.edge_colour, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(preferences_dialog.edge_colour)), "color-set", G_CALLBACK(on_edge_colour_changed), NULL);
  /*End: Right Hand Edge Color*/
  preferences_dialog.hbox16 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox16);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.edgebox), preferences_dialog.hbox16);
  
  preferences_dialog.label34 = gtk_label_new (_("Right hand edge column"));
  gtk_widget_show (preferences_dialog.label34);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox16), preferences_dialog.label34, FALSE, FALSE, 8);
  
  preferences_dialog.edge_column = gtk_spin_button_new_with_range(0, 160, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(preferences_dialog.edge_column), get_preferences_manager_edge_column(main_window.prefmg));
  gtk_widget_show (preferences_dialog.edge_column);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox16), preferences_dialog.edge_column, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (preferences_dialog.edge_column), "value_changed",
                    G_CALLBACK (on_edge_column_changed), NULL);

/*end edge part*/

  preferences_dialog.sidepanel = gtk_frame_new (_("Side Panel:"));
  gtk_widget_show (preferences_dialog.sidepanel);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinbox), preferences_dialog.sidepanel, FALSE, FALSE, 0);

  preferences_dialog.folderbrowser = gtk_check_button_new_with_mnemonic (_("Show file browser (need restart)"));
  /* set tooltip */
  gtk_widget_set_tooltip_text (preferences_dialog.folderbrowser,_("Show/Hide side-panel Folder Browser"));
  gtk_widget_show (preferences_dialog.folderbrowser);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.sidepanel), preferences_dialog.folderbrowser);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.folderbrowser), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.folderbrowser), get_preferences_manager_show_filebrowser(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.folderbrowser)), "toggled", G_CALLBACK(on_save_folderbrowser_toggle), NULL);
  /*end side panel part*/

/* autocompletion part */
  preferences_dialog.autocomp = gtk_frame_new (_("Auto-completion:"));
  gtk_widget_show (preferences_dialog.autocomp);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinbox), preferences_dialog.autocomp, FALSE, FALSE, 0);

  preferences_dialog.autobrace = gtk_check_button_new_with_mnemonic (_("Auto-Completion of Brackets and Quotes"));
  /* set tooltip */
  gtk_widget_set_tooltip_text (preferences_dialog.autobrace,_("Auto-complete the closing brackets/quotes"));
  gtk_widget_show (preferences_dialog.autobrace);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.autocomp), preferences_dialog.autobrace);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.autobrace), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.autobrace), get_preferences_manager_auto_complete_braces(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.autobrace)), "toggled", G_CALLBACK(on_save_autobrace_toggle), NULL);

/*end autocompletion part*/

  preferences_dialog.lblgen = gtk_label_new (_("General"));
  gtk_widget_show (preferences_dialog.lblgen);
  gtk_notebook_prepend_page (GTK_NOTEBOOK (preferences_dialog.notebook1),preferences_dialog.prinbox, preferences_dialog.lblgen);

/*editor tab */

  preferences_dialog.prinboxed = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (preferences_dialog.prinboxed);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.prinboxed), 8);

/* tabs part */  
  preferences_dialog.lbltabs=gtk_frame_new (_("Tabs:"));
  gtk_widget_show (preferences_dialog.lbltabs);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinboxed), preferences_dialog.lbltabs, FALSE, FALSE, 0);

  preferences_dialog.tabsbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (preferences_dialog.tabsbox);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.lbltabs),  preferences_dialog.tabsbox);

  preferences_dialog.hbox14 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox14);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.tabsbox), preferences_dialog.hbox14, TRUE, FALSE, 0);

  preferences_dialog.label32 = gtk_label_new (_("Tab size:"));
  gtk_widget_show (preferences_dialog.label32);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox14), preferences_dialog.label32, FALSE, FALSE, 8);

  preferences_dialog.tab_size = gtk_spin_button_new_with_range(1, 16, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(preferences_dialog.tab_size), get_preferences_manager_tab_size(main_window.prefmg));
  gtk_widget_show (preferences_dialog.tab_size);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox14), preferences_dialog.tab_size, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (preferences_dialog.tab_size), "value_changed",
                    G_CALLBACK (on_tab_size_changed), NULL);

  preferences_dialog.use_tabs_instead_spaces = gtk_check_button_new_with_mnemonic (_("Use tabs instead of spaces for indentation"));
  gtk_widget_show (preferences_dialog.use_tabs_instead_spaces);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.tabsbox), preferences_dialog.use_tabs_instead_spaces, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.use_tabs_instead_spaces), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.use_tabs_instead_spaces), get_preferences_manager_use_tabs_instead_spaces(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.use_tabs_instead_spaces)), "toggled", G_CALLBACK(on_use_tabs_instead_spaces_toggle), NULL);

  preferences_dialog.show_indentation_guides = gtk_check_button_new_with_mnemonic (_("Show indentation guides"));
  gtk_widget_show (preferences_dialog.show_indentation_guides);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.tabsbox), preferences_dialog.show_indentation_guides, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.show_indentation_guides), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.show_indentation_guides), get_preferences_manager_show_indentation_guides(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.show_indentation_guides)), "toggled", G_CALLBACK(on_show_indentation_guides_toggle), NULL);

  preferences_dialog.lblwrap=gtk_frame_new (_("Text wrap:"));
  gtk_widget_show (preferences_dialog.lblwrap);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinboxed), preferences_dialog.lblwrap, FALSE, FALSE, 0);

  preferences_dialog.line_wrapping = gtk_check_button_new_with_mnemonic (_("Wrap long lines"));
  gtk_widget_show (preferences_dialog.line_wrapping);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.lblwrap), preferences_dialog.line_wrapping);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.line_wrapping), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.line_wrapping), get_preferences_manager_line_wrapping(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.line_wrapping)), "toggled", G_CALLBACK(on_line_wrapping_toggle), NULL);


  preferences_dialog.lblsel=gtk_frame_new (_("Selection:"));
  gtk_widget_show (preferences_dialog.lblsel);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinboxed), preferences_dialog.lblsel, FALSE, FALSE, 0);

  /*Begin: Selection Color*/  
  preferences_dialog.vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.vbox10);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.vbox10), 8);

  preferences_dialog.hbox27 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox27);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.lblsel), preferences_dialog.hbox27);
  
  preferences_dialog.label46 = gtk_label_new (_("Selection colour:"));
  gtk_widget_show (preferences_dialog.label46);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox27), preferences_dialog.label46, FALSE, FALSE, 8);

  GdkColor sel_back;
  sel_back.red = (get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff) << 8;
  sel_back.green = ((get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff00) >> 8) << 8;
  sel_back.blue = ((get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  preferences_dialog.sel_back = gtk_color_button_new_with_color (&sel_back);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(preferences_dialog.sel_back),&sel_back);

  gtk_widget_show (preferences_dialog.sel_back);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox27), preferences_dialog.sel_back, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(preferences_dialog.sel_back)), "color-set", G_CALLBACK(on_sel_back_changed), NULL);
  /*End: Selection Color*/

  preferences_dialog.lblcurl=gtk_frame_new (_("Current line:"));
  gtk_widget_show (preferences_dialog.lblcurl);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinboxed), preferences_dialog.lblcurl, FALSE, FALSE, 0);
  
  preferences_dialog.curlbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.curlbox);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.lblcurl), preferences_dialog.curlbox);

  preferences_dialog.higthlightcaretline = gtk_check_button_new_with_mnemonic (_("Highlight Caret Line"));
  /* set tooltip */
  gtk_widget_show (preferences_dialog.higthlightcaretline);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.curlbox), preferences_dialog.higthlightcaretline, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.higthlightcaretline), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(preferences_dialog.higthlightcaretline), get_preferences_manager_higthlight_caret_line(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(preferences_dialog.higthlightcaretline)), "toggled", G_CALLBACK(on_save_higthlightcaretline_toggle), NULL);

  preferences_dialog.colcaret = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.colcaret);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.curlbox), preferences_dialog.colcaret);
  
  preferences_dialog.lblcol = gtk_label_new (_("Highlight Caret Line colour:"));
  gtk_widget_show (preferences_dialog.lblcol);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.colcaret), preferences_dialog.lblcol, FALSE, FALSE, 8);

  GdkColor caret_back;
  caret_back.red = (get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff) << 8;
  caret_back.green = ((get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff00) >> 8) << 8;
  caret_back.blue = ((get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  preferences_dialog.caretline_color = gtk_color_button_new_with_color (&caret_back);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(preferences_dialog.caretline_color),&caret_back);

  gtk_widget_show (preferences_dialog.caretline_color);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.colcaret), preferences_dialog.caretline_color, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(preferences_dialog.caretline_color)), "color-set", G_CALLBACK(on_caretline_back_changed), NULL);

  /*  font quality */

  preferences_dialog.hboxfs = gtk_frame_new (NULL);
  gtk_widget_show (preferences_dialog.hboxfs);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.prinboxed), preferences_dialog.hboxfs, FALSE, FALSE, 0);

  preferences_dialog.fontlabel = gtk_label_new (_("Font Quality"));
  gtk_widget_show (preferences_dialog.fontlabel);
  gtk_frame_set_label_widget (GTK_FRAME (preferences_dialog.hboxfs), preferences_dialog.fontlabel);

  preferences_dialog.fontstyle = gtk_combo_box_entry_new_text ();
  gtk_widget_show (preferences_dialog.fontstyle);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.hboxfs), preferences_dialog.fontstyle);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.fontstyle), 8);
  
  comboitems = get_font_qualities();
  for (items = g_list_first(comboitems); items != NULL; items = g_list_next(items)) {
    // Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
    g_object_set_qdata (G_OBJECT (preferences_dialog.fontstyle), g_quark_from_string (items->data), 
      GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(preferences_dialog.fontstyle)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX (preferences_dialog.fontstyle), items->data);
  }
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (preferences_dialog.fontstyle)), "changed",
                      G_CALLBACK (on_fontqualities_entry_changed),
                      NULL);

  g_list_free (comboitems);
  /* set actual quality */
  gtk_combo_box_set_active (GTK_COMBO_BOX(preferences_dialog.fontstyle), (get_preferences_manager_font_quality(main_window.prefmg)!=0)?get_preferences_manager_font_quality(main_window.prefmg) -1 :0);

/*end editor page */
  preferences_dialog.lbled = gtk_label_new (_("Editor"));
  gtk_widget_show (preferences_dialog.lbled);
  gtk_notebook_append_page (GTK_NOTEBOOK (preferences_dialog.notebook1),preferences_dialog.prinboxed, preferences_dialog.lbled);

/*color page */

  preferences_dialog.vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.vbox10);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.vbox10), 8);

  preferences_dialog.hbox22 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox22);
  gtk_box_pack_end (GTK_BOX (preferences_dialog.vbox10), preferences_dialog.hbox22, FALSE, TRUE, 8);
  
  preferences_dialog.frame2 = gtk_frame_new (NULL);
  gtk_widget_show (preferences_dialog.frame2);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox22), preferences_dialog.frame2, TRUE, TRUE, 8);
  
  preferences_dialog.vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.vbox11);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.frame2), preferences_dialog.vbox11);
  
  preferences_dialog.bold_button = gtk_check_button_new_with_mnemonic (_("Bold"));
  gtk_widget_show (preferences_dialog.bold_button);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox11), preferences_dialog.bold_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.bold_button), 8);
  g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(preferences_dialog.bold_button)), "toggled", G_CALLBACK(on_bold_toggle), NULL);
  
  preferences_dialog.italic_button = gtk_check_button_new_with_mnemonic (_("Italic"));
  gtk_widget_show (preferences_dialog.italic_button);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox11), preferences_dialog.italic_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.italic_button), 8);
  g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(preferences_dialog.italic_button)), "toggled", G_CALLBACK(on_italic_toggle), NULL);
  
  preferences_dialog.label40 = gtk_label_new (_("Attributes"));
  gtk_widget_show (preferences_dialog.label40);
  gtk_frame_set_label_widget (GTK_FRAME (preferences_dialog.frame2), preferences_dialog.label40);
  
  preferences_dialog.frame3 = gtk_frame_new (NULL);
  gtk_widget_show (preferences_dialog.frame3);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox22), preferences_dialog.frame3, TRUE, TRUE, 8);
  
  preferences_dialog.vbox12 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.vbox12);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.frame3), preferences_dialog.vbox12);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.vbox12), 8);
  
  preferences_dialog.hbox23 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox23);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox12), preferences_dialog.hbox23, FALSE, TRUE, 0);
  
  preferences_dialog.label42 = gtk_label_new (_("Foreground:"));
  gtk_widget_show (preferences_dialog.label42);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox23), preferences_dialog.label42, FALSE, FALSE, 0);
  
  preferences_dialog.foreground_colour = gtk_color_button_new();
  gtk_widget_show (preferences_dialog.foreground_colour);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox23), preferences_dialog.foreground_colour, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.foreground_colour), 8);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(preferences_dialog.foreground_colour)), "color-set", G_CALLBACK(on_fore_changed), NULL);
  preferences_dialog.hbox24 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox24);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox12), preferences_dialog.hbox24, FALSE, TRUE, 0);
  
  preferences_dialog.label43 = gtk_label_new (_("Background:"));
  gtk_widget_show (preferences_dialog.label43);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox24), preferences_dialog.label43, FALSE, FALSE, 0);
  
  preferences_dialog.background_colour = gtk_color_button_new();
  gtk_widget_show (preferences_dialog.background_colour);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox24), preferences_dialog.background_colour, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.background_colour), 8);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(preferences_dialog.background_colour)), "color-set", G_CALLBACK(on_fore_changed), NULL);
  
  preferences_dialog.label41 = gtk_label_new (_("Colours"));
  gtk_widget_show (preferences_dialog.label41);
  gtk_frame_set_label_widget (GTK_FRAME (preferences_dialog.frame3), preferences_dialog.label41);
  
  preferences_dialog.highlighting_document = document_new (TAB_FILE, "", 0);
  document_set_scintilla(preferences_dialog.highlighting_document, gtk_scintilla_new());
  document_add_text(preferences_dialog.highlighting_document, sample_text);
  document_goto_pos(preferences_dialog.highlighting_document, 0);
  document_set_readonly(preferences_dialog.highlighting_document, TRUE, TRUE);
  set_document_to_php(preferences_dialog.highlighting_document);

  preferences_dialog.code_sample = document_get_editor_widget(preferences_dialog.highlighting_document);
  gtk_widget_set_size_request (preferences_dialog.code_sample, 200, 200);
  gtk_widget_show (preferences_dialog.code_sample);
  gtk_box_pack_end (GTK_BOX (preferences_dialog.vbox10), preferences_dialog.code_sample, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.code_sample), 8);
  
  preferences_dialog.frame4 = gtk_frame_new (NULL);
  gtk_widget_show (preferences_dialog.frame4);
  gtk_box_pack_end (GTK_BOX (preferences_dialog.vbox10), preferences_dialog.frame4, TRUE, TRUE, 0);
  
  preferences_dialog.hbox25 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox25);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.frame4), preferences_dialog.hbox25);
  
  preferences_dialog.font_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (preferences_dialog.font_combo);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox25), preferences_dialog.font_combo, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.font_combo), 8);
  
  comboitems = get_font_names();
  for (items = g_list_first(comboitems); items != NULL; items = g_list_next(items)) {
    // Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
    g_object_set_qdata (G_OBJECT (preferences_dialog.font_combo), g_quark_from_string (items->data), 
      GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(preferences_dialog.font_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX (preferences_dialog.font_combo), items->data);
  }
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (preferences_dialog.font_combo)), "changed",
                      G_CALLBACK (on_fontname_entry_changed),
                      NULL);
  g_list_free (comboitems);

  preferences_dialog.size_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (preferences_dialog.size_combo);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox25), preferences_dialog.size_combo, FALSE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.size_combo), 8);
  
  int i;
  for (i=0; i<CANT_SIZES; i++) {
    // Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
    g_object_set_qdata (G_OBJECT (preferences_dialog.size_combo), g_quark_from_static_string (font_sizes[i]), 
      GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(preferences_dialog.size_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX (preferences_dialog.size_combo), font_sizes[i]);
    //g_print("Appending Font Size: %s, %d\n", font_sizes[i], g_quark_from_static_string(font_sizes[i]));
  }
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (preferences_dialog.size_combo)), "changed",
                      G_CALLBACK (on_fontsize_entry_changed),
                      NULL);
  preferences_dialog.label44 = gtk_label_new (_("Font"));
  gtk_widget_show (preferences_dialog.label44);
  gtk_frame_set_label_widget (GTK_FRAME (preferences_dialog.frame4), preferences_dialog.label44);
  
  preferences_dialog.frame1 = gtk_frame_new (NULL);
  gtk_widget_show (preferences_dialog.frame1);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox10), preferences_dialog.frame1, TRUE, TRUE, 0);
  
  preferences_dialog.element_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (preferences_dialog.element_combo);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.frame1), preferences_dialog.element_combo);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.element_combo), 8);
  GList *walk = NULL;
  for (walk = preferences_dialog.highlighting_elements; walk != NULL; walk = g_list_next (walk)) {
    g_object_set_qdata (G_OBJECT (preferences_dialog.element_combo), g_quark_from_string (walk->data),
    GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(preferences_dialog.element_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX(preferences_dialog.element_combo),walk->data);
  }
  current_highlighting_element = g_list_first(preferences_dialog.highlighting_elements)->data;
  gtk_combo_box_set_active (GTK_COMBO_BOX(preferences_dialog.element_combo),0);
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (preferences_dialog.element_combo)), "changed",
                      G_CALLBACK (on_element_entry_changed),
                      NULL);
  g_list_free (preferences_dialog.highlighting_elements);
  
  preferences_dialog.label39 = gtk_label_new (_("Element"));
  gtk_widget_show (preferences_dialog.label39);
  gtk_frame_set_label_widget (GTK_FRAME (preferences_dialog.frame1), preferences_dialog.label39);
  
  preferences_dialog.label30 = gtk_label_new (_("Colour"));
  gtk_widget_show (preferences_dialog.label30);
  gtk_notebook_append_page (GTK_NOTEBOOK (preferences_dialog.notebook1),preferences_dialog.vbox10, preferences_dialog.label30);

/**/
  preferences_dialog.vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.vbox7);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.vbox7), 0);
  
  preferences_dialog.hbox17 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox17);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox7), preferences_dialog.hbox17, FALSE, TRUE, 8);
  
  preferences_dialog.label35 = gtk_label_new (_("Syntax check program:"));
  gtk_widget_show (preferences_dialog.label35);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox17), preferences_dialog.label35, FALSE, FALSE, 8);
  
  preferences_dialog.php_file_entry = gtk_entry_new();
  gtk_widget_show (preferences_dialog.php_file_entry);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox17), preferences_dialog.php_file_entry, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(preferences_dialog.php_file_entry), get_preferences_manager_php_binary_location(main_window.prefmg));
  g_signal_connect(G_OBJECT(preferences_dialog.php_file_entry),
                       "changed",
                       G_CALLBACK(on_php_binary_location_changed),
                       NULL);

  preferences_dialog.hbox18 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox18);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox7), preferences_dialog.hbox18, FALSE, TRUE, 8);
  
  preferences_dialog.label36 = gtk_label_new (_("Recognised PHP file extensions:"));
  gtk_widget_show (preferences_dialog.label36);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox18), preferences_dialog.label36, FALSE, FALSE, 8);
  
  preferences_dialog.file_extensions = gtk_entry_new ();
  gtk_widget_show (preferences_dialog.file_extensions);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox18), preferences_dialog.file_extensions, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(preferences_dialog.file_extensions), get_preferences_manager_php_file_extensions(main_window.prefmg));
  g_signal_connect(G_OBJECT(preferences_dialog.file_extensions),
                    "changed", G_CALLBACK(on_php_file_extensions_changed),NULL);

  preferences_dialog.hbox26 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox26);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox7), preferences_dialog.hbox26, FALSE, TRUE, 8);
  
  preferences_dialog.label45 = gtk_label_new (_("Shared library path(s):"));
  gtk_widget_show (preferences_dialog.label45);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox26), preferences_dialog.label45, FALSE, FALSE, 8);
  
  preferences_dialog.shared_source = gtk_entry_new ();
  gtk_widget_show (preferences_dialog.shared_source);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox26), preferences_dialog.shared_source, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(preferences_dialog.shared_source), get_preferences_manager_shared_source_location(main_window.prefmg));
  g_signal_connect(G_OBJECT(preferences_dialog.shared_source),
                   "changed", G_CALLBACK(on_shared_source_changed),NULL);

  preferences_dialog.hbox19 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox19);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox7), preferences_dialog.hbox19, FALSE, TRUE, 8);
  
  preferences_dialog.label37 = gtk_label_new (_("Delay (calltip/function list):"));
  gtk_widget_show (preferences_dialog.label37);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox19), preferences_dialog.label37, FALSE, FALSE, 8);
          
  preferences_dialog.delay = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (get_preferences_manager_calltip_delay(main_window.prefmg), 0, 2500, 0, 0, 0)));
  gtk_widget_show (preferences_dialog.delay);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox19), preferences_dialog.delay, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (GTK_HSCALE (preferences_dialog.delay)), "value_changed",
                    G_CALLBACK (on_calltip_delay_changed), NULL);
  
  preferences_dialog.hbox20 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox20);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox7), preferences_dialog.hbox20, TRUE, TRUE, 8);
  
  preferences_dialog.label38 = gtk_label_new (_("Templates:"));
  gtk_widget_show (preferences_dialog.label38);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox20), preferences_dialog.label38, FALSE, FALSE, 8);
  
  preferences_dialog.vbox8 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (preferences_dialog.vbox8);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox20), preferences_dialog.vbox8, TRUE, TRUE, 0);
  
  preferences_dialog.hbox21 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (preferences_dialog.hbox21);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox8), preferences_dialog.hbox21, TRUE, TRUE, 0);
  
  preferences_dialog.scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (preferences_dialog.scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox21), preferences_dialog.scrolledwindow1, TRUE, TRUE, 0);
  
  preferences_dialog.template_store = gtk_list_store_new (1, G_TYPE_STRING);
  add_templates_to_store();
  
  preferences_dialog.Templates = gtk_tree_view_new_with_model (GTK_TREE_MODEL(preferences_dialog.template_store));
  // g_object_unref (preferences_dialog.template_store);
  // Can't unref it because I need to add to it later....
  templates_treeview_add_column();
  
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (preferences_dialog.Templates),0);
  gtk_widget_show (preferences_dialog.Templates);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.scrolledwindow1), preferences_dialog.Templates);

  preferences_dialog.template_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (preferences_dialog.Templates));
  gtk_tree_selection_set_mode (preferences_dialog.template_selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (preferences_dialog.template_selection), "changed",
                    G_CALLBACK (template_row_activated), NULL);

  preferences_dialog.vbox9 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (preferences_dialog.vbox9);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.hbox21), preferences_dialog.vbox9, FALSE, TRUE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.vbox9), 8);
  
  preferences_dialog.add_template_button = gtk_button_new_with_mnemonic (_("Add..."));
  gtk_widget_show (preferences_dialog.add_template_button);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox9), preferences_dialog.add_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.add_template_button), 2);
  g_signal_connect (G_OBJECT (preferences_dialog.add_template_button), "clicked", G_CALLBACK (add_template_clicked), NULL);
  
  preferences_dialog.edit_template_button = gtk_button_new_with_mnemonic (_("Edit..."));
  gtk_widget_show (preferences_dialog.edit_template_button);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox9), preferences_dialog.edit_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.edit_template_button), 2);
  g_signal_connect (G_OBJECT (preferences_dialog.edit_template_button), "clicked", G_CALLBACK (edit_template_clicked), NULL);
  
  preferences_dialog.delete_template_button = gtk_button_new_with_mnemonic (_("Delete"));
  gtk_widget_show (preferences_dialog.delete_template_button);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox9), preferences_dialog.delete_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (preferences_dialog.delete_template_button), 2);
  g_signal_connect (G_OBJECT (preferences_dialog.delete_template_button), "clicked", G_CALLBACK (delete_template_clicked), NULL);
  
  preferences_dialog.template_sample_scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (preferences_dialog.template_sample_scrolled);
  gtk_box_pack_start (GTK_BOX (preferences_dialog.vbox8), preferences_dialog.template_sample_scrolled, FALSE, TRUE, 0);
  
  preferences_dialog.template_sample = gtk_text_view_new ();
  gtk_widget_show (preferences_dialog.template_sample);
  gtk_container_add (GTK_CONTAINER (preferences_dialog.template_sample_scrolled), preferences_dialog.template_sample);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(preferences_dialog.template_sample), FALSE);
 
  preferences_dialog.label31 = gtk_label_new (_("PHP Coding"));
  gtk_widget_show (preferences_dialog.label31);

  gtk_notebook_append_page (GTK_NOTEBOOK (preferences_dialog.notebook1),preferences_dialog.vbox7, preferences_dialog.label31);
 
  preferences_dialog.apply_button = gtk_button_new_with_mnemonic (_("Apply"));
  gtk_widget_show (preferences_dialog.apply_button);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_action_area(GTK_DIALOG(preferences_dialog.window))),preferences_dialog.apply_button);
  g_signal_connect (G_OBJECT (preferences_dialog.apply_button),
                      "clicked", G_CALLBACK (apply_preferences), NULL);
  g_signal_connect (G_OBJECT (preferences_dialog.window),
                      "response", G_CALLBACK (response_preferences), NULL);
  
  get_current_highlighting_settings(current_highlighting_element);
}
