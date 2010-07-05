/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno (for vijona.com.ar) 

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

#include "syntax_check_window.h"
#include "main_window_callbacks.h"
struct _GtkSyntax_Check_WindowPrivate
{
  
  GtkWidget *scrolledwindow;
  // My new best friend: http://developer.gnome.org/doc/API/2.0/gtk/TreeWidget.html
  GtkListStore *lint_store;
  GtkCellRenderer *lint_renderer;
  GtkWidget *lint_view;
  GtkTreeViewColumn *lint_column;
  GtkTreeSelection *lint_select;
};

#define GTK_SYNTAX_CHECK_WINDOW_GET_PRIVATE(obj)	(G_TYPE_INSTANCE_GET_PRIVATE ((obj), GTK_TYPE_SYNTAX_CHECK_WINDOW, GtkSyntax_Check_WindowPrivate))
static void
gtk_syntax_check_window_class_init (GtkSyntax_Check_WindowClass *klass);
static void
gtk_syntax_check_window_init (GtkSyntax_Check_Window *menu);
static void     gtk_syntax_check_window_finalize    (GObject                   *object);
static void     gtk_syntax_check_window_dispose     (GObject                   *object);
void lint_row_activated (GtkTreeSelection *selection, gpointer data);
static gpointer gtk_syntax_check_window_parent_class;

/*
 * gtk_syntax_check_window_get_type
 * register gtk_syntax_check_window type and returns a new GType
*/
GType
gtk_syntax_check_window_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (GtkSyntax_Check_WindowClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gtk_syntax_check_window_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (GtkSyntax_Check_Window),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gtk_syntax_check_window_init,
        };

        our_type = g_type_register_static (GTK_TYPE_BOX, "GtkSyntax_Check_Window",
                                           &our_info, 0);
  }
    
    return our_type;
}

static void
gtk_syntax_check_window_class_init (GtkSyntax_Check_WindowClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->dispose = gtk_syntax_check_window_dispose;
  gobject_class->finalize = gtk_syntax_check_window_finalize;
  gtk_syntax_check_window_parent_class = g_type_class_peek_parent (klass);
  g_type_class_add_private (klass, sizeof (GtkSyntax_Check_WindowPrivate));
}

static void
gtk_syntax_check_window_init (GtkSyntax_Check_Window *win)
{
  GtkSyntax_Check_WindowPrivate *priv;
  
  priv = GTK_SYNTAX_CHECK_WINDOW_GET_PRIVATE (win);
  
  win->priv = priv;
  priv->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  priv->lint_view = gtk_tree_view_new ();
  gtk_container_add (GTK_CONTAINER (priv->scrolledwindow), priv->lint_view);
  priv->lint_renderer = gtk_cell_renderer_text_new ();
  priv->lint_column = gtk_tree_view_column_new_with_attributes (_("Syntax Check Output"),
                priv->lint_renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->lint_view), priv->lint_column);
  gtk_widget_set_size_request (priv->lint_view, 80,80);
  priv->lint_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->lint_view));
  gtk_tree_selection_set_mode (priv->lint_select, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (priv->lint_select), "changed",
            G_CALLBACK (lint_row_activated), NULL);
  gtk_box_pack_start(GTK_BOX(win), GTK_WIDGET(priv->scrolledwindow), TRUE, TRUE, 2);
  gtk_widget_show(priv->scrolledwindow);
  gtk_widget_show(priv->lint_view);

}

static void
gtk_syntax_check_window_finalize (GObject *object)
{
//  GtkSyntax_Check_Window *menu = GTK_SYNTAX_CHECK_WINDOW (object);
//  GtkSyntax_Check_WindowPrivate *priv = menu->priv;
  
  G_OBJECT_CLASS (gtk_syntax_check_window_parent_class)->finalize (object);
}

static void
gtk_syntax_check_window_dispose (GObject *object)
{
  GtkSyntax_Check_Window *menu = GTK_SYNTAX_CHECK_WINDOW (object);
  GtkSyntax_Check_WindowPrivate *priv = menu->priv;

  if (priv->lint_store) gtk_list_store_clear(priv->lint_store);

  G_OBJECT_CLASS (gtk_syntax_check_window_parent_class)->dispose (object);
}
void gtk_syntax_check_window_set_model(GtkSyntax_Check_Window *win, GtkListStore *model){
  GtkSyntax_Check_WindowPrivate *priv;
  priv = GTK_SYNTAX_CHECK_WINDOW_GET_PRIVATE (win);
  if (priv->lint_store) g_object_unref(priv->lint_store);
  priv->lint_store=model;
  gtk_tree_view_set_model(GTK_TREE_VIEW(priv->lint_view), GTK_TREE_MODEL(priv->lint_store));
}

void gtk_syntax_check_window_set_string(GtkSyntax_Check_Window *win, const gchar *string)
{
  GtkTreeIter iter;
  GtkSyntax_Check_WindowPrivate *priv;
  priv = GTK_SYNTAX_CHECK_WINDOW_GET_PRIVATE (win);
  if (priv->lint_store) g_object_unref(priv->lint_store);
  priv->lint_store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_list_store_clear(priv->lint_store);
  gtk_list_store_append (priv->lint_store, &iter);
  gtk_list_store_set (priv->lint_store, &iter, 0, string, -1);
  gtk_tree_view_set_model(GTK_TREE_VIEW(priv->lint_view), GTK_TREE_MODEL(priv->lint_store));
}

void lint_row_activated (GtkTreeSelection *selection, gpointer data)
{
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar *line;
  gchar *space;

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
    gtk_tree_model_get (model, &iter, 0, &line, -1);

    // Get the line
    space = strrchr(line, ' ');
    space++;
    // Go to that line
      goto_line(space);

    g_free (line);
  }
}

/*
 * Public API
 */

GtkWidget *
gtk_syntax_check_window_new (void)
{
  return g_object_new (GTK_TYPE_SYNTAX_CHECK_WINDOW, NULL);
}
