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
#include "plugindialog.h"
#include "templates_editor.h"

#include "preferences_dialog.h"

#define PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_PREFERENCES_DIALOG,              \
						PreferencesDialogPrivate))

struct _PreferencesDialogPrivate 
{
  GtkWidget *diagbox;
  GtkWidget *save_session;
  GtkWidget *single_instance_only;
  GtkWidget *edge_mode;
  GtkWidget *edge_column;
  GtkWidget *folderbrowser;
  GtkWidget *autobrace;
  GtkWidget *delay;
  GtkWidget *tab_size;
  GtkWidget *use_tabs_instead_spaces;
  GtkWidget *show_indentation_guides;
  GtkWidget *line_wrapping;
  GtkWidget *higthlightcaretline;
  GtkWidget *font_settings;
  GtkWidget *size_combo;
  GtkListStore *list_store;
  GtkWidget *code_sample;
  GtkWidget *shared_source;
  GtkWidget *file_extensions;
  GtkWidget *php_file_entry;

  GtkWidget *templates_editor;
  GtkWidget *plugindialog;

  GtkWidget *close_button;
  GtkWidget *accept_button;
  GtkWidget *apply_button;
};

G_DEFINE_TYPE(PreferencesDialog, PREFERENCES_DIALOG, GTK_TYPE_DIALOG)

enum
{
	ID_COLUMN = 0,
	NAME_COLUMN,
	DESC_COLUMN,
	NUM_COLUMNS
};

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
  gtk_widget_destroy(GTK_WIDGET(dialog));
 }
}

void on_tab_size_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  g_object_set(main_window.prefmg,"indentation_size",gtk_spin_button_get_value_as_int(spinbutton), 
    "tab_size",gtk_spin_button_get_value_as_int(spinbutton), NULL);
}

void on_calltip_delay_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  g_object_set(main_window.prefmg,"calltip_delay",gtk_spin_button_get_value_as_int(spinbutton), 
    "autocomplete_delay", gtk_spin_button_get_value_as_int(spinbutton),NULL);
}

void on_edge_column_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  g_object_set(main_window.prefmg, "edge_column", gtk_spin_button_get_value_as_int(spinbutton), NULL);
}

void on_show_indentation_guides_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set(main_window.prefmg, "show_indentation_guides", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_edge_mode_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set(main_window.prefmg, "edge_mode", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_line_wrapping_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set(main_window.prefmg, "line_wrapping", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_use_tabs_instead_spaces_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set(main_window.prefmg, "tabs_instead_spaces", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_session_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
 g_object_set (main_window.prefmg, "save_session", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_folderbrowser_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  set_preferences_manager_show_filebrowser(main_window.prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_save_autobrace_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set (main_window.prefmg, "auto_complete_braces", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_higthlightcaretline_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set (main_window.prefmg, "higthlight_caret_line", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_single_instance_only_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  g_object_set (main_window.prefmg, "single_instance_only", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_php_binary_location_changed (GtkEntry *entry, gpointer user_data)
{
  g_object_set (main_window.prefmg, "php_binary_location", gtk_entry_get_text(entry), NULL);
}

void on_php_file_extensions_changed (GtkEntry *entry, gpointer user_data)
{
  g_object_set (main_window.prefmg, "php_file_extensions", gtk_entry_get_text(entry), NULL);
}

void on_shared_source_changed (GtkEntry *entry, gpointer user_data)
{
  g_object_set (main_window.prefmg, "shared_source_location", gtk_entry_get_text(entry), NULL);
}

void on_font_settings_toggle(GtkFontButton *widget, gpointer user_data){
  set_font_settings (main_window.prefmg, (gchar *)gtk_font_button_get_font_name (widget));
}

static void
scheme_description_cell_data_func (GtkTreeViewColumn *column,
				   GtkCellRenderer   *renderer,
				   GtkTreeModel      *model,
				   GtkTreeIter       *iter,
				   gpointer           data)
{
	gchar *name;
	gchar *desc;
	gchar *text;

	gtk_tree_model_get (model, iter,
			    NAME_COLUMN, &name,
			    DESC_COLUMN, &desc,
			    -1);

	if (desc != NULL)
	{
		text = g_markup_printf_escaped ("<b>%s</b> - %s",
						name,
						desc);
	}
	else
	{
		text = g_markup_printf_escaped ("<b>%s</b>",
						name);
	}

	g_free (name);
	g_free (desc);

	g_object_set (G_OBJECT (renderer),
		      "markup",
		      text,
		      NULL);

	g_free (text);
}

static gint
schemes_compare (gconstpointer a, gconstpointer b)
{
	GtkSourceStyleScheme *scheme_a = (GtkSourceStyleScheme *)a;
	GtkSourceStyleScheme *scheme_b = (GtkSourceStyleScheme *)b;

	const gchar *name_a = gtk_source_style_scheme_get_name (scheme_a);
	const gchar *name_b = gtk_source_style_scheme_get_name (scheme_b);

	return g_utf8_collate (name_a, name_b);
}

GSList *
style_scheme_manager_list_schemes_sorted (GtkSourceStyleSchemeManager *manager)
{
	const gchar * const * scheme_ids;
	GSList *schemes = NULL;

	g_return_val_if_fail (GTK_IS_SOURCE_STYLE_SCHEME_MANAGER (manager), NULL);

	scheme_ids = gtk_source_style_scheme_manager_get_scheme_ids (manager);
	
	while (*scheme_ids != NULL)
	{
		GtkSourceStyleScheme *scheme;

		scheme = gtk_source_style_scheme_manager_get_scheme (manager, 
								     *scheme_ids);

		schemes = g_slist_prepend (schemes, scheme);

		++scheme_ids;
	}

	if (schemes != NULL)
		schemes = g_slist_sort (schemes, (GCompareFunc)schemes_compare);

	return schemes;
}

static const gchar *
populate_color_scheme_list (PreferencesDialog *dlg, const gchar *def_id)
{
  PreferencesDialogPrivate *priv = PREFERENCES_DIALOG_GET_PRIVATE(dlg);
	GSList *schemes;
	GSList *l;
	
	gtk_list_store_clear (priv->list_store);
	
  g_object_get(main_window.prefmg, "style_name", &def_id, NULL);

	if (def_id == NULL) 
	{
		g_warning ("Cannot build the list of available color schemes.\n"
		           "Please check your GtkSourceView installation.");
		return NULL;
	}
	
	schemes = style_scheme_manager_list_schemes_sorted (main_window.stylemg);
	l = schemes;
	while (l != NULL)
	{
		GtkSourceStyleScheme *scheme;
		const gchar *id;
		const gchar *name;
		const gchar *description;
		GtkTreeIter iter;

		scheme = GTK_SOURCE_STYLE_SCHEME (l->data);
				
		id = gtk_source_style_scheme_get_id (scheme);
		name = gtk_source_style_scheme_get_name (scheme);
		description = gtk_source_style_scheme_get_description (scheme);

		gtk_list_store_append (priv->list_store, &iter);
		gtk_list_store_set (priv->list_store,
				    &iter,
				    ID_COLUMN, id,
				    NAME_COLUMN, name,
				    DESC_COLUMN, description,
				    -1);

		g_return_val_if_fail (def_id != NULL, NULL);
		if (strcmp (id, def_id) == 0)
		{
			GtkTreeSelection *selection;
			
			selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->size_combo));
			gtk_tree_selection_select_iter (selection, &iter);
		}
		
		l = g_slist_next (l);
	}
	
	g_slist_free (schemes);
	
	return def_id;
}

static void
style_scheme_changed (GtkWidget *treeview,
		      PreferencesDialog *dlg)
{
	GtkTreePath *path;
	GtkTreeIter iter;
	gchar *id;
  PreferencesDialogPrivate *priv = PREFERENCES_DIALOG_GET_PRIVATE(dlg);

	gtk_tree_view_get_cursor (GTK_TREE_VIEW (priv->size_combo), &path, NULL);
	gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->list_store),
				 &iter, path);
	gtk_tree_path_free (path);
	gtk_tree_model_get (GTK_TREE_MODEL (priv->list_store),
			    &iter, ID_COLUMN, &id, -1);

  g_object_set(main_window.prefmg, "style_name", id, NULL);
  document_manager_refresh_properties_all(main_window.docmg);

	g_free (id);
}

static void
PREFERENCES_DIALOG_init (PreferencesDialog *dialog)
{
  PreferencesDialogPrivate *priv = PREFERENCES_DIALOG_GET_PRIVATE(dialog);
  priv->diagbox = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
  gtk_box_set_spacing (GTK_BOX(priv->diagbox), 2);

  GtkWidget *notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start(GTK_BOX(priv->diagbox), notebook, FALSE, FALSE, 0);
  
  GtkWidget *prinbox = gtk_vbox_new (FALSE, 0);
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

  gboolean save_session, single_instance;
  g_object_get (main_window.prefmg, "save_session", &save_session, "single_instance_only", &single_instance, NULL);


  priv->save_session = gtk_check_button_new_with_mnemonic (_("Save session (including open files) on exit"));
  gtk_widget_show (priv->save_session);
  gtk_box_pack_start (GTK_BOX (prinbox), priv->save_session, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->save_session), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->save_session), save_session);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->save_session)), "toggled", G_CALLBACK(on_save_session_toggle), NULL);

  priv->single_instance_only = gtk_check_button_new_with_mnemonic (_("Only ever run 1 copy of gPHPEdit at a time"));
  gtk_widget_show (priv->single_instance_only);
  gtk_box_pack_start (GTK_BOX (prinbox), priv->single_instance_only, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->single_instance_only), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->single_instance_only), single_instance);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->single_instance_only)), "toggled", G_CALLBACK(on_single_instance_only_toggle), NULL);
  /*end session*/

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(prinbox), adj, FALSE, FALSE, 6);

  GtkWidget *label = gtk_label_new(_("<b>Hand edge guide:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  gboolean edge_mode;
  gint edge_column;
  g_object_get (main_window.prefmg, "edge_mode", &edge_mode, "edge_column",&edge_column, NULL);

  priv->edge_mode = gtk_check_button_new_with_mnemonic (_("Show right hand edge guide"));
  gtk_widget_show (priv->edge_mode);
  gtk_box_pack_start(GTK_BOX(prinbox), priv->edge_mode, FALSE, FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (priv->edge_mode), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->edge_mode), edge_mode);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->edge_mode)), "toggled", G_CALLBACK(on_edge_mode_toggle), NULL);

/*Begin: Right Hand Edge */
  GtkWidget *hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_box_pack_start(GTK_BOX(prinbox), hbox15, FALSE, FALSE, 6);
    
  label = gtk_label_new (_("Right hand edge column"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox15), label, FALSE, FALSE, 8);
  
  priv->edge_column = gtk_spin_button_new_with_range(0, 160, 1);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->edge_column), edge_column);
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
  gboolean auto_brace;
  gint delay;
  g_object_get(main_window.prefmg, "auto_complete_braces", &auto_brace, "calltip_delay", &delay, NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->autobrace), auto_brace);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->autobrace)), "toggled", G_CALLBACK(on_save_autobrace_toggle), NULL);

  hbox15 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox15);
  gtk_container_add (GTK_CONTAINER (prinbox), hbox15);

  GtkWidget *label37 = gtk_label_new (_("Delay (calltip/function list):"));
  gtk_widget_show (label37);
  gtk_box_pack_start (GTK_BOX (hbox15), label37, FALSE, FALSE, 8);
  
  priv->delay = gtk_spin_button_new_with_range(0, 2500, 10);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->delay), delay);
  gtk_widget_show (priv->delay);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->delay, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (priv->delay), "value_changed", G_CALLBACK (on_calltip_delay_changed), NULL);
/*end autocompletion part*/
  label = gtk_label_new (_("General"));
  gtk_widget_show (label);
  gtk_notebook_prepend_page (GTK_NOTEBOOK (notebook), prinbox, label);
/* end first tab */
/*editor tab */

  GtkWidget *prinboxed = gtk_vbox_new (FALSE, 0);
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
  gint tab_size;
  g_object_get(main_window.prefmg, "tab_size", &tab_size, NULL);
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->tab_size), tab_size);
  gtk_widget_show (priv->tab_size);
  gtk_box_pack_start (GTK_BOX (hbox15), priv->tab_size, FALSE, TRUE, 0);
  g_signal_connect (G_OBJECT (priv->tab_size), "value_changed", G_CALLBACK (on_tab_size_changed), NULL);

  gboolean show_indent_guides, higthlight_caret_line, line_wrapping, tabs_spaces;
  g_object_get(main_window.prefmg, "show_indentation_guides", &show_indent_guides, "higthlight_caret_line", &higthlight_caret_line,
    "line_wrapping",&line_wrapping,"tabs_instead_spaces", &tabs_spaces, NULL);

  priv->use_tabs_instead_spaces = gtk_check_button_new_with_mnemonic (_("Use tabs instead of spaces for indentation"));
  gtk_widget_show (priv->use_tabs_instead_spaces);
  gtk_box_pack_start (GTK_BOX (edgebox), priv->use_tabs_instead_spaces, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->use_tabs_instead_spaces), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_tabs_instead_spaces), tabs_spaces);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->use_tabs_instead_spaces)), "toggled", G_CALLBACK(on_use_tabs_instead_spaces_toggle), NULL);

  priv->show_indentation_guides = gtk_check_button_new_with_mnemonic (_("Show indentation guides"));
  gtk_widget_show (priv->show_indentation_guides);
  gtk_box_pack_start (GTK_BOX (edgebox), priv->show_indentation_guides, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->show_indentation_guides), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->show_indentation_guides), show_indent_guides);
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
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->line_wrapping), line_wrapping);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->line_wrapping)), "toggled", G_CALLBACK(on_line_wrapping_toggle), NULL);

  priv->higthlightcaretline = gtk_check_button_new_with_mnemonic (_("Highlight Caret Line"));
  /* set tooltip */
  gtk_widget_show (priv->higthlightcaretline);
  gtk_box_pack_start (GTK_BOX (prinboxed), priv->higthlightcaretline, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->higthlightcaretline), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->higthlightcaretline), higthlight_caret_line);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->higthlightcaretline)), "toggled", G_CALLBACK(on_save_higthlightcaretline_toggle), NULL);


/*  */
  GtkWidget *codbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (codbox);
  gtk_box_pack_start(GTK_BOX(prinboxed), codbox, FALSE, FALSE, 6);

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(codbox), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>PHP Coding:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  GtkWidget *hbox17 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox17);
  gtk_box_pack_start (GTK_BOX (codbox), hbox17, FALSE, TRUE, 8);
  
  GtkWidget *label40 = gtk_label_new (_("Syntax check program:"));
  gtk_widget_show (label40);
  gtk_box_pack_start (GTK_BOX (hbox17), label40, FALSE, FALSE, 8);
  
  const gchar *shared_source_location;
  const gchar *php_binary_location;
  const gchar *php_file_extensions;
  g_object_get(main_window.prefmg, "shared_source_location", &shared_source_location, 
    "php_binary_location", &php_binary_location,"php_file_extensions", &php_file_extensions, NULL);

  priv->php_file_entry = gtk_entry_new();
  gtk_widget_show (priv->php_file_entry);
  gtk_box_pack_start (GTK_BOX (hbox17), priv->php_file_entry, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->php_file_entry), php_binary_location);
  g_signal_connect(G_OBJECT(priv->php_file_entry), "changed", G_CALLBACK(on_php_binary_location_changed), NULL);

  GtkWidget *hbox18 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox18);
  gtk_box_pack_start (GTK_BOX (codbox), hbox18, FALSE, TRUE, 8);
  
  GtkWidget *label48 = gtk_label_new (_("Recognised PHP file extensions:"));
  gtk_widget_show (label48);
  gtk_box_pack_start (GTK_BOX (hbox18), label48, FALSE, FALSE, 8);
  
  priv->file_extensions = gtk_entry_new ();
  gtk_widget_show (priv->file_extensions);
  gtk_box_pack_start (GTK_BOX (hbox18), priv->file_extensions, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->file_extensions), php_file_extensions);
  g_signal_connect(G_OBJECT(priv->file_extensions), "changed", G_CALLBACK(on_php_file_extensions_changed), NULL);

  GtkWidget *hbox26 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox26);
  gtk_box_pack_start (GTK_BOX (codbox), hbox26, FALSE, TRUE, 8);
  
  GtkWidget *label47 = gtk_label_new (_("Shared library path(s):"));
  gtk_widget_show (label47);
  gtk_box_pack_start (GTK_BOX (hbox26), label47, FALSE, FALSE, 8);
  
  priv->shared_source = gtk_entry_new ();
  gtk_widget_show (priv->shared_source);
  gtk_box_pack_start (GTK_BOX (hbox26), priv->shared_source, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(priv->shared_source), shared_source_location);
  g_signal_connect(G_OBJECT(priv->shared_source), "changed", G_CALLBACK(on_shared_source_changed), NULL);

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

  adj= gtk_alignment_new (0.00, 0.50, 0, 0);
  gtk_box_pack_start(GTK_BOX(princolor), adj, FALSE, FALSE, 6);

  label = gtk_label_new(_("<b>Font:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_container_add (GTK_CONTAINER (adj), label);

  GtkWidget *cont = gtk_hbox_new (FALSE, 12);
  gtk_widget_show (cont);
  gtk_box_pack_start(GTK_BOX(princolor), cont, FALSE, FALSE, 6);

  label = gtk_label_new_with_mnemonic(_("Editor _font:"));
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
  gtk_widget_show(label);
  gtk_box_pack_start(GTK_BOX(cont), label, FALSE, FALSE, 6);

  gchar *style_font_name;
  guint size;
  g_object_get(main_window.prefmg, "style_font_name", &style_font_name,"font_size", &size, NULL);
  gchar *font_setting = g_strdup_printf("%s %d", style_font_name + 1, size);
  priv->font_settings = gtk_font_button_new_with_font (font_setting);
  g_free(font_setting);
  gtk_widget_show(priv->font_settings);
  gtk_widget_set_size_request (priv->font_settings, 140, -1);
  gtk_box_pack_start(GTK_BOX(cont), priv->font_settings, FALSE, FALSE, 6);
  g_signal_connect(G_OBJECT(priv->font_settings), "font-set", G_CALLBACK(on_font_settings_toggle), NULL);
  gtk_font_button_set_show_style (GTK_FONT_BUTTON(priv->font_settings), FALSE);

	label = gtk_label_new (_("<b>Color Scheme:</b>"));
  gtk_label_set_use_markup (GTK_LABEL(label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	
  gtk_box_pack_start(GTK_BOX(princolor), label, FALSE, FALSE, 6);
  
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;
	const gchar *def_id;

  priv->size_combo = gtk_tree_view_new ();

	/* Create GtkListStore for styles & setup treeview. */
	priv->list_store = gtk_list_store_new (NUM_COLUMNS,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING,
								G_TYPE_STRING);

	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (priv->list_store), 0, GTK_SORT_ASCENDING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->size_combo), GTK_TREE_MODEL (priv->list_store));
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (priv->size_combo), FALSE);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (priv->size_combo), TRUE);
	column = gtk_tree_view_column_new ();

	renderer = gtk_cell_renderer_text_new ();
	g_object_set (renderer, "ellipsize", PANGO_ELLIPSIZE_END, NULL);
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_cell_data_func (column,
						 renderer,
						 scheme_description_cell_data_func,
						 NULL,
						 NULL);

	gtk_tree_view_append_column (GTK_TREE_VIEW (priv->size_combo),
				     column);

	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->size_combo));
	gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);

	def_id = populate_color_scheme_list (dialog, NULL);
	
  g_signal_connect (priv->size_combo,
			  "cursor-changed",
			  G_CALLBACK (style_scheme_changed),
			  dialog);

  GtkWidget *scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW(scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(scrolledwindow), GTK_SHADOW_ETCHED_IN);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), priv->size_combo);
  gtk_widget_show(scrolledwindow);
  gtk_widget_show(priv->size_combo);
  gtk_box_pack_start(GTK_BOX(princolor), scrolledwindow, TRUE, TRUE, 6);

  GtkWidget *label30 = gtk_label_new (_("Colour"));
  gtk_widget_show (label30);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), princolor, label30);

/* end color tab */
/* coding tab */
  GtkWidget *princod = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (princod);
  gtk_container_set_border_width (GTK_CONTAINER (princod), 8);

  priv->templates_editor = templates_editor_new ();
  gtk_widget_show (priv->templates_editor);
  gtk_box_pack_start (GTK_BOX (princod), priv->templates_editor, TRUE, TRUE, 8);
/**/
  GtkWidget *label36 = gtk_label_new (_("Templates"));
  gtk_widget_show (label36);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), princod, label36);

/**/
/*plugins tab */
  GtkWidget *prinplug = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (prinplug);
  gtk_container_set_border_width (GTK_CONTAINER (prinplug), 8);

/*  */
  priv->plugindialog = plugin_dialog_new();
  gtk_box_pack_start (GTK_BOX (prinplug), priv->plugindialog, TRUE, TRUE, 0);

  label36 = gtk_label_new (_("Plugin"));
  gtk_widget_show (label36);
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), prinplug, label36);
/**/
  gtk_widget_show_all (priv->diagbox);

  priv->accept_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_OK, GTK_RESPONSE_OK);
  priv->close_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
  priv->apply_button = gtk_dialog_add_button (GTK_DIALOG(dialog), GTK_STOCK_APPLY, GTK_RESPONSE_APPLY);

  gtk_dialog_set_default_response (GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);

  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(preferences_dialog_process_response), priv);
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
