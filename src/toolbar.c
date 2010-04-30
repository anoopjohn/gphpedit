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

#include "toolbar.h"
#include "main_window.h"

/* Creates a tree model containing the completions */
GtkTreeModel *create_completion_model (void)
	
{
  GtkListStore *store;
  GtkTreeIter iter;
  
  store = gtk_list_store_new (1, G_TYPE_STRING);
  GSList *walk;
  for (walk = preferences.search_history; walk!=NULL; walk = g_slist_next(walk)) {
	  /* Append one word */
         gtk_list_store_append (store, &iter);
         gtk_list_store_set (store, &iter, 0, (gchar *) walk->data, -1);
	//g_print("completion added:%s\n",(gchar *) walk->data);
   }
 
  return GTK_TREE_MODEL (store);
}


/**
* on_cleanicon_press
* Clear entry text 
*/
void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
    gtk_entry_set_text (entry,"");
}
/**
* create_toolbar_stock_item
* creates a new toolbar stock item
*/

static inline void create_toolbar_stock_item(GtkWidget **toolitem,GtkWidget *toolbar,const gchar *stock_id, gchar *tooltip_text){
	*toolitem = GTK_WIDGET(gtk_tool_button_new_from_stock(stock_id));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (*toolitem), tooltip_text);
  	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (*toolitem), -1);
	gtk_widget_show (*toolitem);
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

/**
* sincronice toolbar icon size menu item with current toolbar icon size value
*/
static void sincronice_menu_items_size (GtkToolbar *toolbar){
	/*sincronice with menu items*/
       if (gtk_toolbar_get_icon_size (toolbar)==GTK_ICON_SIZE_SMALL_TOOLBAR){
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizesmall), TRUE);
       }else {
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizebig), TRUE);
       }
}
/* set custom toolbar style */        
static inline void set_toolbar_style(GtkToolbar *toolbar){
	gtk_container_set_border_width (GTK_CONTAINER (toolbar), 0);
	gtk_toolbar_set_style (toolbar, GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (toolbar, TRUE);
}
void main_window_create_maintoolbar(void)
{
	
        /* Create the Main Toolbar */
        main_window.toolbar_main=g_slice_new(Maintoolbar);
	main_window.toolbar_main->toolbar = gtk_toolbar_new ();
        gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_main->toolbar, FALSE, FALSE, 0);
        
	/* set toolbar style */
	set_toolbar_style(GTK_TOOLBAR (main_window.toolbar_main->toolbar));

	/* Add the File operations to the Main Toolbar */
	create_toolbar_stock_item(&main_window.toolbar_main->button_new,main_window.toolbar_main->toolbar,GTK_STOCK_NEW, _("New File"));

	create_toolbar_stock_item(&main_window.toolbar_main->button_open,main_window.toolbar_main->toolbar,GTK_STOCK_OPEN, _("Open File"));

	create_toolbar_stock_item(&main_window.toolbar_main->button_save,main_window.toolbar_main->toolbar,GTK_STOCK_SAVE, _("Save current File"));

	create_toolbar_stock_item(&main_window.toolbar_main->button_save_as,main_window.toolbar_main->toolbar,GTK_STOCK_SAVE_AS, _("Save File As..."));

	create_toolbar_stock_item(&main_window.toolbar_main->button_close,main_window.toolbar_main->toolbar,GTK_STOCK_CLOSE, _("Close Current File"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_close), "clicked", G_CALLBACK (on_close1_activate), NULL);

	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
	/* Add the Undo operations to the Main Toolbar */
	create_toolbar_stock_item(&main_window.toolbar_main->button_undo,main_window.toolbar_main->toolbar,GTK_STOCK_UNDO, _("Undo last change"));

	create_toolbar_stock_item(&main_window.toolbar_main->button_redo,main_window.toolbar_main->toolbar,GTK_STOCK_REDO, _("Redo last change"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));

	/* Add the Clipboard operations to the Main Toolbar */

	create_toolbar_stock_item(&main_window.toolbar_main->button_cut,main_window.toolbar_main->toolbar,GTK_STOCK_CUT, _("Cut Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);

	create_toolbar_stock_item(&main_window.toolbar_main->button_copy,main_window.toolbar_main->toolbar,GTK_STOCK_COPY, _("Copy Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);

	create_toolbar_stock_item(&main_window.toolbar_main->button_paste,main_window.toolbar_main->toolbar,GTK_STOCK_PASTE, _("Paste text from clipboard"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
	/* Add the Search operations to the Main Toolbar */
	create_toolbar_stock_item(&main_window.toolbar_main->button_find,main_window.toolbar_main->toolbar,GTK_STOCK_FIND, _("Find text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);

	create_toolbar_stock_item(&main_window.toolbar_main->button_replace,main_window.toolbar_main->toolbar,GTK_STOCK_FIND_AND_REPLACE, _("Replace Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));

	/* Add the indent/unindent operations to the Main Toolbar */
	/*indent block*/
	create_toolbar_stock_item(&main_window.toolbar_main->button_indent,main_window.toolbar_main->toolbar,GTK_STOCK_INDENT, _("Indent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_indent), "clicked", G_CALLBACK (block_indent), NULL);
	/*unindent block*/
	create_toolbar_stock_item(&main_window.toolbar_main->button_unindent,main_window.toolbar_main->toolbar,GTK_STOCK_UNINDENT, _("Unindent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
        /* Add Zoom operations to the main Toolbar */
	/* zoom in */
	create_toolbar_stock_item(&main_window.toolbar_main->button_zoom_in,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_IN, _("Increases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
	/* zoom out */
	 create_toolbar_stock_item(&main_window.toolbar_main->button_zoom_out,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_OUT, _("Decreases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
	/* zoom 100% */
	create_toolbar_stock_item(&main_window.toolbar_main->button_zoom_100,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_100, _("Zoom 100%"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_100), "clicked", G_CALLBACK (zoom_100), NULL);

	/* sincronice toolbar icon size */
	sincronice_menu_items_size (GTK_TOOLBAR (main_window.toolbar_main->toolbar));

	/* show toolbar after all the items are created */
	gtk_widget_show (main_window.toolbar_main->toolbar);
}

static inline void create_entry(GtkWidget **entry, const gchar *tooltip_text,gint max_lenght){
	*entry = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY(*entry),max_lenght);
	gtk_entry_set_width_chars(GTK_ENTRY(*entry),max_lenght + 1);
	gtk_widget_set_tooltip_text (*entry,tooltip_text);
        gtk_entry_set_icon_from_stock (GTK_ENTRY(*entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
	gtk_widget_show(*entry);
}

void main_window_create_findtoolbar(void){
        /* Create the Main Toolbar */
        main_window.toolbar_find=g_slice_new(Findtoolbar);
	main_window.toolbar_find->toolbar = gtk_toolbar_new ();
	gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_find->toolbar, FALSE, FALSE, 0);

	/* set toolbar style */
	set_toolbar_style(GTK_TOOLBAR (main_window.toolbar_find->toolbar));

	main_window.toolbar_find->search_label = gtk_label_new(_("Search for: "));
	gtk_widget_show(main_window.toolbar_find->search_label);
	create_custom_toolbar_item (GTK_TOOLBAR(main_window.toolbar_find->toolbar), main_window.toolbar_find->search_label);

	create_entry(&main_window.toolbar_find->search_entry, _("Incremental search"),20);
        g_signal_connect (G_OBJECT (main_window.toolbar_find->search_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);

	/* search completion code */
	main_window.toolbar_find->completion= gtk_entry_completion_new();

	/* Create a tree model and use it as the completion model */
	main_window.toolbar_find->completion_model = create_completion_model ();
	gtk_entry_completion_set_model (main_window.toolbar_find->completion, main_window.toolbar_find->completion_model);
	g_object_unref (main_window.toolbar_find->completion_model);
    
 	/* Use model column 0 as the text column */
	gtk_entry_completion_set_text_column (main_window.toolbar_find->completion, 0);
	/* set autocompletion settings: complete inline and show pop-up */
	gtk_entry_completion_set_popup_completion (main_window.toolbar_find->completion,TRUE);
	gtk_entry_completion_set_inline_completion (main_window.toolbar_find->completion,TRUE);
	/* set min match as 2 */
	gtk_entry_completion_set_minimum_key_length (main_window.toolbar_find->completion,2);
	/* Assign the completion to the entry */
    	gtk_entry_set_completion (GTK_ENTRY(main_window.toolbar_find->search_entry), main_window.toolbar_find->completion);
    	g_object_unref (main_window.toolbar_find->completion);
 
	/* connect entry signals */
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "insert_text", G_CALLBACK(inc_search_typed), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "key_release_event", G_CALLBACK(inc_search_key_release_event), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "activate", G_CALLBACK(inc_search_activate), NULL);

	create_custom_toolbar_item (GTK_TOOLBAR(main_window.toolbar_find->toolbar), main_window.toolbar_find->search_entry);

	/* create a new separator */
	main_window.toolbar_find->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (main_window.toolbar_find->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_find->toolbar_separator));

	/* goto widgets */
	main_window.toolbar_find->goto_label = gtk_label_new(_("Go to line: "));
	gtk_widget_show(main_window.toolbar_find->goto_label);
	create_custom_toolbar_item (GTK_TOOLBAR(main_window.toolbar_find->toolbar), main_window.toolbar_find->goto_label);
	/* create goto entry */
	create_entry(&main_window.toolbar_find->goto_entry, _("Go to line"),8);
        g_signal_connect (G_OBJECT (main_window.toolbar_find->goto_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->goto_entry), "activate", G_CALLBACK(goto_line_activate), NULL);
	/* create a new toolbar item with the entry */
	create_custom_toolbar_item (GTK_TOOLBAR(main_window.toolbar_find->toolbar), main_window.toolbar_find->goto_entry);
	
	/* show toolbar after all the items are created */
	gtk_widget_show (main_window.toolbar_find->toolbar);
}
