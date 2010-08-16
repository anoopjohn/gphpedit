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
#include <stdlib.h>

#include "main_window.h"
#include "tab_util.h"
#include "debug.h"
#include "templates.h"
#include "edit_template.h"

#include "preferences_dialog.h"

#define PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_PREFERENCES_DIALOG,              \
						PreferencesDialogPrivate))

struct _PreferencesDialogPrivate 
{
  GList *highlighting_elements;
  gboolean changing_highlight_element;
  gchar *current_key;

  GtkWidget *diagbox;
  GtkWidget *save_session;
  GtkWidget *single_instance_only;
  GtkWidget *edge_mode;
  GtkWidget *edge_colour;
  GtkWidget *edge_column;
  GtkWidget *folderbrowser;
  GtkWidget *autobrace;
  GtkWidget *delay;
  GtkWidget *tab_size;
  GtkWidget *use_tabs_instead_spaces;
  GtkWidget *show_indentation_guides;
  GtkWidget *line_wrapping;
  GtkWidget *higthlightcaretline;
  GtkWidget *sel_back;
  GtkWidget *caretline_color;
  GtkWidget *element_combo;
  GtkWidget *font_combo;
  GtkWidget *size_combo;
  Document *highlighting_document;
  GtkWidget *code_sample;
  GtkWidget *bold_button;
  GtkWidget *italic_button;
  GtkWidget *foreground_colour;
  GtkWidget *background_colour;
  GtkWidget *shared_source;
  GtkWidget *file_extensions;
  GtkWidget *php_file_entry;
  GtkWidget *add_template_button;
  GtkWidget *edit_template_button;
  GtkWidget *delete_template_button;
  GtkWidget *template_sample;
  GtkListStore *template_store;
  GtkTreeSelection *template_selection;
  GtkWidget *Templates;

  GtkWidget *close_button;
  GtkWidget *accept_button;
  GtkWidget *apply_button;
};

G_DEFINE_TYPE(PreferencesDialog, PREFERENCES_DIALOG, GTK_TYPE_DIALOG)

static void
PREFERENCES_DIALOG_class_init (PreferencesDialogClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);

	g_type_class_add_private (klass, sizeof (PreferencesDialogPrivate));
}

void preferences_dialog_process_response (GtkDialog *dialog, gint response_id, gpointer   user_data)
{
 if (response_id==GTK_RESPONSE_DELETE_EVENT || response_id==GTK_RESPONSE_CANCEL){
  preferences_manager_restore_data(main_window.prefmg);
  document_manager_refresh_properties_all(main_window.docmg);
  gtk_widget_destroy(GTK_WIDGET(dialog));
 } else if (response_id==GTK_RESPONSE_APPLY){
  document_manager_refresh_properties_all(main_window.docmg);
 } else if (response_id==GTK_RESPONSE_OK){
  document_manager_refresh_properties_all(main_window.docmg);
  // Save the preferences definitely
  preferences_manager_save_data_full(main_window.prefmg);
 }
}

void get_current_preferences(PreferencesDialogPrivate *priv)
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

  priv->highlighting_elements = highlighting_list;
}

static void add_to_template_list(gpointer key, gpointer value, gpointer user_data)
{
  GtkTreeIter iter;
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;

  gtk_list_store_append (priv->template_store, &iter);
  gtk_list_store_set (priv->template_store, &iter,0, (gchar *)key, -1);
}

void add_templates_to_store(PreferencesDialogPrivate *priv)
{
  g_hash_table_foreach(templates, add_to_template_list, priv);
}

void update_template_display(PreferencesDialogPrivate *priv, gchar *template)
{
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->template_sample));

  gtk_text_buffer_set_text (buffer, template, -1);  
}

void template_row_activated(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeModel *model;
  gchar *content, *template;
  GtkTreeIter iter;
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) data;
  if (priv->current_key) {
    g_free(priv->current_key);
  }
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, 0, &priv->current_key, -1);

    // display template content
    template = template_find(priv->current_key);
    if (template) {
      content = template_convert_to_display(template);
      update_template_display(data, content);
      g_free(content);
    }
  }
}

static void templates_treeview_add_column(PreferencesDialogPrivate *priv)
{  
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* column for description */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name/Shortcut"), renderer, "text", 0, NULL);
  gtk_tree_view_column_set_sort_column_id (column, 0);
  gtk_tree_view_append_column (GTK_TREE_VIEW(priv->Templates), column);
}

void add_template_clicked(GtkButton *button, gpointer data)
{
  gchar *name;
  gchar *template;
  GtkTreeIter iter;
  GtkTextBuffer *buffer;
  GtkTextIter begin, end;
  gchar *content;
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) data;
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
    template_delete(priv->current_key); // Just in case you accidentally type the name of an existing template
    template_replace(name, template);
    
    // add to treeview
    gtk_list_store_append (priv->template_store, &iter);
    gtk_list_store_set (priv->template_store, &iter,0, (gchar *)name, -1);
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
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) data;

  // create dialog
  create_edit_template_dialog();

  // fill in existing content
  gtk_entry_set_text(GTK_ENTRY(edit_template_dialog.entry1), priv->current_key);
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->template_sample));
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
    template_delete(priv->current_key);
    template_replace(name, template);
    
    // replace in treeview
    gtk_tree_selection_get_selected (priv->template_selection, NULL, &iter);
    gtk_list_store_set (priv->template_store, &iter,0, (gchar *)name, -1);
    
    // replace in display
    update_template_display(priv, content);
  }
  
  // destroy/null dialog
  gtk_widget_destroy(edit_template_dialog.window1);
  edit_template_dialog.window1 = NULL;
}

void delete_template_clicked(GtkButton *button, gpointer data)
{
  GtkTreeIter iter;
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) data;

  gchar *message = g_strdup_printf(_("Are you sure you want to delete template %s?"),priv->current_key);
  // confirm deletion with dialog
  if (yes_no_dialog (_("gPHPEdit"), message) == GTK_RESPONSE_YES) {
    // delete from templates
    template_delete(priv->current_key);

    // delete from treeview
    gtk_tree_selection_get_selected (priv->template_selection, NULL, &iter);
    gtk_list_store_remove (priv->template_store, &iter);
    priv->current_key = NULL;
  }
   g_free(message);
}

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

void set_controls_to_highlight(PreferencesDialogPrivate *priv, gchar *setting_name, gchar *fontname, gint fontsize, gboolean bold, gboolean italic, gint fore, gint back)
{
  gchar *sfontsize;
  gint row;
  
  // Debug print for preferences being set
  gphpedit_debug_message(DEBUG_PREFS, "Getting %s: %s %d %d %d %d %d", setting_name, fontname, fontsize, bold, italic, fore, back);
  
  priv->changing_highlight_element=TRUE;
  
  fontname++; // Ignore the initial ! for Pango rendering
  
  row = GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (priv->font_combo), g_quark_from_string (fontname)));
  gtk_combo_box_set_active (GTK_COMBO_BOX(priv->font_combo), row);
  sfontsize = g_strdup_printf("%d", fontsize);

  row = GPOINTER_TO_INT (g_object_get_qdata (G_OBJECT (priv->size_combo), g_quark_from_string (sfontsize)));
  gtk_combo_box_set_active (GTK_COMBO_BOX(priv->size_combo), row);
  g_free(sfontsize);
  
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->bold_button), bold);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->italic_button), italic);

  GdkColor color;

  color.red = (fore & 0xff) << 8;
  color.green = ((fore & 0xff00) >> 8) << 8;
  color.blue = ((fore & 0xff0000) >> 16) << 8;

  gtk_color_button_set_color (GTK_COLOR_BUTTON(priv->foreground_colour), &color);

  color.red = (back & 0xff) << 8;
  color.green = ((back & 0xff00) >> 8) << 8;
  color.blue = ((back & 0xff0000) >> 16) << 8;
  gtk_color_button_set_color (GTK_COLOR_BUTTON(priv->background_colour), &color);

  priv->changing_highlight_element=FALSE;
}

#define IS_FONT_NAME(name1, name2) g_str_has_prefix(name1, name2)

void get_current_highlighting_settings(PreferencesDialogPrivate *priv)
{
  gchar *current_highlighting_element = gtk_combo_box_get_active_text (GTK_COMBO_BOX(priv->element_combo));
  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  if (IS_FONT_NAME(current_highlighting_element, _("Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "default_style", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("Line Number (margin)"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "line_numbers", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("Line Number (margin)"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_tag", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Tag"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_tag_unknown", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Unknown Tag"), font, size, bold, italic, fore, back);
   }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_attribute", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Attribute"), font, size, bold, italic, fore, back);
 }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_attribute_unknown", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Unknown Attribute"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_single_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Single-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_double_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Double-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_entity", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Entity"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_script", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Script"), font, size, bold, italic, fore, back); 
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_question", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Question"), font, size, bold, italic, fore, back);
 }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "html_value", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("HTML Value"), font, size, bold, italic, fore, back);  
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Multiple-line Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_line", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Single-line Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_comment_doc", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Document Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_keyword", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Keyword"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_singlestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Single-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_doublestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Double-quoted String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "javascript_symbols", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("JavaScript Symbol"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_hstring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP 'HString'"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_simplestring", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Simple String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "php_variable", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Variable"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "php_comment_line", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Single-line Comment"), font, size, bold, italic, fore, back);  
}
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "php_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("PHP Multiple-line Comment"), font, size, bold, italic, fore, back);  
}
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_tag", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS_Tag"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_class", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Class"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Pseudoclass"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_pseudoclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Pseudoclass"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_pseudoclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Unknown Pseudoclass"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_unknown_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Unknown Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_value", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Value"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_id", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Id"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
   get_preferences_manager_style_settings(main_window.prefmg, "css_important", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Important"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "css_directive", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("CSS Directive"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "sql_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("SQL Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Default"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_default", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Default"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C String"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_string", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C String"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Character"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_character", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Character"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Word"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_word", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Word"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Number"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_number", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Number"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Identifier"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_identifier", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Identifier"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Comment"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_comment", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Comment"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Commentline"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_commentline", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Commentline"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Preprocesor"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_preprocesor", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Preprocesor"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Operator"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_operator", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Operator"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Regex"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_regex", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Regex"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C UUID"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_uuid", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C UUID"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Verbatim"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_verbatim", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Verbatim"), font, size, bold, italic, fore, back);
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Globalclass"))) {
    get_preferences_manager_style_settings(main_window.prefmg, "c_globalclass", &font , &size, &fore, &back, &italic, &bold);
    set_controls_to_highlight(priv, _("C Globalclass"), font, size, bold, italic, fore, back);
  }
  g_free(current_highlighting_element);
}

void on_element_entry_changed(GtkComboBox *widget, gpointer user_data)
{
  get_current_highlighting_settings(user_data);
}

void get_control_values_to_highlight(PreferencesDialogPrivate *priv, gchar *setting_name)
{
  GString *tempfontname;
  GString *message;
  gchar *fontname;
  gboolean bold, italic;
  gint fore, back, fontsize;

  gint newfontsize;
  gint result;

  get_preferences_manager_style_settings(main_window.prefmg, setting_name, &fontname , &fontsize, NULL, NULL, NULL, NULL);

  if (priv->changing_highlight_element) return;
  gchar *combofontname = gtk_combo_box_get_active_text(GTK_COMBO_BOX(priv->font_combo));
  tempfontname = g_string_new(combofontname);
  tempfontname = g_string_prepend(tempfontname, "!");

  if (!g_str_has_prefix(fontname, tempfontname->str)) {
    message = g_string_new(NULL);
    g_string_printf(message, _("You have just changed the font to %s\n\nWould you like to use this font as the default for every element?"), combofontname);
    result = yes_no_dialog (_("gPHPEdit"), message->str);
    gphpedit_debug_message(DEBUG_PREFS, "change_font_global: %d, YES=%d, NO=%d", result, GTK_RESPONSE_YES, GTK_RESPONSE_NO);
    if (result==GTK_RESPONSE_YES) {
    set_preferences_manager_style_settings_font_to_all (main_window.prefmg, tempfontname->str);
    }
    g_string_free(message, TRUE);
    set_document_to_php(priv->highlighting_document);
  }
  g_free(combofontname);
  newfontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(priv->size_combo)));
  if (fontsize != newfontsize && newfontsize != 0) {
    message = g_string_new(NULL);
    g_string_printf(message, _("You have just changed the font size to %dpt\n\nWould you like to use this font size as the default for every element?"), newfontsize);
    result = yes_no_dialog (_("gPHPEdit"), message->str);
    gphpedit_debug_message(DEBUG_PREFS,"change_size_global: %d", result);
    if (result==GTK_RESPONSE_YES) {
    set_preferences_manager_style_settings_size_to_all (main_window.prefmg, newfontsize);
    }
    g_string_free(message, TRUE);
    fontsize = atoi(gtk_combo_box_get_active_text(GTK_COMBO_BOX(priv->size_combo)));
    set_document_to_php(priv->highlighting_document);
  }
  bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->bold_button));
  italic = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->italic_button));
  GdkColor color;
  gtk_color_button_get_color (GTK_COLOR_BUTTON(priv->foreground_colour),&color);
  fore = scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8));

  gtk_color_button_get_color (GTK_COLOR_BUTTON(priv->background_colour),&color);
  back = scintilla_color(color.red >> 8, (color.green >> 8), (color.blue >> 8));
  // Debug print for preferences being set
  gphpedit_debug_message(DEBUG_PREFS,"Setting %s: %s %d %d %d %d %d\n", setting_name, fontname, fontsize, bold, italic, fore, back);  
  set_preferences_manager_style_settings(main_window.prefmg, setting_name, tempfontname->str , &newfontsize, &fore, &back, &italic, &bold);
  g_string_free(tempfontname, FALSE);
}

void set_current_highlighting_font(PreferencesDialogPrivate *priv)
{
  gchar *current_highlighting_element = gtk_combo_box_get_active_text (GTK_COMBO_BOX(priv->element_combo));
  if (IS_FONT_NAME(current_highlighting_element, _("Default"))) {
    get_control_values_to_highlight(priv, "default_style");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("Line Number (margin)"))) {
    get_control_values_to_highlight(priv, "line_numbers");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Tag"))) {
    get_control_values_to_highlight(priv, "html_tag");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Tag"))) {
    get_control_values_to_highlight(priv, "html_tag_unknown");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Attribute"))) {
    get_control_values_to_highlight(priv, "html_attribute");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Unknown Attribute"))) {
    get_control_values_to_highlight(priv, "html_attribute_unknown");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Number"))) {
    get_control_values_to_highlight(priv, "html_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Single-quoted String"))) {
    get_control_values_to_highlight(priv, "html_single_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Double-quoted String"))) {
    get_control_values_to_highlight(priv, "html_double_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Comment"))) {
    get_control_values_to_highlight(priv, "html_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Entity"))) {
    get_control_values_to_highlight(priv, "html_entity");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Script"))) {
    get_control_values_to_highlight(priv, "html_script");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Question"))) {
    get_control_values_to_highlight(priv, "html_question");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("HTML Value"))) {
    get_control_values_to_highlight(priv, "html_value");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Multiple-line Comment"))) {
    get_control_values_to_highlight(priv, "javascript_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-line Comment"))) {
    get_control_values_to_highlight(priv, "javascript_comment_line");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Document Comment"))) {
    get_control_values_to_highlight(priv, "javascript_comment_doc");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Word"))) {
    get_control_values_to_highlight(priv, "javascript_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Keyword"))) {
    get_control_values_to_highlight(priv, "javascript_keyword");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Single-quoted String"))) {
    get_control_values_to_highlight(priv, "javascript_singlestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Double-quoted String"))) {
    get_control_values_to_highlight(priv, "javascript_doublestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("JavaScript Symbol"))) {
    get_control_values_to_highlight(priv, "javascript_symbols");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Default"))) {
    get_control_values_to_highlight(priv, "php_default");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP 'HString'"))) {
    get_control_values_to_highlight(priv, "php_hstring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Simple String"))) {
    get_control_values_to_highlight(priv, "php_simplestring");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Word"))) {
    get_control_values_to_highlight(priv, "php_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Number"))) {
    get_control_values_to_highlight(priv, "php_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Variable"))) {
    get_control_values_to_highlight(priv, "php_variable");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Single-line Comment"))) {
    get_control_values_to_highlight(priv, "php_comment_line");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("PHP Multiple-line Comment"))) {
    get_control_values_to_highlight(priv, "php_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Tag"))) {
    get_control_values_to_highlight(priv, "css_tag");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Class"))) {
    get_control_values_to_highlight(priv, "css_class");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Psuedoclass"))) {
    get_control_values_to_highlight(priv, "css_pseudoclass");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Pseudoclass"))) {
    get_control_values_to_highlight(priv, "css_unknown_pseudoclass");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Operator"))) {
    get_control_values_to_highlight(priv, "css_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Identifier"))) {
    get_control_values_to_highlight(priv, "css_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Unknown Identifier"))) {
    get_control_values_to_highlight(priv, "css_unknown_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Value"))) {
    get_control_values_to_highlight(priv, "css_value");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Comment"))) {
    get_control_values_to_highlight(priv, "css_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS ID"))) {
    get_control_values_to_highlight(priv, "css_id");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Important"))) {
    get_control_values_to_highlight(priv, "css_important");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("CSS Directive"))) {
    get_control_values_to_highlight(priv, "css_directive");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Word"))) {
    get_control_values_to_highlight(priv, "sql_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Identifier"))) {
    get_control_values_to_highlight(priv, "sql_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Number"))) {
    get_control_values_to_highlight(priv, "sql_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL String"))) {
    get_control_values_to_highlight(priv, "sql_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Operator"))) {
    get_control_values_to_highlight(priv, "sql_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("SQL Comment"))) {
    get_control_values_to_highlight(priv, "sql_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Default"))) {
    get_control_values_to_highlight(priv, "c_default");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C String"))) {
    get_control_values_to_highlight(priv, "c_string");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Character"))) {
    get_control_values_to_highlight(priv, "c_character");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Word"))) {
    get_control_values_to_highlight(priv, "c_word");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Number"))) {
    get_control_values_to_highlight(priv, "c_number");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Identifier"))) {
    get_control_values_to_highlight(priv, "c_identifier");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Comment"))) {
    get_control_values_to_highlight(priv, "c_comment");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Commentline"))) {
    get_control_values_to_highlight(priv, "c_commentline");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Preprocesor"))) {
    get_control_values_to_highlight(priv, "c_preprocesor");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Operator"))) {
    get_control_values_to_highlight(priv, "c_operator");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Regex"))) {
    get_control_values_to_highlight(priv, "c_regex");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C UUID"))) {
    get_control_values_to_highlight(priv, "c_uuid");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Verbatim"))) {
    get_control_values_to_highlight(priv, "c_verbatim");
  }
  if (IS_FONT_NAME(current_highlighting_element, _("C Globalclass"))) {
    get_control_values_to_highlight(priv, "c_globalclass");
  }
  tab_check_php_file(priv->highlighting_document);
  g_free(current_highlighting_element);
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

void on_calltip_delay_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  set_preferences_manager_calltip_delay(main_window.prefmg, gtk_spin_button_get_value_as_int(spinbutton));
  set_preferences_manager_auto_complete_delay(main_window.prefmg, gtk_spin_button_get_value_as_int(spinbutton));
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

void on_bold_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_current_highlighting_font(user_data);
}

void on_italic_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_current_highlighting_font(user_data);
}

void on_fontname_entry_changed(GtkEntry *Entry, gpointer user_data)
{
  set_current_highlighting_font(user_data);
}

void on_fontsize_entry_changed(GtkEntry *Entry, gpointer user_data)
{
  set_current_highlighting_font(user_data);
}

void on_fore_changed(GtkColorButton *widget, gpointer user_data)
{
  set_current_highlighting_font(user_data);
}

void on_back_changed(GtkColorButton *widget, gpointer user_data)
{
  set_current_highlighting_font(user_data);
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

#define CANT_SIZES 25
/* These are what we use as the standard font sizes, for the size list. */
static const gchar *font_sizes[CANT_SIZES] = {
  "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16", "17", "18", "20", "22", "24", "26", "28",
  "32", "36", "40", "48", "56", "64", "72"};

gchar sample_text[]= "<?php\n\n/* A class to implement a car\n   by Mr Somebody */\n\nclass Car extends Vehicle\n  implements EAccident\n{\n  private $PetrolTankFull = true;\n  protected $Name = \"betty\";\n  public $YearMade = 1999;\n\n  function __construct()\n  {\n    parent::__construct();\n    print \"Made $Name!\";\n  }\n  \n  private function go()\n  {\n    // Just go for it!\n    print 'Go!!!';\n  }\n}\n\n?>\n\n<html>\n <head>\n  <title>My test page</title>\n </head>\n\n <body>\n  <h1 class='winter'>Test</h1>\n </body>\n</html>  \n";

static void
PREFERENCES_DIALOG_init (PreferencesDialog *dialog)
{
  PreferencesDialogPrivate *priv = PREFERENCES_DIALOG_GET_PRIVATE(dialog);
  priv->diagbox = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
  gtk_box_set_spacing (GTK_BOX(priv->diagbox), 5);
  priv->current_key=NULL;
  get_current_preferences(priv);

  GtkWidget *notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), notebook, FALSE, FALSE, 0);
  
  GtkWidget *prinbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (prinbox);
  gtk_container_set_border_width (GTK_CONTAINER (prinbox), 8);

  /* current session */  
  GtkWidget *adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinbox), adj, FALSE, FALSE, 6);

  GtkWidget *sessionlabel = gtk_label_new(_("<b>Session:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(sessionlabel), TRUE);
  gtk_label_set_justify(GTK_LABEL(sessionlabel), GTK_JUSTIFY_LEFT);
  gtk_widget_show(sessionlabel);
  gtk_container_add (GTK_CONTAINER (adj), sessionlabel);

  priv->save_session = gtk_check_button_new_with_mnemonic (_("Save session (including open files) on exit"));
  gtk_widget_show (priv->save_session);
  gtk_box_pack_start (GTK_BOX (prinbox), priv->save_session, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->save_session), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->save_session), get_preferences_manager_saved_session(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->save_session)), "toggled", G_CALLBACK(on_save_session_toggle), NULL);

  priv->single_instance_only = gtk_check_button_new_with_mnemonic (_("Only ever run 1 copy of gPHPEdit at a time"));
  gtk_widget_show (priv->single_instance_only);
  gtk_box_pack_start (GTK_BOX (prinbox), priv->single_instance_only, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->single_instance_only), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->single_instance_only), get_preferences_manager_single_instance_only(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->single_instance_only)), "toggled", G_CALLBACK(on_single_instance_only_toggle), NULL);
  /*end session*/

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinbox), adj, FALSE, FALSE, 6);

  GtkWidget *label = gtk_label_new(_("<b>Hand edge guide:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  priv->edge_mode = gtk_check_button_new_with_mnemonic (_("Show right hand edge guide"));
  gtk_widget_show (priv->edge_mode);
  gtk_box_pack_start(GTK_BOX(prinbox), priv->edge_mode, FALSE, FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (priv->edge_mode), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->edge_mode), get_preferences_manager_edge_mode(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->edge_mode)), "toggled", G_CALLBACK(on_edge_mode_toggle), NULL);

  /*Begin: Right Hand Edge Color*/
  GtkWidget *hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_box_pack_start(GTK_BOX(prinbox), hbox15, FALSE, FALSE, 6);
  
  label = gtk_label_new (_("Right hand edge colour:"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox15), label, FALSE, FALSE, 8);

  GdkColor color;
  color.red = (get_preferences_manager_edge_colour(main_window.prefmg) & 0xff) << 8;
  color.green = ((get_preferences_manager_edge_colour(main_window.prefmg) & 0xff00) >> 8) << 8;
  color.blue = ((get_preferences_manager_edge_colour(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  priv->edge_colour = gtk_color_button_new_with_color (&color);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(priv->edge_colour), &color);

  gtk_widget_show (priv->edge_colour);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->edge_colour, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(priv->edge_colour)), "color-set", G_CALLBACK(on_edge_colour_changed), NULL);
  /*End: Right Hand Edge Color*/

/*edge part*/

  hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_container_add (GTK_CONTAINER (prinbox), hbox15);
  
  label = gtk_label_new (_("Right hand edge column"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox15), label, FALSE, FALSE, 8);
  
  priv->edge_column = gtk_spin_button_new_with_range(0, 160, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->edge_column), get_preferences_manager_edge_column(main_window.prefmg));
  gtk_widget_show (priv->edge_column);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->edge_column, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (priv->edge_column), "value_changed", G_CALLBACK (on_edge_column_changed), NULL);

/*end edge part*/
/* side panel part */
  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinbox), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Side Panel:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  priv->folderbrowser = gtk_check_button_new_with_mnemonic (_("Show file browser (need restart)"));
  /* set tooltip */
  gtk_widget_set_tooltip_text (priv->folderbrowser,_("Show/Hide side-panel Folder Browser"));
  gtk_widget_show (priv->folderbrowser);
  gtk_container_add (GTK_CONTAINER (prinbox), priv->folderbrowser);
  gtk_container_set_border_width (GTK_CONTAINER (priv->folderbrowser), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->folderbrowser), get_preferences_manager_show_filebrowser(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->folderbrowser)), "toggled", G_CALLBACK(on_save_folderbrowser_toggle), NULL);
  /*end side panel part*/
/* autocompletion part */
  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_widget_show (adj);
  gtk_box_pack_start(GTK_BOX(prinbox), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Auto-completion:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  priv->autobrace = gtk_check_button_new_with_mnemonic (_("Auto-Completion of Brackets and Quotes"));
  /* set tooltip */
  gtk_widget_set_tooltip_text (priv->autobrace,_("Auto-complete the closing brackets/quotes"));
  gtk_widget_show (priv->autobrace);
  gtk_container_add (GTK_CONTAINER (prinbox), priv->autobrace);
  gtk_container_set_border_width (GTK_CONTAINER (priv->autobrace), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->autobrace), get_preferences_manager_auto_complete_braces(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->autobrace)), "toggled", G_CALLBACK(on_save_autobrace_toggle), NULL);

  hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_container_add (GTK_CONTAINER (prinbox), hbox15);

  GtkWidget *label37 = gtk_label_new (_("Delay (calltip/function list):"));
  gtk_widget_show (label37);
  gtk_box_pack_start (GTK_BOX (hbox15), label37, FALSE, FALSE, 8);
  
  priv->delay = gtk_spin_button_new_with_range(0, 2500, 10);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->delay), get_preferences_manager_calltip_delay(main_window.prefmg));
  gtk_widget_show (priv->delay);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->delay, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (priv->delay), "value_changed", G_CALLBACK (on_calltip_delay_changed), NULL);
/*end autocompletion part*/
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);
  gtk_notebook_prepend_page (GTK_NOTEBOOK (notebook), prinbox, label);
/* end first tab */
/*editor tab */

  GtkWidget *prinboxed = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (prinboxed);
  gtk_container_set_border_width (GTK_CONTAINER (prinboxed), 8);

/* tabs part */
  GtkWidget *edgebox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (edgebox);
  gtk_box_pack_start(GTK_BOX(prinboxed), edgebox, FALSE, FALSE, 6);

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(edgebox), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Tabs:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_box_pack_start (GTK_BOX (prinboxed), hbox15, FALSE, FALSE, 0);

  GtkWidget *label32 = gtk_label_new (_("Tab size:"));
  gtk_widget_show (label32);
  gtk_box_pack_start (GTK_BOX (hbox15), label32, FALSE, FALSE, 8);

  priv->tab_size = gtk_spin_button_new_with_range(1, 16, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->tab_size), get_preferences_manager_tab_size(main_window.prefmg));
  gtk_widget_show (priv->tab_size);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->tab_size, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (priv->tab_size), "value_changed", G_CALLBACK (on_tab_size_changed), NULL);

  priv->use_tabs_instead_spaces = gtk_check_button_new_with_mnemonic (_("Use tabs instead of spaces for indentation"));
  gtk_widget_show (priv->use_tabs_instead_spaces);
  gtk_box_pack_start (GTK_BOX (edgebox), priv->use_tabs_instead_spaces, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->use_tabs_instead_spaces), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_tabs_instead_spaces), get_preferences_manager_use_tabs_instead_spaces(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->use_tabs_instead_spaces)), "toggled", G_CALLBACK(on_use_tabs_instead_spaces_toggle), NULL);

  priv->show_indentation_guides = gtk_check_button_new_with_mnemonic (_("Show indentation guides"));
  gtk_widget_show (priv->show_indentation_guides);
  gtk_box_pack_start (GTK_BOX (edgebox), priv->show_indentation_guides, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->show_indentation_guides), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->show_indentation_guides), get_preferences_manager_show_indentation_guides(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->show_indentation_guides)), "toggled", G_CALLBACK(on_show_indentation_guides_toggle), NULL);

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinboxed), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Line Properties:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_box_pack_start (GTK_BOX (prinboxed), hbox15, FALSE, FALSE, 0);

  priv->line_wrapping = gtk_check_button_new_with_mnemonic (_("Wrap long lines"));
  gtk_widget_show (priv->line_wrapping);
  gtk_container_add (GTK_CONTAINER (hbox15), priv->line_wrapping);
  gtk_container_set_border_width (GTK_CONTAINER (priv->line_wrapping), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->line_wrapping), get_preferences_manager_line_wrapping(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->line_wrapping)), "toggled", G_CALLBACK(on_line_wrapping_toggle), NULL);

  priv->higthlightcaretline = gtk_check_button_new_with_mnemonic (_("Highlight Caret Line"));
  /* set tooltip */
  gtk_widget_show (priv->higthlightcaretline);
  gtk_box_pack_start (GTK_BOX (prinboxed), priv->higthlightcaretline, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->higthlightcaretline), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->higthlightcaretline), get_preferences_manager_higthlight_caret_line(main_window.prefmg));
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->higthlightcaretline)), "toggled", G_CALLBACK(on_save_higthlightcaretline_toggle), NULL);

  GtkWidget *colcaret = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (colcaret);
  gtk_box_pack_start (GTK_BOX (prinboxed), colcaret, FALSE, FALSE, 0);
  
  GtkWidget *lblcol = gtk_label_new (_("Highlight Caret Line colour:"));
  gtk_widget_show (lblcol);
  gtk_box_pack_start (GTK_BOX (colcaret), lblcol, FALSE, FALSE, 8);

  GdkColor caret_back;
  caret_back.red = (get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff) << 8;
  caret_back.green = ((get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff00) >> 8) << 8;
  caret_back.blue = ((get_preferences_manager_higthlight_caret_line_color(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  priv->caretline_color = gtk_color_button_new_with_color (&caret_back);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(priv->caretline_color),&caret_back);
  gtk_widget_show (priv->caretline_color);

  gtk_box_pack_start (GTK_BOX (colcaret), priv->caretline_color, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(priv->caretline_color)), "color-set", G_CALLBACK(on_caretline_back_changed), NULL);
  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinboxed), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Selection:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  GtkWidget *hbox16 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox16);
  gtk_box_pack_start (GTK_BOX (prinboxed), hbox16, FALSE, FALSE, 0);

  /*Begin: Selection Color*/  
  GtkWidget *label46 = gtk_label_new (_("Selection colour:"));
  gtk_widget_show (label46);
  gtk_box_pack_start (GTK_BOX (hbox16), label46, FALSE, FALSE, 8);

  GdkColor sel_back;
  sel_back.red = (get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff) << 8;
  sel_back.green = ((get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff00) >> 8) << 8;
  sel_back.blue = ((get_preferences_manager_set_sel_back(main_window.prefmg) & 0xff0000) >> 16) << 8;  
  priv->sel_back = gtk_color_button_new_with_color (&sel_back);
  gtk_color_button_set_color (GTK_COLOR_BUTTON(priv->sel_back), &sel_back);

  gtk_widget_show (priv->sel_back);
  gtk_box_pack_start (GTK_BOX (hbox16), priv->sel_back, FALSE, FALSE, 0);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(priv->sel_back)), "color-set", G_CALLBACK(on_sel_back_changed), NULL);
  /*End: Selection Color*/

/*end editor page */

  GtkWidget *label33 = gtk_label_new (_("Editor"));
  gtk_widget_show (label33);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), prinboxed, label33);
/* end second tab */
/* color tab*/
/*editor tab */
  GtkWidget *princolor = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (princolor);
  gtk_container_set_border_width (GTK_CONTAINER (princolor), 8);

  GtkWidget *frame1 = gtk_frame_new (_("Element:"));
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (princolor), frame1, TRUE, TRUE, 8);

  GtkWidget *elementbox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (elementbox), 8);
  gtk_widget_show (elementbox);
  gtk_container_add (GTK_CONTAINER (frame1), elementbox);

  priv->element_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (priv->element_combo);
  gtk_container_add (GTK_CONTAINER (elementbox), priv->element_combo);

  GList *walk = NULL;
  for (walk = priv->highlighting_elements; walk != NULL; walk = g_list_next (walk)) {
    g_object_set_qdata (G_OBJECT (priv->element_combo), g_quark_from_string (walk->data),
    GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(priv->element_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX(priv->element_combo),walk->data);
  }
  gtk_combo_box_set_active (GTK_COMBO_BOX(priv->element_combo), 0);
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (priv->element_combo)), "changed", G_CALLBACK (on_element_entry_changed), priv);
  g_list_free (priv->highlighting_elements);

  GtkWidget *frame = gtk_frame_new (_("Font:"));
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (princolor), frame, TRUE, TRUE, 8);

  GtkWidget *fontbox = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (fontbox), 8);
  gtk_widget_show (fontbox);
  gtk_container_add (GTK_CONTAINER (frame), fontbox);

  priv->font_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (priv->font_combo);
  gtk_box_pack_start (GTK_BOX (fontbox), priv->font_combo, TRUE, TRUE, 8);
  
  GList *comboitems = get_font_names();
  GList *items = NULL;
  for (items = g_list_first(comboitems); items != NULL; items = g_list_next(items)) {
    // Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
    g_object_set_qdata (G_OBJECT (priv->font_combo), g_quark_from_string (items->data), 
      GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(priv->font_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->font_combo), items->data);
  }
  g_signal_connect (G_OBJECT (priv->font_combo), "changed", G_CALLBACK (on_fontname_entry_changed), priv);
  g_list_free (comboitems);  

  priv->size_combo = gtk_combo_box_entry_new_text ();
  gtk_widget_show (priv->size_combo);
  gtk_box_pack_start (GTK_BOX (fontbox), priv->size_combo, FALSE, FALSE, 0);
  
  int i;
  for (i=0; i<CANT_SIZES; i++) {
    // Suggested by__tim in #Gtk+/Freenode to be able to find the item again from set_control_to_highlight
    g_object_set_qdata (G_OBJECT (priv->size_combo), g_quark_from_static_string (font_sizes[i]), 
      GINT_TO_POINTER (gtk_tree_model_iter_n_children (gtk_combo_box_get_model (GTK_COMBO_BOX(priv->size_combo)), NULL)));
    gtk_combo_box_append_text (GTK_COMBO_BOX (priv->size_combo), font_sizes[i]);
    gphpedit_debug_message(DEBUG_PREFS, "Appending Font Size: %s, %d\n", font_sizes[i], g_quark_from_static_string(font_sizes[i]));
  }
  g_signal_connect (G_OBJECT (GTK_COMBO_BOX (priv->size_combo)), "changed", G_CALLBACK (on_fontsize_entry_changed), priv);

  priv->highlighting_document = document_new (TAB_FILE, "", 0);
  document_set_scintilla(priv->highlighting_document, gtk_scintilla_new());
  document_add_text(priv->highlighting_document, sample_text);
  document_goto_pos(priv->highlighting_document, 0);
  document_set_readonly(priv->highlighting_document, TRUE, TRUE);
  set_document_to_php(priv->highlighting_document);

  priv->code_sample = document_get_editor_widget (priv->highlighting_document);
  gtk_widget_set_size_request (priv->code_sample, 200, 200);
  gtk_widget_show (priv->code_sample);
  gtk_box_pack_start (GTK_BOX (princolor), priv->code_sample, TRUE, TRUE, 0);

  GtkWidget *hbox22 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox22);
  gtk_box_pack_end (GTK_BOX (princolor), hbox22, FALSE, TRUE, 8);
  
  GtkWidget *frame2 = gtk_frame_new (_("Attributes"));
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (hbox22), frame2, TRUE, TRUE, 8);
  
  GtkWidget *vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox11);
  gtk_container_set_border_width (GTK_CONTAINER (vbox11), 4);
  gtk_container_add (GTK_CONTAINER (frame2), vbox11);
  
  priv->bold_button = gtk_check_button_new_with_mnemonic (_("Bold"));
  gtk_widget_show (priv->bold_button);
  gtk_box_pack_start (GTK_BOX (vbox11), priv->bold_button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(priv->bold_button)), "toggled", G_CALLBACK(on_bold_toggle), priv);
  
  priv->italic_button = gtk_check_button_new_with_mnemonic (_("Italic"));
  gtk_widget_show (priv->italic_button);
  gtk_box_pack_start (GTK_BOX (vbox11), priv->italic_button, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(GTK_TOGGLE_BUTTON(priv->italic_button)), "toggled", G_CALLBACK(on_italic_toggle), priv);
  
  GtkWidget *frame3 = gtk_frame_new (_("Colours"));
  gtk_widget_show (frame3);
  gtk_box_pack_start (GTK_BOX (hbox22), frame3, TRUE, TRUE, 8);
  
  GtkWidget *vbox12 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox12);
  gtk_container_add (GTK_CONTAINER (frame3), vbox12);
  gtk_container_set_border_width (GTK_CONTAINER (vbox12), 8);
  
  GtkWidget *hbox23 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox23);
  gtk_box_pack_start (GTK_BOX (vbox12), hbox23, FALSE, TRUE, 0);
  
  GtkWidget *label42 = gtk_label_new (_("Foreground:"));
  gtk_widget_show (label42);
  gtk_box_pack_start (GTK_BOX (hbox23), label42, FALSE, FALSE, 0);
  
  priv->foreground_colour = gtk_color_button_new();
  gtk_widget_show (priv->foreground_colour);
  gtk_box_pack_start (GTK_BOX (hbox23), priv->foreground_colour, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->foreground_colour), 8);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(priv->foreground_colour)), "color-set", G_CALLBACK(on_fore_changed), priv);

  GtkWidget *hbox24 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox24);
  gtk_box_pack_start (GTK_BOX (vbox12), hbox24, FALSE, TRUE, 0);
  
  GtkWidget *label43 = gtk_label_new (_("Background:"));
  gtk_widget_show (label43);
  gtk_box_pack_start (GTK_BOX (hbox24), label43, FALSE, FALSE, 0);
  
  priv->background_colour = gtk_color_button_new();
  gtk_widget_show (priv->background_colour);
  gtk_box_pack_start (GTK_BOX (hbox24), priv->background_colour, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->background_colour), 8);
  g_signal_connect(G_OBJECT(GTK_COLOR_BUTTON(priv->background_colour)), "color-set", G_CALLBACK(on_fore_changed), priv);

  GtkWidget *label30 = gtk_label_new (_("Colour"));
  gtk_widget_show (label30);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), princolor, label30);

/* end color tab */
/* coding tab */
  GtkWidget *princod = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (princod);
  gtk_container_set_border_width (GTK_CONTAINER (princod), 8);

/*  */
  GtkWidget *codbox = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (codbox);
  gtk_box_pack_start(GTK_BOX(princod), codbox, FALSE, FALSE, 6);

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(codbox), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>PHP Coding:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  GtkWidget *hbox17 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox17);
  gtk_box_pack_start (GTK_BOX (princod), hbox17, FALSE, TRUE, 8);
  
  GtkWidget *label40 = gtk_label_new (_("Syntax check program:"));
  gtk_widget_show (label40);
  gtk_box_pack_start (GTK_BOX (hbox17), label40, FALSE, FALSE, 8);
  
  priv->php_file_entry = gtk_entry_new();
  gtk_widget_show (priv->php_file_entry);
  gtk_box_pack_start (GTK_BOX (hbox17), priv->php_file_entry, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->php_file_entry), get_preferences_manager_php_binary_location(main_window.prefmg));
  g_signal_connect(G_OBJECT(priv->php_file_entry), "changed", G_CALLBACK(on_php_binary_location_changed), NULL);

  GtkWidget *hbox18 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox18);
  gtk_box_pack_start (GTK_BOX (princod), hbox18, FALSE, TRUE, 8);
  
  GtkWidget *label48 = gtk_label_new (_("Recognised PHP file extensions:"));
  gtk_widget_show (label48);
  gtk_box_pack_start (GTK_BOX (hbox18), label48, FALSE, FALSE, 8);
  
  priv->file_extensions = gtk_entry_new ();
  gtk_widget_show (priv->file_extensions);
  gtk_box_pack_start (GTK_BOX (hbox18), priv->file_extensions, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->file_extensions), get_preferences_manager_php_file_extensions(main_window.prefmg));
  g_signal_connect(G_OBJECT(priv->file_extensions), "changed", G_CALLBACK(on_php_file_extensions_changed), NULL);

  GtkWidget *hbox26 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox26);
  gtk_box_pack_start (GTK_BOX (princod), hbox26, FALSE, TRUE, 8);
  
  GtkWidget *label47 = gtk_label_new (_("Shared library path(s):"));
  gtk_widget_show (label47);
  gtk_box_pack_start (GTK_BOX (hbox26), label47, FALSE, FALSE, 8);
  
  priv->shared_source = gtk_entry_new ();
  gtk_widget_show (priv->shared_source);
  gtk_box_pack_start (GTK_BOX (hbox26), priv->shared_source, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->shared_source), get_preferences_manager_shared_source_location(main_window.prefmg));
  g_signal_connect(G_OBJECT(priv->shared_source), "changed", G_CALLBACK(on_shared_source_changed), NULL);

  GtkWidget *hbox20 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox20);
  gtk_box_pack_start (GTK_BOX (princod), hbox20, TRUE, TRUE, 8);

  GtkWidget *label50 = gtk_label_new (_("Templates:"));
  gtk_widget_show (label50);
  gtk_box_pack_start (GTK_BOX (hbox20), label50, FALSE, FALSE, 8);
  
  GtkWidget *vbox8 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox8);
  gtk_box_pack_start (GTK_BOX (hbox20), vbox8, TRUE, TRUE, 0);
  
  GtkWidget *hbox21 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox21);
  gtk_box_pack_start (GTK_BOX (vbox8), hbox21, TRUE, TRUE, 0);
  
  GtkWidget *scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (hbox21), scrolledwindow1, TRUE, TRUE, 0);
  
  priv->template_store = gtk_list_store_new (1, G_TYPE_STRING);
  add_templates_to_store(priv);
  
  priv->Templates = gtk_tree_view_new_with_model (GTK_TREE_MODEL(priv->template_store));
  // g_object_unref (preferences_dialog.template_store);
  // Can't unref it because I need to add to it later....
  templates_treeview_add_column(priv);
  
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (priv->Templates),0);
  gtk_widget_show (priv->Templates);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), priv->Templates);

  priv->template_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->Templates));
  gtk_tree_selection_set_mode (priv->template_selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (priv->template_selection), "changed", G_CALLBACK (template_row_activated), priv);

  GtkWidget *vbox9 = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox9);
  gtk_box_pack_start (GTK_BOX (hbox21), vbox9, FALSE, TRUE, 8);
  gtk_container_set_border_width (GTK_CONTAINER (vbox9), 8);
  
  priv->add_template_button = gtk_button_new_with_mnemonic (_("Add..."));
  gtk_widget_show (priv->add_template_button);
  gtk_box_pack_start (GTK_BOX (vbox9), priv->add_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->add_template_button), 2);
  g_signal_connect (G_OBJECT (priv->add_template_button), "clicked", G_CALLBACK (add_template_clicked), priv);
  
  priv->edit_template_button = gtk_button_new_with_mnemonic (_("Edit..."));
  gtk_widget_show (priv->edit_template_button);
  gtk_box_pack_start (GTK_BOX (vbox9), priv->edit_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->edit_template_button), 2);
  g_signal_connect (G_OBJECT (priv->edit_template_button), "clicked", G_CALLBACK (edit_template_clicked), priv);
  
  priv->delete_template_button = gtk_button_new_with_mnemonic (_("Delete"));
  gtk_widget_show (priv->delete_template_button);
  gtk_box_pack_start (GTK_BOX (vbox9), priv->delete_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->delete_template_button), 2);
  g_signal_connect (G_OBJECT (priv->delete_template_button), "clicked", G_CALLBACK (delete_template_clicked), priv);
  
  GtkWidget *template_sample_scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (template_sample_scrolled);
  gtk_box_pack_start (GTK_BOX (vbox8), template_sample_scrolled, FALSE, TRUE, 0);
  
  priv->template_sample = gtk_text_view_new ();
  gtk_widget_show (priv->template_sample);
  gtk_container_add (GTK_CONTAINER (template_sample_scrolled), priv->template_sample);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(priv->template_sample), FALSE);
/**/
  GtkWidget *label36 = gtk_label_new (_("Coding"));
  gtk_widget_show (label36);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), princod, label36);

/**/
  gtk_widget_show_all (priv->diagbox);

  priv->accept_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
  priv->close_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  priv->apply_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);

  gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(preferences_dialog_process_response), priv);

  get_current_highlighting_settings(priv);
}

GtkWidget *
preferences_dialog_new (GtkWindow *parent)
{
	PreferencesDialog *dialog;

	dialog = g_object_new (GOBJECT_TYPE_PREFERENCES_DIALOG, "has-separator", FALSE, NULL);

	if (parent != NULL)
	{
		gtk_window_set_transient_for (GTK_WINDOW (dialog),
					      parent);
	
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog),
						    TRUE);
	}

  gtk_window_set_position (GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Preferences"));
  gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (dialog), 10);

	return GTK_WIDGET (dialog);
}

