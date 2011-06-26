/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar) 

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
#include "main_window_callbacks.h"
#include "toolbar.h"

#define TOOLBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_TOOLBAR,              \
						ToolBarPrivate))

struct _ToolBarPrivate
{
  GtkWidget *toolbar;
/* main toolbar widgets */
  GtkWidget *button_new;
  GtkWidget *button_open;
  GtkWidget *button_save;
  GtkWidget *button_save_as;
  GtkWidget *button_undo;
  GtkWidget *button_redo;
  GtkWidget *button_cut;
  GtkWidget *button_copy;
  GtkWidget *button_paste;
  GtkWidget *button_find;
  GtkWidget *button_replace;
  GtkWidget *button_indent;
  GtkWidget *button_unindent;
  GtkWidget *button_zoom_in;
  GtkWidget *button_zoom_out;
  GtkWidget *button_zoom_100;
};

G_DEFINE_TYPE(ToolBar, TOOLBAR, GTK_TYPE_BOX)

static void
TOOLBAR_class_init (ToolBarClass *klass)
{
	g_type_class_add_private (klass, sizeof (ToolBarPrivate));
}

static void
TOOLBAR_init (ToolBar *toolbar)
{
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  GtkBuilder *builder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (builder, GPHPEDIT_UI_DIR "/toolbar.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }


  priv->toolbar = GTK_WIDGET(gtk_builder_get_object (builder, "maintoolbar"));
  gtk_box_pack_start (GTK_BOX (toolbar), priv->toolbar, TRUE, TRUE, 0);

  GtkStyleContext *context = gtk_widget_get_style_context (GTK_WIDGET(priv->toolbar));
  gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
  gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
  gtk_style_context_save (context);

  priv->button_new = GTK_WIDGET(gtk_builder_get_object (builder, "new"));
  g_signal_connect (G_OBJECT (priv->button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
  priv->button_open = GTK_WIDGET(gtk_builder_get_object (builder, "open"));
  g_signal_connect (G_OBJECT (priv->button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
  priv->button_save = GTK_WIDGET(gtk_builder_get_object (builder, "save"));
  g_signal_connect (G_OBJECT (priv->button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
  priv->button_save_as = GTK_WIDGET(gtk_builder_get_object (builder, "save_as"));
  g_signal_connect (G_OBJECT (priv->button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);

  priv->button_undo = GTK_WIDGET(gtk_builder_get_object (builder, "undo"));
  g_signal_connect (G_OBJECT (priv->button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
  priv->button_redo = GTK_WIDGET(gtk_builder_get_object (builder, "redo"));
  g_signal_connect (G_OBJECT (priv->button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);

  priv->button_cut = GTK_WIDGET(gtk_builder_get_object (builder, "cut"));
  g_signal_connect (G_OBJECT (priv->button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);
  priv->button_copy = GTK_WIDGET(gtk_builder_get_object (builder, "copy"));
  g_signal_connect (G_OBJECT (priv->button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);
  priv->button_paste = GTK_WIDGET(gtk_builder_get_object (builder, "paste"));
  g_signal_connect (G_OBJECT (priv->button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);

  priv->button_find = GTK_WIDGET(gtk_builder_get_object (builder, "find"));
  g_signal_connect (G_OBJECT (priv->button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);
  priv->button_replace = GTK_WIDGET(gtk_builder_get_object (builder, "replace"));
  g_signal_connect (G_OBJECT (priv->button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);

  /* Add the indent/unindent operations to the Main Toolbar */
  /*indent block*/
  priv->button_indent = GTK_WIDGET(gtk_builder_get_object (builder, "indent"));
  g_signal_connect (G_OBJECT (priv->button_indent), "clicked", G_CALLBACK (block_indent), NULL);
  priv->button_unindent = GTK_WIDGET(gtk_builder_get_object (builder, "unindent"));
  g_signal_connect (G_OBJECT (priv->button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);

  /* Add Zoom operations to the main Toolbar */
  priv->button_zoom_in = GTK_WIDGET(gtk_builder_get_object (builder, "zoomin"));
  g_signal_connect (G_OBJECT (priv->button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
  priv->button_zoom_out = GTK_WIDGET(gtk_builder_get_object (builder, "zoomout"));
  g_signal_connect (G_OBJECT (priv->button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
  priv->button_zoom_100 = GTK_WIDGET(gtk_builder_get_object (builder, "zoom100"));
  g_signal_connect (G_OBJECT (priv->button_zoom_100), "clicked", G_CALLBACK (zoom_100), NULL);

}

GtkWidget *
toolbar_new (void)
{
  return GTK_WIDGET(g_object_new (GOBJECT_TYPE_TOOLBAR, NULL));
}

void toolbar_update_controls(ToolBar *toolbar, gboolean is_scintilla, gboolean isreadonly)
{
  if (!toolbar) return ;
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  if (!priv) return ;
  if (is_scintilla){
    //activate toolbar items
    gtk_widget_set_sensitive (priv->button_cut, TRUE);
    gtk_widget_set_sensitive (priv->button_paste, TRUE);
    gtk_widget_set_sensitive (priv->button_undo, TRUE);
    gtk_widget_set_sensitive (priv->button_redo, TRUE);
    gtk_widget_set_sensitive (priv->button_replace, TRUE);
    gtk_widget_set_sensitive (priv->button_indent, TRUE);
    gtk_widget_set_sensitive (priv->button_unindent, TRUE);
    gtk_widget_set_sensitive (priv->button_save_as, TRUE);
    if (isreadonly){
      gtk_widget_set_sensitive (priv->button_save, FALSE);
    } else {
      gtk_widget_set_sensitive (priv->button_save, TRUE);
    } 
  } else {
      //deactivate toolbar items
    gtk_widget_set_sensitive (priv->button_cut, FALSE);
    gtk_widget_set_sensitive (priv->button_paste, FALSE);
    gtk_widget_set_sensitive (priv->button_undo, FALSE);
    gtk_widget_set_sensitive (priv->button_redo, FALSE);
    gtk_widget_set_sensitive (priv->button_replace, FALSE);
    gtk_widget_set_sensitive (priv->button_indent, FALSE);
    gtk_widget_set_sensitive (priv->button_unindent, FALSE);
    gtk_widget_set_sensitive (priv->button_save, FALSE);
    gtk_widget_set_sensitive (priv->button_save_as, FALSE);
  }
}
