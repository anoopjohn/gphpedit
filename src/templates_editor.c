/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno (for vijona.com.ar) 

   For more information or to find the latest release, visit our 
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "templates_editor.h"
#include "templates.h"
#include "edit_template.h"

#include "tab.h"
#include "debug.h"

#define TEMPLATES_EDITOR_NAME_TITLE _("Templates")

#define TEMPLATES_EDITOR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), OBJECT_TYPE_TEMPLATES_EDITOR, TemplatesEditorPrivate))

struct _TemplatesEditorPrivate
{
  gchar *current_key;
  TemplatesManager *tempmg;

  GtkWidget *add_template_button;
  GtkWidget *edit_template_button;
  GtkWidget *delete_template_button;

  GtkWidget *template_sample;
  GtkListStore *template_store;
  GtkTreeSelection *template_selection;
  GtkWidget *Templates;
};

static void templates_editor_finalize (GObject *object);

G_DEFINE_TYPE(TemplatesEditor, templates_editor, GTK_TYPE_VBOX)

static void 
templates_editor_class_init (TemplatesEditorClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->finalize = templates_editor_finalize;
	g_type_class_add_private (object_class, sizeof (TemplatesEditorPrivate));
}


static void add_to_template_list(gpointer data, gpointer user_data)
{
  GtkTreeIter iter;
  TemplatesEditorPrivate *priv = (TemplatesEditorPrivate *) user_data;

  gtk_list_store_append (priv->template_store, &iter);
  gtk_list_store_set (priv->template_store, &iter,0, (gchar *)data, -1);
}

static void add_templates_to_store(TemplatesEditorPrivate *priv)
{
  GList *keylist = get_templates_manager_templates_names(priv->tempmg);
  g_list_foreach (keylist, add_to_template_list, priv);
}

static void update_template_display(TemplatesEditorPrivate *priv, gchar *template)
{
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->template_sample));

  gtk_text_buffer_set_text (buffer, template, -1);  
}

static void template_row_activated(GtkTreeSelection *selection, gpointer data)
{
  GtkTreeModel *model;
  gchar *content, *template;
  GtkTreeIter iter;
  TemplatesEditorPrivate *priv = (TemplatesEditorPrivate *) data;
  if (priv->current_key) {
    g_free(priv->current_key);
  }
    if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, 0, &priv->current_key, -1);

    // display template content
    template = template_find(priv->tempmg, priv->current_key);
    if (template) {
      content = template_convert_to_display(template);
      update_template_display(data, content);
      g_free(content);
    }
  }
}

static void templates_treeview_add_column(TemplatesEditorPrivate *priv)
{  
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  /* column for description */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name/Shortcut"), renderer, "text", 0, NULL);
  gtk_tree_view_column_set_sort_column_id (column, 0);
  gtk_tree_view_append_column (GTK_TREE_VIEW(priv->Templates), column);
}

static void add_template_clicked(GtkButton *button, gpointer data)
{
  gchar *name;
  gchar *template;
  GtkTreeIter iter;
  GtkTextBuffer *buffer;
  GtkTextIter begin, end;
  gchar *content;
  TemplatesEditorPrivate *priv = (TemplatesEditorPrivate *) data;
  // create dialog
  GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET(button));
  create_edit_template_dialog(window);

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
    template_delete(priv->tempmg, priv->current_key); // Just in case you accidentally type the name of an existing template
    template_replace(priv->tempmg, name, template);
    
    // add to treeview
    gtk_list_store_append (priv->template_store, &iter);
    gtk_list_store_set (priv->template_store, &iter,0, (gchar *)name, -1);
  }
  
  // destroy/null dialog
  gtk_widget_destroy(edit_template_dialog.window1);
  edit_template_dialog.window1 = NULL;
}

static void edit_template_clicked(GtkButton *button, gpointer data)
{
  gchar *name;
  gchar *template;
  GtkTextBuffer *buffer;
  GtkTextIter begin, end;
  gchar *content;
  GtkTreeIter iter;
  TemplatesEditorPrivate *priv = (TemplatesEditorPrivate *) data;

  // create dialog
  GtkWidget *window = gtk_widget_get_toplevel (GTK_WIDGET(button));
  create_edit_template_dialog(window);

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
    template_delete(priv->tempmg, priv->current_key);
    template_replace(priv->tempmg, name, template);
    
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

static void delete_template_clicked(GtkButton *button, gpointer data)
{
  GtkTreeIter iter;
  TemplatesEditorPrivate *priv = (TemplatesEditorPrivate *) data;

  gchar *message = g_strdup_printf(_("Are you sure you want to delete template %s?"),priv->current_key);
  // confirm deletion with dialog
  if (yes_no_dialog (_("gPHPEdit"), message) == GTK_RESPONSE_YES) {
    // delete from templates
    template_delete(priv->tempmg, priv->current_key);

    // delete from treeview
    gtk_tree_selection_get_selected (priv->template_selection, NULL, &iter);
    gtk_list_store_remove (priv->template_store, &iter);
    priv->current_key = NULL;
  }
   g_free(message);
}

static void 
templates_editor_init (TemplatesEditor *te)
{
  TemplatesEditorPrivate *priv = TEMPLATES_EDITOR_GET_PRIVATE(te);

  priv->current_key=NULL;

  priv->tempmg = templates_manager_new();

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (te), vbox, TRUE, TRUE, 0);

  GtkWidget *label = gtk_label_new_with_mnemonic (_("Active Templates:"));
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

  GtkWidget *hbox20 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox20);
  gtk_box_pack_start (GTK_BOX (vbox), hbox20, TRUE, TRUE, 8);

  GtkWidget *scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_box_pack_start (GTK_BOX (hbox20), scrolledwindow1, TRUE, TRUE, 0);
  
  priv->template_store = gtk_list_store_new (1, G_TYPE_STRING);
  add_templates_to_store(priv);
  
  priv->Templates = gtk_tree_view_new_with_model (GTK_TREE_MODEL(priv->template_store));
  // g_object_unref (preferences_dialog.template_store);
  // Can't unref it because I need to add to it later....
  templates_treeview_add_column(priv);
  
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (priv->Templates), 0);
  gtk_widget_show (priv->Templates);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), priv->Templates);

  priv->template_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->Templates));
  gtk_tree_selection_set_mode (priv->template_selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (priv->template_selection), "changed", G_CALLBACK (template_row_activated), priv);

  GtkWidget *vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox20), vbox2, FALSE, FALSE, 0);

  priv->add_template_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_widget_show (priv->add_template_button);
  gtk_box_pack_start (GTK_BOX (vbox2), priv->add_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->add_template_button), 2);
  g_signal_connect (G_OBJECT (priv->add_template_button), "clicked", G_CALLBACK (add_template_clicked), priv);
  
  priv->edit_template_button = gtk_button_new_from_stock (GTK_STOCK_EDIT);
  gtk_widget_show (priv->edit_template_button);
  gtk_box_pack_start (GTK_BOX (vbox2), priv->edit_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->edit_template_button), 2);
  g_signal_connect (G_OBJECT (priv->edit_template_button), "clicked", G_CALLBACK (edit_template_clicked), priv);
   
  priv->delete_template_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_widget_show (priv->delete_template_button);
  gtk_box_pack_start (GTK_BOX (vbox2), priv->delete_template_button, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (priv->delete_template_button), 2);
  g_signal_connect (G_OBJECT (priv->delete_template_button), "clicked", G_CALLBACK (delete_template_clicked), priv);

  GtkWidget *label50 = gtk_label_new_with_mnemonic (_("Templates:"));
  gtk_label_set_justify (GTK_LABEL (label50), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label50), 0.0, 0.5);
  gtk_box_pack_start(GTK_BOX(vbox), label50, FALSE, FALSE, 0);

  GtkWidget *template_sample_scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (template_sample_scrolled);
  gtk_box_pack_start (GTK_BOX (vbox), template_sample_scrolled, FALSE, FALSE, 0);
  gtk_widget_set_size_request (template_sample_scrolled, -1, 160);
  
  priv->template_sample = gtk_text_view_new ();
  gtk_widget_show (priv->template_sample);
  gtk_container_add (GTK_CONTAINER (template_sample_scrolled), priv->template_sample);
  gtk_text_view_set_editable (GTK_TEXT_VIEW(priv->template_sample), FALSE);

}

static void
templates_editor_finalize (GObject *object)
{
    TemplatesEditor *te = TEMPLATES_EDITOR (object);
    TemplatesEditorPrivate *priv = TEMPLATES_EDITOR_GET_PRIVATE(te);

    if(priv->tempmg) g_object_unref(priv->tempmg);

    G_OBJECT_CLASS (templates_editor_parent_class)->finalize (object);
}



GtkWidget *templates_editor_new (void)
{
	return g_object_new (OBJECT_TYPE_TEMPLATES_EDITOR, NULL);
}
