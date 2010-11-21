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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main_window.h"
#include "main_window_callbacks.h"

#include <gdk/gdkkeysyms.h>
#include "toolbar.h"

#define TOOLBAR_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), \
						GOBJECT_TYPE_TOOLBAR,              \
						ToolBarPrivate))

/* toolbar type */
enum 
{
	MAIN_TOOLBAR = 0,
	FIND_TOOLBAR = 1
};

struct _ToolBarPrivate 
{
  gint type; /* 0 for main toolbar 1 for find toolbar*/

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
  GtkToolItem *toolbar_separator;
  GtkWidget *button_indent;
  GtkWidget *button_unindent;
  GtkWidget *button_zoom_in;
  GtkWidget *button_zoom_out;
  GtkWidget *button_zoom_100;

/* find toolbar widgets */
  GtkAccelGroup *accel_group;
  GtkWidget *toolbar_find;
  GtkWidget *search_label;
  GtkWidget *search_entry;
  GtkWidget *goto_label;
  GtkWidget *goto_entry;
  GtkWidget *cleanimg;
  GtkEntryCompletion *completion;
  GtkTreeModel *completion_model;
};

G_DEFINE_TYPE(ToolBar, TOOLBAR, GTK_TYPE_TOOLBAR)

enum
{
  PROP_0,
  PROP_TOOLBAR_TYPE,
  PROP_ACCEL_GROUP
};

static void
TOOLBAR_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(object);

	switch (prop_id)
	{
		case PROP_TOOLBAR_TYPE:
			priv->type = g_value_get_int (value);
			break;
		case PROP_ACCEL_GROUP:
			priv->accel_group = g_value_get_object (value);
			break;
 		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
TOOLBAR_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(object);

	switch (prop_id)
	{
		case PROP_TOOLBAR_TYPE:
			g_value_set_int (value, priv->type);
			break;
		case PROP_ACCEL_GROUP:
			g_value_set_object (value, priv->accel_group);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
TOOLBAR_class_init (ToolBarClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->set_property = TOOLBAR_set_property;
  object_class->get_property = TOOLBAR_get_property;

  g_object_class_install_property (object_class,
                              PROP_TOOLBAR_TYPE,
                              g_param_spec_int ("toolbar_type",
                              NULL, NULL, 0, G_MAXINT, 
                              MAIN_TOOLBAR, G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  g_object_class_install_property (object_class,
                              PROP_ACCEL_GROUP,
                              g_param_spec_object ("accel_group",
                              NULL, NULL, GTK_TYPE_ACCEL_GROUP,
                              G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

	g_type_class_add_private (klass, sizeof (ToolBarPrivate));
}

/**
* sincronice toolbar icon size menu item with current toolbar icon size value
*/
static void sincronice_menu_items_size (GtkToolbar *toolbar){
  /*sincronice with menu items*/
  if (gtk_toolbar_get_icon_size (toolbar)==GTK_ICON_SIZE_SMALL_TOOLBAR){
    menubar_set_toolbar_size(MENUBAR(main_window.menu), TRUE);
  }else {
    menubar_set_toolbar_size(MENUBAR(main_window.menu), FALSE);
  }
}

/**
* create_toolbar_stock_item
* creates a new toolbar stock item
*/

static inline void create_toolbar_stock_item(GtkWidget **toolitem,ToolBar *toolbar,const gchar *stock_id, gchar *tooltip_text){
  *toolitem = GTK_WIDGET(gtk_tool_button_new_from_stock(stock_id));
  gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (*toolitem), tooltip_text);
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (*toolitem), -1);
  gtk_widget_show (*toolitem);
}


static void
main_toolbar_init (ToolBar *toolbar)
{
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);

  /* Add the File operations to the Main Toolbar */
  create_toolbar_stock_item(&priv->button_new,toolbar,GTK_STOCK_NEW, _("New File"));

  create_toolbar_stock_item(&priv->button_open,toolbar,GTK_STOCK_OPEN, _("Open File"));

  create_toolbar_stock_item(&priv->button_save,toolbar,GTK_STOCK_SAVE, _("Save current File"));

  create_toolbar_stock_item(&priv->button_save_as,toolbar,GTK_STOCK_SAVE_AS, _("Save File As..."));

  g_signal_connect (G_OBJECT (priv->button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
  g_signal_connect (G_OBJECT (priv->button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
  g_signal_connect (G_OBJECT (priv->button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
  g_signal_connect (G_OBJECT (priv->button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);

  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));
  /* Add the Undo operations to the Main Toolbar */
  create_toolbar_stock_item(&priv->button_undo,toolbar,GTK_STOCK_UNDO, _("Undo last change"));
  create_toolbar_stock_item(&priv->button_redo,toolbar,GTK_STOCK_REDO, _("Redo last change"));
  g_signal_connect (G_OBJECT (priv->button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
  g_signal_connect (G_OBJECT (priv->button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);
  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));

  /* Add the Clipboard operations to the Main Toolbar */

  create_toolbar_stock_item(&priv->button_cut,toolbar,GTK_STOCK_CUT, _("Cut Current Selection"));
  g_signal_connect (G_OBJECT (priv->button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);

  create_toolbar_stock_item(&priv->button_copy,toolbar,GTK_STOCK_COPY, _("Copy Current Selection"));
  g_signal_connect (G_OBJECT (priv->button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);

  create_toolbar_stock_item(&priv->button_paste,toolbar,GTK_STOCK_PASTE, _("Paste text from clipboard"));
  g_signal_connect (G_OBJECT (priv->button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);
  
  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));
  /* Add the Search operations to the Main Toolbar */
  create_toolbar_stock_item(&priv->button_find,toolbar,GTK_STOCK_FIND, _("Find text"));
  g_signal_connect (G_OBJECT (priv->button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);

  create_toolbar_stock_item(&priv->button_replace,toolbar,GTK_STOCK_FIND_AND_REPLACE, _("Replace Text"));
  g_signal_connect (G_OBJECT (priv->button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);
  
  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));

  /* Add the indent/unindent operations to the Main Toolbar */
  /*indent block*/
  create_toolbar_stock_item(&priv->button_indent,toolbar,GTK_STOCK_INDENT, _("Indent Selected Text"));
  g_signal_connect (G_OBJECT (priv->button_indent), "clicked", G_CALLBACK (block_indent), NULL);
  /*unindent block*/
  create_toolbar_stock_item(&priv->button_unindent,toolbar,GTK_STOCK_UNINDENT, _("Unindent Selected Text"));
  g_signal_connect (G_OBJECT (priv->button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);
  
  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));
  /* Add Zoom operations to the main Toolbar */
  /* zoom in */
  create_toolbar_stock_item(&priv->button_zoom_in,toolbar,GTK_STOCK_ZOOM_IN, _("Increases Zoom level"));
  g_signal_connect (G_OBJECT (priv->button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
  /* zoom out */
  create_toolbar_stock_item(&priv->button_zoom_out,toolbar,GTK_STOCK_ZOOM_OUT, _("Decreases Zoom level"));
  g_signal_connect (G_OBJECT (priv->button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
  /* zoom 100% */
  create_toolbar_stock_item(&priv->button_zoom_100,toolbar,GTK_STOCK_ZOOM_100, _("Zoom 100%"));
  g_signal_connect (G_OBJECT (priv->button_zoom_100), "clicked", G_CALLBACK (zoom_100), NULL);

}

static inline void create_entry(GtkWidget **entry, const gchar *tooltip_text,gint max_lenght){
  *entry = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(*entry),max_lenght);
  gtk_entry_set_width_chars(GTK_ENTRY(*entry),max_lenght + 1);
  gtk_widget_set_tooltip_text (*entry,tooltip_text);
  gtk_entry_set_icon_from_stock (GTK_ENTRY(*entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
  gtk_widget_show(*entry);
}

/**
* on_cleanicon_press
* Clear entry text 
*/
void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
    gtk_entry_set_text (entry,"");
}

/**
* create and insert a new toolbar item with a custom widget in it
*/
static inline void create_custom_toolbar_item (GtkToolbar *toolbar, GtkWidget *control){
  GtkToolItem *item;
  item=gtk_tool_item_new();
  gtk_tool_item_set_expand (item, FALSE);
  gtk_container_add (GTK_CONTAINER (item), control);
  gtk_toolbar_insert(toolbar, GTK_TOOL_ITEM (item), -1);
  gtk_widget_show(GTK_WIDGET(item));
}

/* Creates a tree model containing the completions */
GtkTreeModel *create_completion_model (void)
  
{
  GtkListStore *store;
  GtkTreeIter iter;
  
  store = gtk_list_store_new (1, G_TYPE_STRING);
  GSList *walk;
  for (walk = get_preferences_manager_search_history(main_window.prefmg); walk!=NULL; walk = g_slist_next(walk)) {
    /* Append one word */
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter, 0, (gchar *) walk->data, -1);
    //g_print("completion added:%s\n",(gchar *) walk->data);
   }
 
  return GTK_TREE_MODEL (store);
}

static void
find_toolbar_init (ToolBar *toolbar)
{
  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  GtkAccelGroup *accel_group;
  g_object_get(toolbar, "accel_group", &accel_group, NULL);

  priv->search_label = gtk_label_new(_("Search for: "));
  gtk_widget_show(priv->search_label);
  create_custom_toolbar_item (GTK_TOOLBAR(toolbar), priv->search_label);

  create_entry(&priv->search_entry, _("Incremental search"), 20);
  g_signal_connect (G_OBJECT (priv->search_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);

  /* search completion code */
  priv->completion= gtk_entry_completion_new();

  /* Create a tree model and use it as the completion model */
  priv->completion_model = create_completion_model ();
  gtk_entry_completion_set_model (priv->completion, priv->completion_model);
  g_object_unref (priv->completion_model);
    
   /* Use model column 0 as the text column */
  gtk_entry_completion_set_text_column (priv->completion, 0);
  /* set autocompletion settings: complete inline and show pop-up */
  gtk_entry_completion_set_popup_completion (priv->completion,TRUE);
  gtk_entry_completion_set_inline_completion (priv->completion,TRUE);
  /* set min match as 2 */
  gtk_entry_completion_set_minimum_key_length (priv->completion,2);
  /* Assign the completion to the entry */
  gtk_entry_set_completion (GTK_ENTRY(priv->search_entry), priv->completion);
  g_object_unref (priv->completion);

  /* connect entry signals */
  gtk_widget_add_accelerator (priv->search_entry, "grab-focus", accel_group, GDK_i, GDK_CONTROL_MASK, 0);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "insert_text", G_CALLBACK(inc_search_typed), NULL);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "key_release_event", G_CALLBACK(inc_search_key_release_event), NULL);
  g_signal_connect_after(G_OBJECT(priv->search_entry), "activate", G_CALLBACK(inc_search_activate), NULL);

  create_custom_toolbar_item (GTK_TOOLBAR(toolbar), priv->search_entry);

  /* create a new separator */
  priv->toolbar_separator=gtk_separator_tool_item_new();
  gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (priv->toolbar_separator), -1);
  gtk_widget_show (GTK_WIDGET(priv->toolbar_separator));

  /* goto widgets */
  priv->goto_label = gtk_label_new(_("Go to line: "));
  gtk_widget_show(priv->goto_label);
  create_custom_toolbar_item (GTK_TOOLBAR(toolbar), priv->goto_label);
  /* create goto entry */
  create_entry(&priv->goto_entry, _("Go to line"), 8);
  gtk_widget_add_accelerator (priv->goto_entry, "grab-focus", accel_group, GDK_g, GDK_CONTROL_MASK, 0);
  g_signal_connect (G_OBJECT (priv->goto_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
  g_signal_connect_after(G_OBJECT(priv->goto_entry), "activate", G_CALLBACK(goto_line_activate), NULL);
  /* create a new toolbar item with the entry */
  create_custom_toolbar_item (GTK_TOOLBAR(toolbar), priv->goto_entry);

}

static void
TOOLBAR_init (ToolBar *toolbar)
{
//  ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
 /* set toolbar style */
  gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);
  gtk_toolbar_set_style (GTK_TOOLBAR(toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR(toolbar), TRUE);

  /* sincronice toolbar icon size */
  sincronice_menu_items_size (GTK_TOOLBAR (toolbar));
}

GtkWidget *
toolbar_new (void)
{
  ToolBar *toolbar = g_object_new (GOBJECT_TYPE_TOOLBAR, NULL);
  main_toolbar_init (toolbar);

	return GTK_WIDGET(toolbar);
}

GtkWidget *toolbar_find_new (GtkAccelGroup *accel_group)
{
  ToolBar *toolbar = g_object_new (GOBJECT_TYPE_TOOLBAR, "toolbar_type", FIND_TOOLBAR, 
    "accel_group", accel_group, NULL);
  find_toolbar_init (toolbar);

	return GTK_WIDGET(toolbar);
}

void toolbar_set_search_text(ToolBar *toolbar, gchar *text){
 ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  if (priv->type==FIND_TOOLBAR){
  if (text) gtk_entry_set_text(GTK_ENTRY(priv->search_entry), text);
  gtk_widget_grab_focus(GTK_WIDGET(priv->search_entry));
  }
}

void toolbar_completion_add_text(ToolBar *toolbar, const gchar *text){
 ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  if (priv->type==FIND_TOOLBAR){
  if (text) gtk_entry_completion_insert_action_text (priv->completion, 0, g_strdup(text));    
  }
}

void toolbar_grab_goto_focus(ToolBar *toolbar){
 ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  if (priv->type==FIND_TOOLBAR){
      gtk_widget_grab_focus(GTK_WIDGET(priv->goto_entry));
  }
}

void toolbar_update_controls(ToolBar *toolbar, gboolean is_scintilla, gboolean isreadonly)
{
 ToolBarPrivate *priv = TOOLBAR_GET_PRIVATE(toolbar);
  if (is_scintilla){
    //activate toolbar items
    if (priv->type==MAIN_TOOLBAR){
      gtk_widget_set_sensitive (priv->button_cut, TRUE);
      gtk_widget_set_sensitive (priv->button_paste, TRUE);
      gtk_widget_set_sensitive (priv->button_undo, TRUE);
      gtk_widget_set_sensitive (priv->button_redo, TRUE);
      gtk_widget_set_sensitive (priv->button_replace, TRUE);
      gtk_widget_set_sensitive (priv->button_indent, TRUE);
      gtk_widget_set_sensitive (priv->button_unindent, TRUE);
      if (isreadonly){
        gtk_widget_set_sensitive (priv->button_save, FALSE);
      } else {
        gtk_widget_set_sensitive (priv->button_save, TRUE);
      } 
      gtk_widget_set_sensitive (priv->button_save_as, TRUE);
    } else {
      gtk_widget_set_sensitive (priv->search_entry, TRUE);
      gtk_widget_set_sensitive (priv->goto_entry, TRUE);
    }
  }else{
      //deactivate toolbar items
        if (priv->type==MAIN_TOOLBAR){
          gtk_widget_set_sensitive (priv->button_cut, FALSE);
          gtk_widget_set_sensitive (priv->button_paste, FALSE);
          gtk_widget_set_sensitive (priv->button_undo, FALSE);
          gtk_widget_set_sensitive (priv->button_redo, FALSE);
          gtk_widget_set_sensitive (priv->button_replace, FALSE);
          gtk_widget_set_sensitive (priv->button_indent, FALSE);
          gtk_widget_set_sensitive (priv->button_unindent, FALSE);
          gtk_widget_set_sensitive (priv->button_save, FALSE);
          gtk_widget_set_sensitive (priv->button_save_as, FALSE);
        } else {
          gtk_widget_set_sensitive (priv->search_entry, FALSE);
          gtk_widget_set_sensitive (priv->goto_entry, FALSE);
        }
  }
}
