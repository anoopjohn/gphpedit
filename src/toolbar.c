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

GtkWidget *create_toolbar_stock_item(GtkWidget *toolitem,GtkWidget *toolbar,const gchar *stock_id, gchar *tooltip_text){
	toolitem = GTK_WIDGET(gtk_tool_button_new_from_stock(stock_id));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (toolitem), tooltip_text);
  	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (toolitem), -1);
	gtk_widget_show (toolitem);
        return toolitem;
}

void main_window_create_maintoolbar(void)
{
	
        // Create the Main Toolbar
        main_window.toolbar_main=g_slice_new(Maintoolbar);
	main_window.toolbar_main->toolbar = gtk_toolbar_new ();
	gtk_widget_show (main_window.toolbar_main->toolbar);
        gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_main->toolbar, FALSE, FALSE, 0);
        
	gtk_container_set_border_width (GTK_CONTAINER (main_window.toolbar_main->toolbar), 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (main_window.toolbar_main->toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (main_window.toolbar_main->toolbar), TRUE);


	// Add the File operations to the Main Toolbar
	main_window.toolbar_main->button_new= create_toolbar_stock_item(main_window.toolbar_main->button_new,main_window.toolbar_main->toolbar,GTK_STOCK_NEW, _("New File"));
	main_window.toolbar_main->button_open= create_toolbar_stock_item(main_window.toolbar_main->button_open,main_window.toolbar_main->toolbar,GTK_STOCK_OPEN, _("Open File"));
        main_window.toolbar_main->button_save= create_toolbar_stock_item(main_window.toolbar_main->button_save,main_window.toolbar_main->toolbar,GTK_STOCK_SAVE, _("Save current File"));
	main_window.toolbar_main->button_save_as= create_toolbar_stock_item(main_window.toolbar_main->button_save_as,main_window.toolbar_main->toolbar,GTK_STOCK_SAVE_AS, _("Save File As..."));
	main_window.toolbar_main->button_close= create_toolbar_stock_item(main_window.toolbar_main->button_close,main_window.toolbar_main->toolbar,GTK_STOCK_CLOSE, _("Close Current File"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_close), "clicked", G_CALLBACK (on_close1_activate), NULL);
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
	// Add the Undo operations to the Main Toolbar
        main_window.toolbar_main->button_undo= create_toolbar_stock_item(main_window.toolbar_main->button_undo,main_window.toolbar_main->toolbar,GTK_STOCK_UNDO, _("Undo last change"));
	main_window.toolbar_main->button_redo= create_toolbar_stock_item(main_window.toolbar_main->button_redo,main_window.toolbar_main->toolbar,GTK_STOCK_REDO, _("Redo last change"));
    g_signal_connect (G_OBJECT (main_window.toolbar_main->button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
    g_signal_connect (G_OBJECT (main_window.toolbar_main->button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));

	// Add the Clipboard operations to the Main Toolbar
        main_window.toolbar_main->button_cut= create_toolbar_stock_item(main_window.toolbar_main->button_cut,main_window.toolbar_main->toolbar,GTK_STOCK_CUT, _("Cut Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);
	main_window.toolbar_main->button_copy= create_toolbar_stock_item(main_window.toolbar_main->button_copy,main_window.toolbar_main->toolbar,GTK_STOCK_COPY, _("Copy Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);
	main_window.toolbar_main->button_paste= create_toolbar_stock_item(main_window.toolbar_main->button_paste,main_window.toolbar_main->toolbar,GTK_STOCK_PASTE, _("Paste text from clipboard"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
	// Add the Search operations to the Main Toolbar
        main_window.toolbar_main->button_find= create_toolbar_stock_item(main_window.toolbar_main->button_find,main_window.toolbar_main->toolbar,GTK_STOCK_FIND, _("Find text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);

        main_window.toolbar_main->button_replace= create_toolbar_stock_item(main_window.toolbar_main->button_replace,main_window.toolbar_main->toolbar,GTK_STOCK_FIND_AND_REPLACE, _("Replace Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));

	// Add the indent/unindent operations to the Main Toolbar	
	/*indent block*/
        main_window.toolbar_main->button_indent= create_toolbar_stock_item(main_window.toolbar_main->button_indent,main_window.toolbar_main->toolbar,GTK_STOCK_INDENT, _("Indent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_indent), "clicked", G_CALLBACK (block_indent), NULL);
	/*unindent block*/
        main_window.toolbar_main->button_unindent= create_toolbar_stock_item(main_window.toolbar_main->button_unindent,main_window.toolbar_main->toolbar,GTK_STOCK_UNINDENT, _("Unindent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);
	
	main_window.toolbar_main->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main->toolbar), GTK_TOOL_ITEM (main_window.toolbar_main->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_main->toolbar_separator));
        // Add Zoom operations to the main Toolbar
	//zoom in
        main_window.toolbar_main->button_zoom_in= create_toolbar_stock_item(main_window.toolbar_main->button_zoom_in,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_IN, _("Increases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
	//zoom out
        main_window.toolbar_main->button_zoom_out= create_toolbar_stock_item(main_window.toolbar_main->button_zoom_out,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_OUT, _("Decreases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
	//zoom 100%
        main_window.toolbar_main->button_zoom_100= create_toolbar_stock_item(main_window.toolbar_main->button_zoom_100,main_window.toolbar_main->toolbar,GTK_STOCK_ZOOM_100, _("Zoom 100%"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main->button_zoom_100), "clicked", G_CALLBACK (zoom_100), NULL);
	// Create the Search Toolbar

	/*sincronice with menu items*/
       if (gtk_toolbar_get_icon_size (GTK_TOOLBAR (main_window.toolbar_main->toolbar))==GTK_ICON_SIZE_SMALL_TOOLBAR){
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizesmall), TRUE);
       }else {
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizebig), TRUE);
       }
}

void main_window_create_findtoolbar(void){
        // Create the Main Toolbar
        main_window.toolbar_find=g_slice_new(Findtoolbar);
	main_window.toolbar_find->toolbar = gtk_toolbar_new ();
	gtk_widget_show (main_window.toolbar_find->toolbar);
        gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_find->toolbar, FALSE, FALSE, 0);
        
	gtk_container_set_border_width (GTK_CONTAINER (main_window.toolbar_find->toolbar), 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (main_window.toolbar_find->toolbar), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (main_window.toolbar_find->toolbar), TRUE);

	GtkToolItem *item;

	main_window.toolbar_find->search_label = gtk_label_new(_("Search for: "));
	gtk_widget_show(main_window.toolbar_find->search_label);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find->search_label);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	main_window.toolbar_find->search_entry = gtk_entry_new();
	gtk_widget_show(main_window.toolbar_find->search_entry);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find->search_entry);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (item), _("Incremental search"));
        gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window.toolbar_find->search_entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
        g_signal_connect (G_OBJECT (main_window.toolbar_find->search_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "insert_text", G_CALLBACK(inc_search_typed), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "key_release_event", G_CALLBACK(inc_search_key_release_event), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->search_entry), "activate", G_CALLBACK(inc_search_activate), NULL);
main_window.toolbar_find->toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (main_window.toolbar_find->toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_find->toolbar_separator));

	
	main_window.toolbar_find->goto_label = gtk_label_new(_("Go to line: "));
	gtk_widget_show(main_window.toolbar_find->goto_label);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find->goto_label);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	main_window.toolbar_find->goto_entry = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY(main_window.toolbar_find->goto_entry),8);
	gtk_entry_set_width_chars(GTK_ENTRY(main_window.toolbar_find->goto_entry),9);
        gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window.toolbar_find->goto_entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
       g_signal_connect (G_OBJECT (main_window.toolbar_find->goto_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
	gtk_widget_show(main_window.toolbar_find->goto_entry);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find->goto_entry);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (item), _("Go to line"));
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find->toolbar), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	g_signal_connect_after(G_OBJECT(main_window.toolbar_find->goto_entry), "activate", G_CALLBACK(goto_line_activate), NULL);
}




