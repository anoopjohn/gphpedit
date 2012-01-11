/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 - 2010 Jose Rostagno

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


#include <string.h>
#include <glib/gi18n.h>
#include "main_window.h"
#include "debug.h"
#include "plugindialog.h"
#include "templates_editor.h"
#include "preferences_dialog.h"

#define PREFERENCES_DIALOG_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_PREFERENCES_DIALOG,              \
						PreferencesDialogPrivate))

struct _PreferencesDialogPrivate 
{
  PreferencesManager *prefmg;
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
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
 if (response_id==GTK_RESPONSE_DELETE_EVENT || response_id==GTK_RESPONSE_CANCEL){
  preferences_manager_restore_data(priv->prefmg);
  document_manager_refresh_properties_all(main_window.docmg);
  gtk_widget_destroy(GTK_WIDGET(dialog));
 } else if (response_id==GTK_RESPONSE_APPLY){
  document_manager_refresh_properties_all(main_window.docmg);
 } else if (response_id==GTK_RESPONSE_OK){
  document_manager_refresh_properties_all(main_window.docmg);
  // Save the preferences definitely
  preferences_manager_save_data_full(priv->prefmg);
  gtk_widget_destroy(GTK_WIDGET(dialog));
 }
}

void on_tab_size_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg,"indentation_size",gtk_spin_button_get_value_as_int(spinbutton), 
    "tab_size",gtk_spin_button_get_value_as_int(spinbutton), NULL);
}

void on_calltip_delay_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg,"calltip_delay",gtk_spin_button_get_value_as_int(spinbutton), 
    "autocomplete_delay", gtk_spin_button_get_value_as_int(spinbutton),NULL);
}

void on_edge_column_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg, "edge_column", gtk_spin_button_get_value_as_int(spinbutton), NULL);
}

void on_show_indentation_guides_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg, "show_indentation_guides", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_edge_mode_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg, "edge_mode", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_line_wrapping_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg, "line_wrapping", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_use_tabs_instead_spaces_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set(priv->prefmg, "tabs_instead_spaces", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_session_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
 g_object_set (priv->prefmg, "save_session", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_folderbrowser_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  set_preferences_manager_show_filebrowser(priv->prefmg, gtk_toggle_button_get_active(togglebutton));
}

void on_save_autobrace_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "auto_complete_braces", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_save_higthlightcaretline_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "higthlight_caret_line", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_single_instance_only_toggle(GtkToggleButton *togglebutton, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "single_instance_only", gtk_toggle_button_get_active(togglebutton), NULL);
}

void on_php_binary_location_changed (GtkEntry *entry, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "php_binary_location", gtk_entry_get_text(entry), NULL);
}

void on_php_file_extensions_changed (GtkEntry *entry, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "php_file_extensions", gtk_entry_get_text(entry), NULL);
}

void on_shared_source_changed (GtkEntry *entry, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  g_object_set (priv->prefmg, "shared_source_location", gtk_entry_get_text(entry), NULL);
}

void on_font_settings_toggle(GtkFontButton *widget, gpointer user_data)
{
  PreferencesDialogPrivate *priv = (PreferencesDialogPrivate *) user_data;
  set_font_settings (priv->prefmg, (gchar *)gtk_font_button_get_font_name (widget));
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
	
  g_object_get(priv->prefmg, "style_name", &def_id, NULL);

	if (def_id == NULL) 
	{
		g_warning ("Cannot build the list of available color schemes.\n"
		           "Please check your GtkSourceView installation.");
		return NULL;
	}

    GtkSourceStyleSchemeManager *stylemg = gtk_source_style_scheme_manager_new();
	schemes = style_scheme_manager_list_schemes_sorted (stylemg);
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

    g_object_set(priv->prefmg, "style_name", id, NULL);
    document_manager_refresh_properties_all(main_window.docmg);

	g_free (id);
}

static void
PREFERENCES_DIALOG_init (PreferencesDialog *dialog)
{
  PreferencesDialogPrivate *priv = PREFERENCES_DIALOG_GET_PRIVATE(dialog);
  priv->diagbox = gtk_dialog_get_content_area (GTK_DIALOG(dialog));
  gtk_box_set_spacing (GTK_BOX(priv->diagbox), 2);

  GtkBuilder *builder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (builder, GPHPEDIT_UI_DIR "/preferences_dialog.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }

  priv->prefmg = preferences_manager_new();
  GtkWidget *notebook = GTK_WIDGET(gtk_builder_get_object (builder, "prefnote"));
  gtk_widget_show (notebook);
  gtk_widget_reparent (notebook, priv->diagbox);

  gboolean save_session, single_instance;
  g_object_get (priv->prefmg, "save_session", &save_session, "single_instance_only", &single_instance, NULL);

  priv->save_session = GTK_WIDGET(gtk_builder_get_object (builder, "save_session"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->save_session), save_session);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->save_session)), "toggled", G_CALLBACK(on_save_session_toggle), priv);

  priv->single_instance_only = GTK_WIDGET(gtk_builder_get_object (builder, "single_instance_only"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->single_instance_only), single_instance);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->single_instance_only)), "toggled", G_CALLBACK(on_single_instance_only_toggle), priv);

  gboolean edge_mode;
  gint edge_column;
  g_object_get (priv->prefmg, "edge_mode", &edge_mode, "edge_column",&edge_column, NULL);

  priv->edge_mode = GTK_WIDGET(gtk_builder_get_object (builder, "edge_mode"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->edge_mode), edge_mode);
  g_signal_connect(G_OBJECT(GTK_CHECK_BUTTON(priv->edge_mode)), "toggled", G_CALLBACK(on_edge_mode_toggle), priv);

  priv->edge_column = GTK_WIDGET(gtk_builder_get_object (builder, "edge_column"));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->edge_column), edge_column);
  g_signal_connect (G_OBJECT (priv->edge_column), "value_changed", G_CALLBACK (on_edge_column_changed), priv);

  priv->folderbrowser = GTK_WIDGET(gtk_builder_get_object (builder, "filebrowser"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->folderbrowser), get_preferences_manager_show_filebrowser(priv->prefmg));
  g_signal_connect(GTK_CHECK_BUTTON(priv->folderbrowser), "toggled", G_CALLBACK(on_save_folderbrowser_toggle), priv);

  priv->autobrace = GTK_WIDGET(gtk_builder_get_object (builder, "autobrace"));
  gboolean auto_brace;
  gint delay;
  g_object_get(priv->prefmg, "auto_complete_braces", &auto_brace, "calltip_delay", &delay, NULL);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->autobrace), auto_brace);
  g_signal_connect(GTK_CHECK_BUTTON(priv->autobrace), "toggled", G_CALLBACK(on_save_autobrace_toggle), priv);

  priv->delay = GTK_WIDGET(gtk_builder_get_object (builder, "delay"));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->delay), delay);
  g_signal_connect (G_OBJECT (priv->delay), "value_changed", G_CALLBACK (on_calltip_delay_changed), priv);

  gboolean show_indent_guides, higthlight_caret_line, line_wrapping, tabs_spaces;
  g_object_get(priv->prefmg, "show_indentation_guides", &show_indent_guides, "higthlight_caret_line", &higthlight_caret_line,
    "line_wrapping",&line_wrapping,"tabs_instead_spaces", &tabs_spaces, NULL);

  priv->use_tabs_instead_spaces = GTK_WIDGET(gtk_builder_get_object (builder, "use_tabs_instead_spaces"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->use_tabs_instead_spaces), tabs_spaces);
  g_signal_connect(GTK_CHECK_BUTTON(priv->use_tabs_instead_spaces), "toggled", G_CALLBACK(on_use_tabs_instead_spaces_toggle), priv);

  priv->show_indentation_guides = GTK_WIDGET(gtk_builder_get_object (builder, "show_indentation_guides"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->show_indentation_guides), show_indent_guides);
  g_signal_connect(GTK_CHECK_BUTTON(priv->show_indentation_guides), "toggled", G_CALLBACK(on_show_indentation_guides_toggle), priv);

  gint tab_size;
  g_object_get(priv->prefmg, "tab_size", &tab_size, NULL);

  priv->tab_size = GTK_WIDGET(gtk_builder_get_object (builder, "tab_size"));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON(priv->tab_size), tab_size);
  g_signal_connect (G_OBJECT (priv->tab_size), "value_changed", G_CALLBACK (on_tab_size_changed), priv);

  priv->line_wrapping = GTK_WIDGET(gtk_builder_get_object (builder, "line_wrapping"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->line_wrapping), line_wrapping);
  g_signal_connect(GTK_CHECK_BUTTON(priv->line_wrapping), "toggled", G_CALLBACK(on_line_wrapping_toggle), priv);

  priv->higthlightcaretline = GTK_WIDGET(gtk_builder_get_object (builder, "higthlightcaretline"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->higthlightcaretline), higthlight_caret_line);
  g_signal_connect(GTK_CHECK_BUTTON(priv->higthlightcaretline), "toggled", G_CALLBACK(on_save_higthlightcaretline_toggle), priv);

  const gchar *shared_source_location;
  const gchar *php_binary_location;
  const gchar *php_file_extensions;
  g_object_get(priv->prefmg, "shared_source_location", &shared_source_location, 
    "php_binary_location", &php_binary_location,"php_file_extensions", &php_file_extensions, NULL);

  priv->php_file_entry = GTK_WIDGET(gtk_builder_get_object (builder, "php_file_entry"));
  gtk_entry_set_text(GTK_ENTRY(priv->php_file_entry), php_binary_location);
  g_signal_connect(G_OBJECT(priv->php_file_entry), "changed", G_CALLBACK(on_php_binary_location_changed), priv);

  priv->file_extensions = GTK_WIDGET(gtk_builder_get_object (builder, "file_extensions"));
  gtk_entry_set_text(GTK_ENTRY(priv->file_extensions), php_file_extensions);
  g_signal_connect(G_OBJECT(priv->file_extensions), "changed", G_CALLBACK(on_php_file_extensions_changed), priv);

  priv->shared_source = GTK_WIDGET(gtk_builder_get_object (builder, "shared_source"));
  gtk_entry_set_text(GTK_ENTRY(priv->shared_source), shared_source_location);
  g_signal_connect(G_OBJECT(priv->shared_source), "changed", G_CALLBACK(on_shared_source_changed), priv);

  gchar *style_font_name;
  guint size;
  g_object_get(priv->prefmg, "style_font_name", &style_font_name,"font_size", &size, NULL);
  gchar *font_setting = g_strdup_printf("%s %d", style_font_name + 1, size);
  priv->font_settings = GTK_WIDGET(gtk_builder_get_object (builder, "font_button"));
  gtk_font_button_set_font_name(GTK_FONT_BUTTON(priv->font_settings), font_setting);
  g_signal_connect(G_OBJECT(priv->font_settings), "font-set", G_CALLBACK(on_font_settings_toggle), priv);
  g_free(font_setting);

  
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *selection;

  priv->size_combo = GTK_WIDGET(gtk_builder_get_object (builder, "schemes_treeview"));

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

  populate_color_scheme_list (dialog, NULL);
	
  g_signal_connect (priv->size_combo,
			  "cursor-changed",
			  G_CALLBACK (style_scheme_changed),
			  dialog);

/* coding tab */
  GtkWidget *princod = GTK_WIDGET(gtk_builder_get_object (builder, "templates_box"));
  priv->templates_editor = templates_editor_new ();
  gtk_widget_show (priv->templates_editor);
  gtk_box_pack_start (GTK_BOX (princod), priv->templates_editor, TRUE, TRUE, 8);
/**/

/* plugins tab */
  GtkWidget *prinplug = GTK_WIDGET(gtk_builder_get_object (builder, "Plugin_vbox"));
  priv->plugindialog = plugin_dialog_new();
  gtk_box_pack_start (GTK_BOX (prinplug), priv->plugindialog, TRUE, TRUE, 0);

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

	dialog = g_object_new (GOBJECT_TYPE_PREFERENCES_DIALOG, NULL);

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
