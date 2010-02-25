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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit. If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "stdlib.h"
#include "main_window.h"
#include "tab.h"
#include "main_window_callbacks.h"
#include "preferences.h"
#include "classbrowser.h"
#include "plugin.h"
#include "templates.h"

MainWindow main_window;
GIOChannel* inter_gphpedit_io;
guint inter_gphpedit_event_id;
gboolean DEBUG_MODE = FALSE;

void create_untitled_if_empty(void)
{
	if (g_slist_length(editors) == 0) {
		tab_create_new(TAB_FILE, NULL);
	}
}


void main_window_open_command_line_files(char **argv, gint argc)
{
	guint i;

	if (argc>1) {
		i = 1;
		while (argv[i] != NULL) {
			switch_to_file_or_open(argv[i],0);
			
			++i;
		}
	}
}


void main_window_pass_command_line_files(char **argv)
{
	guint i;
	GError *error;
	gsize bytes_written;

	error = NULL;
	inter_gphpedit_io = g_io_channel_new_file("/tmp/gphpedit.sock","w",&error);
	if (argv) {
		i = 1;
		while (argv[i] != NULL) {
			//g_print("%s:%d\n", argv[i], strlen(argv[i]));
			g_io_channel_write_chars(inter_gphpedit_io, argv[i], strlen(argv[i]),
									 &bytes_written, &error);
			++i;
		}
	}
}


gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data )
{
	guint size;
	gchar buf[1024];
        GError *error=NULL;
        if (g_io_channel_read_chars (inter_gphpedit_io,buf,sizeof(buf), &size,&error)!=G_IO_STATUS_NORMAL){
            g_print("Error reading GIO Chanel. Error:%s\n",error->message);
        }
	g_print("Passed %s\n", buf);
	tab_create_new(TAB_FILE, g_string_new(buf));
	return FALSE;
}

void force_config_folder(void)
{
        GFile *config;
        GError *error;
        error=NULL;
        GString *uri;
        uri = g_string_new ("");
        g_string_printf(uri,"%s/%s",g_get_home_dir(),".gphpedit");
        config=g_file_new_for_path (uri->str);
        g_string_free (uri, TRUE);
        if (!g_file_make_directory (config, NULL, &error)){
            if (error->code !=2){
            g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code,error->message);
            exit(-1);
            }
        }
        error=NULL;
        uri = g_string_new ("");
        g_string_printf(uri,"%s/%s/%s",g_get_home_dir(),".gphpedit","plugins");
        config=g_file_new_for_path (uri->str);
        g_string_free (uri, TRUE);
        if (!g_file_make_directory (config, NULL, &error)){
            //if error code = 2 dir already exist
            if (error->code !=2){
            g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code,error->message);
            exit(-1);
            }
        }
	g_object_unref(config);
}

void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
    gtk_entry_set_text (entry,"");
}
GtkWidget *create_toolbar_stock_item(GtkWidget *toolitem,GtkWidget *toolbar,const gchar *stock_id, gchar *tooltip_text){
	toolitem = GTK_WIDGET(gtk_tool_button_new_from_stock(stock_id));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (toolitem), tooltip_text);
  	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM (toolitem), -1);
	gtk_widget_show (toolitem);
        return toolitem;
}

static void main_window_create_toolbars(void)
{
	
        // Create the Main Toolbar
	main_window.toolbar_main = gtk_toolbar_new ();
	gtk_widget_show (main_window.toolbar_main);
        gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_main, FALSE, FALSE, 0);
        
	gtk_container_set_border_width (GTK_CONTAINER (main_window.toolbar_main), 0);
	gtk_toolbar_set_style (GTK_TOOLBAR (main_window.toolbar_main), GTK_TOOLBAR_ICONS);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (main_window.toolbar_main), TRUE);


	// Add the File operations to the Main Toolbar
	main_window.toolbar_main_button_new= create_toolbar_stock_item(main_window.toolbar_main_button_new,main_window.toolbar_main,GTK_STOCK_NEW, _("New File"));
	main_window.toolbar_main_button_open= create_toolbar_stock_item(main_window.toolbar_main_button_open,main_window.toolbar_main,GTK_STOCK_OPEN, _("Open File"));
        main_window.toolbar_main_button_save= create_toolbar_stock_item(main_window.toolbar_main_button_save,main_window.toolbar_main,GTK_STOCK_SAVE, _("Save current File"));
	main_window.toolbar_main_button_save_as= create_toolbar_stock_item(main_window.toolbar_main_button_save_as,main_window.toolbar_main,GTK_STOCK_SAVE_AS, _("Save File As..."));
	main_window.toolbar_main_button_close= create_toolbar_stock_item(main_window.toolbar_main_button_close,main_window.toolbar_main,GTK_STOCK_CLOSE, _("Close Current File"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_close), "clicked", G_CALLBACK (on_close1_activate), NULL);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	// Add the Undo operations to the Main Toolbar
        main_window.toolbar_main_button_undo= create_toolbar_stock_item(main_window.toolbar_main_button_undo,main_window.toolbar_main,GTK_STOCK_UNDO, _("Undo last change"));
	main_window.toolbar_main_button_redo= create_toolbar_stock_item(main_window.toolbar_main_button_redo,main_window.toolbar_main,GTK_STOCK_REDO, _("Redo last change"));
        g_signal_connect (G_OBJECT (main_window.toolbar_main_button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));

	// Add the Clipboard operations to the Main Toolbar
        main_window.toolbar_main_button_cut= create_toolbar_stock_item(main_window.toolbar_main_button_cut,main_window.toolbar_main,GTK_STOCK_CUT, _("Cut Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);
	main_window.toolbar_main_button_copy= create_toolbar_stock_item(main_window.toolbar_main_button_copy,main_window.toolbar_main,GTK_STOCK_COPY, _("Copy Current Selection"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);
	main_window.toolbar_main_button_paste= create_toolbar_stock_item(main_window.toolbar_main_button_paste,main_window.toolbar_main,GTK_STOCK_PASTE, _("Paste text from clipboard"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);
	
	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	// Add the Search operations to the Main Toolbar
        main_window.toolbar_main_button_find= create_toolbar_stock_item(main_window.toolbar_main_button_find,main_window.toolbar_main,GTK_STOCK_FIND, _("Find text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);

        main_window.toolbar_main_button_replace= create_toolbar_stock_item(main_window.toolbar_main_button_replace,main_window.toolbar_main,GTK_STOCK_FIND_AND_REPLACE, _("Replace Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);
	
	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));

	// Add the indent/unindent operations to the Main Toolbar	
	/*indent block*/
        main_window.toolbar_main_button_indent= create_toolbar_stock_item(main_window.toolbar_main_button_indent,main_window.toolbar_main,GTK_STOCK_INDENT, _("Indent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_indent), "clicked", G_CALLBACK (block_indent), NULL);
	/*unindent block*/
        main_window.toolbar_main_button_unindent= create_toolbar_stock_item(main_window.toolbar_main_button_unindent,main_window.toolbar_main,GTK_STOCK_UNINDENT, _("Unindent Selected Text"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);
	
	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
        // Add Zoom operations to the main Toolbar
	//zoom in
        main_window.toolbar_main_button_zoom_in= create_toolbar_stock_item(main_window.toolbar_main_button_zoom_in,main_window.toolbar_main,GTK_STOCK_ZOOM_IN, _("Increases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
	//zoom out
        main_window.toolbar_main_button_zoom_out= create_toolbar_stock_item(main_window.toolbar_main_button_zoom_out,main_window.toolbar_main,GTK_STOCK_ZOOM_OUT, _("Decreases Zoom level"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
	//zoom 100%
        main_window.toolbar_main_button_zoom_100= create_toolbar_stock_item(main_window.toolbar_main_button_zoom_100,main_window.toolbar_main,GTK_STOCK_ZOOM_100, _("Zoom 100%"));
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_zoom_100), "clicked", G_CALLBACK (zoom_100), NULL);
	// Create the Search Toolbar
	
	GtkToolItem *item;

	main_window.toolbar_find = gtk_toolbar_new ();
	gtk_widget_show (main_window.toolbar_find);
	gtk_toolbar_set_show_arrow (GTK_TOOLBAR (main_window.toolbar_find), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (main_window.toolbar_find), 0);
        gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_find, FALSE, FALSE, 0);

	main_window.toolbar_find_search_label = gtk_label_new(_("Search for: "));
	gtk_widget_show(main_window.toolbar_find_search_label);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find_search_label);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	main_window.toolbar_find_search_entry = gtk_entry_new();
	gtk_widget_show(main_window.toolbar_find_search_entry);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find_search_entry);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (item), _("Incremental search"));
        gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window.toolbar_find_search_entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
        g_signal_connect (G_OBJECT (main_window.toolbar_find_search_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	g_signal_connect_after(G_OBJECT(main_window.toolbar_find_search_entry), "insert_text", G_CALLBACK(inc_search_typed), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find_search_entry), "key_release_event", G_CALLBACK(inc_search_key_release_event), NULL);
	g_signal_connect_after(G_OBJECT(main_window.toolbar_find_search_entry), "activate", G_CALLBACK(inc_search_activate), NULL);
	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	
	main_window.toolbar_find_goto_label = gtk_label_new(_("Go to line: "));
	gtk_widget_show(main_window.toolbar_find_goto_label);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find_goto_label);
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	main_window.toolbar_find_goto_entry = gtk_entry_new();
        gtk_entry_set_max_length (GTK_ENTRY(main_window.toolbar_find_goto_entry),8);
	gtk_entry_set_width_chars(GTK_ENTRY(main_window.toolbar_find_goto_entry),9);
        gtk_entry_set_icon_from_stock (GTK_ENTRY(main_window.toolbar_find_goto_entry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
        g_signal_connect (G_OBJECT (main_window.toolbar_find_goto_entry), "icon-press", G_CALLBACK (on_cleanicon_press), NULL);
	gtk_widget_show(main_window.toolbar_find_goto_entry);
	item=gtk_tool_item_new();
	gtk_tool_item_set_expand (item, FALSE);
	gtk_container_add (GTK_CONTAINER (item), main_window.toolbar_find_goto_entry);
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (item), _("Go to line"));
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_find), GTK_TOOL_ITEM (item), -1);
	gtk_widget_show(GTK_WIDGET(item));

	g_signal_connect_after(G_OBJECT(main_window.toolbar_find_goto_entry), "activate", G_CALLBACK(goto_line_activate), NULL);
	/*sincronice with menu items*/
       if (gtk_toolbar_get_icon_size (GTK_TOOLBAR (main_window.toolbar_main))==GTK_ICON_SIZE_SMALL_TOOLBAR){
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizesmall), TRUE);
       }else {
       gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (main_window.menu->sizebig), TRUE);
       }
}


static void main_window_create_appbar(void)
{
    main_window.appbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.appbar, FALSE, TRUE, 1);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (main_window.appbar), FALSE);
    GtkWidget *box;
    box = gtk_hbox_new(FALSE, 0);
    main_window.zoomlabel=gtk_label_new(_("Zoom:100%"));
    gtk_widget_show (main_window.zoomlabel);
    gtk_box_pack_start(GTK_BOX(box), main_window.zoomlabel, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(main_window.appbar), box, FALSE, FALSE, 25);
    gtk_widget_show (box);
    gtk_widget_show (main_window.appbar);
}

static void main_window_create_panes(void)
{
	main_window.main_vertical_pane = gtk_vpaned_new ();
	gtk_widget_show (main_window.main_vertical_pane);
        gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.main_vertical_pane, TRUE, TRUE, 0);
	main_window.main_horizontal_pane = gtk_hpaned_new ();
	gtk_widget_show (main_window.main_horizontal_pane);
	gtk_paned_pack1 (GTK_PANED (main_window.main_vertical_pane), main_window.main_horizontal_pane, FALSE, TRUE);

	g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (classbrowser_accept_size), NULL);
	move_classbrowser_position();
        GConfClient *config;
        config=gconf_client_get_default ();
        
	if (gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_hidden",NULL) == 1)
		classbrowser_hide();
        g_object_unref(config);
}

static void main_window_fill_panes(void)
{
	GtkWidget *box;
	GtkWidget *box2;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	gint pos;

        GtkWidget *notebook_manager;
	notebook_manager = gtk_notebook_new ();
	main_window.notebook_manager= gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (main_window.notebook_manager), GTK_POS_BOTTOM);
	gtk_widget_set_size_request (main_window.notebook_manager, 200,400);
	gtk_widget_show (main_window.notebook_manager);
	box = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(box);
//	gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), box, FALSE, TRUE);
	gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), main_window.notebook_manager, FALSE, TRUE);

	main_window.classlabel = gtk_label_new ("Class Browser");
	gtk_widget_show(main_window.classlabel);
	pos=gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), box, main_window.classlabel, 0);

	//Close button for the side bar
	GtkWidget *hbox;
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_container_set_border_width(GTK_CONTAINER(hbox), 0);
	main_window.close_image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
	gtk_misc_set_padding(GTK_MISC(main_window.close_image), 0, 0);
	main_window.close_sidebar_button = gtk_button_new();
	gtk_widget_set_tooltip_text(main_window.close_sidebar_button, _("Close class Browser"));
	gtk_button_set_image(GTK_BUTTON(main_window.close_sidebar_button), main_window.close_image);
	gtk_button_set_relief(GTK_BUTTON(main_window.close_sidebar_button), GTK_RELIEF_NONE);
	gtk_button_set_focus_on_click(GTK_BUTTON(main_window.close_sidebar_button), FALSE);
	g_signal_connect(G_OBJECT(main_window.close_sidebar_button), "clicked", G_CALLBACK (classbrowser_show_hide),NULL);
	gtk_widget_show(main_window.close_image);
	gtk_widget_show(main_window.close_sidebar_button);
	gtk_box_pack_end(GTK_BOX(hbox), main_window.close_sidebar_button, FALSE, FALSE, 0);
	gtk_widget_show(hbox);
	//add checkbox to show only current file's classes
	//the signals to be checked for the check box are onclick of the checkbox
	//and the on change of the file.
	main_window.chkOnlyCurFileFuncs = gtk_check_button_new_with_label(_("Parse only current file"));
	gtk_widget_show (main_window.chkOnlyCurFileFuncs);
	gtk_box_pack_start(GTK_BOX(hbox), main_window.chkOnlyCurFileFuncs, TRUE, TRUE, 10);
//	gtk_box_pack_start(GTK_BOX(box), main_window.chkOnlyCurFileFuncs, FALSE, FALSE, 10);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 10);
	g_signal_connect (G_OBJECT (main_window.chkOnlyCurFileFuncs), "clicked",
						G_CALLBACK (on_parse_current_click), NULL);

	main_window.scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
	gtk_widget_show (main_window.scrolledwindow3);
	gtk_box_pack_start(GTK_BOX(box), main_window.scrolledwindow3, TRUE, TRUE, 0);
	//gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), main_window.scrolledwindow3, FALSE, TRUE);

	box2 = gtk_hbox_new(FALSE, 0);
	gtk_widget_show(box2);
	main_window.treeviewlabel = gtk_label_new(_("FILE: "));
	gtk_label_set_justify(GTK_LABEL(main_window.treeviewlabel), GTK_JUSTIFY_LEFT);
	gtk_widget_show(main_window.treeviewlabel);
	gtk_box_pack_start(GTK_BOX(box2), main_window.treeviewlabel, FALSE, FALSE, 0);
	gtk_box_pack_end(GTK_BOX(box), box2, FALSE, FALSE, 4);
	
	//gtk_container_add (GTK_CONTAINER (notebook_manager), main_window.scrolledwindow3);

	main_window.classtreestore = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING,
															 G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
	//enable sorting of the columns
	classbrowser_set_sortable(main_window.classtreestore);

	main_window.classtreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (main_window.classtreestore));
	gtk_widget_show (main_window.classtreeview);
	gtk_container_add (GTK_CONTAINER (main_window.scrolledwindow3), main_window.classtreeview);

	main_window.classtreeselect = gtk_tree_view_get_selection (GTK_TREE_VIEW (main_window.classtreeview));
	gtk_tree_selection_set_mode (main_window.classtreeselect, GTK_SELECTION_SINGLE);
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes (_("Name"),
					 renderer, "text", NAME_COLUMN, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (main_window.classtreeview), column);

	/*label1 = gtk_label_new ("Functions");
	gtk_widget_show (label1);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_manager), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_manager), 0), label1);
	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
	 
	notebook_manager_functions_page = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (notebook_manager_functions_page);
	gtk_container_add (GTK_CONTAINER (notebook_manager), notebook_manager_functions_page);

	label2 = gtk_label_new ("Files");
	gtk_widget_show (label2);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_manager), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_manager), 1), label2);
	gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);

	notebook_manager_files_page = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (notebook_manager_files_page);
	gtk_container_add (GTK_CONTAINER (notebook_manager), notebook_manager_files_page);

	label3 = gtk_label_new ("Help");
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook_manager), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook_manager), 2), label3);
	gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
	*/
        main_window.scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
	gtk_paned_pack2 (GTK_PANED (main_window.main_vertical_pane), main_window.scrolledwindow1, FALSE, TRUE);
        main_window.lint_view = gtk_tree_view_new ();
	gtk_container_add (GTK_CONTAINER (main_window.scrolledwindow1), main_window.lint_view);
	main_window.lint_renderer = gtk_cell_renderer_text_new ();
	main_window.lint_column = gtk_tree_view_column_new_with_attributes (_("Syntax Check Output"),
								main_window.lint_renderer,
								"text", 0,
								NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (main_window.lint_view), main_window.lint_column);
	//gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (main_window.lint_view), FALSE);
        gtk_widget_set_size_request (main_window.lint_view, 80,80);
	main_window.lint_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (main_window.lint_view));
	gtk_tree_selection_set_mode (main_window.lint_select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (main_window.lint_select), "changed",
						G_CALLBACK (lint_row_activated), NULL);

	main_window.notebook_editor = gtk_notebook_new ();
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(main_window.notebook_editor), TRUE);
	//GTK_WIDGET_UNSET_FLAGS (main_window.notebook_editor, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
	// Fix to scrollable list of tabs, however it then messes up grabbing of focus
	// Hence the focus-tab event (which GTK doesn't seem to recognise
	GTK_WIDGET_UNSET_FLAGS (main_window.notebook_editor, GTK_RECEIVES_DEFAULT);
	gtk_widget_show (main_window.notebook_editor);
	gtk_paned_pack2 (GTK_PANED (main_window.main_horizontal_pane), main_window.notebook_editor, TRUE, TRUE);
        gtk_widget_set_size_request (main_window.notebook_editor, 400,400);
        g_signal_connect (G_OBJECT (main_window.notebook_editor), "switch_page", G_CALLBACK (on_notebook_switch_page), NULL);
	g_signal_connect (G_OBJECT (main_window.notebook_editor), "focus-tab", G_CALLBACK (on_notebook_focus_tab), NULL);
}

/**
 * Compare two filenames and find the length of the part of the
 * directory names that match each other. Eg: passing ./home/src/a.php
 * and ./home/b.php will return 7 i.e. the length of the common
 * part of the directory names.
 */
guint get_longest_matching_length(gchar *filename1, gchar *filename2)
{
	gchar *base1, *base1_alloc;
	gchar *base2, *base2_alloc;
	guint length;

	//Store the pointers so as to be freed in the end.
	base1 = g_path_get_dirname(filename1);
	base1_alloc = base1;
	base2 = g_path_get_dirname(filename2);
	base2_alloc = base2;

	length = 0;
	//Check only if both base paths are not ".".
	if (strcmp(base2_alloc, ".")!=0 && strcmp(base2_alloc, ".")!=0) {
		//Increment count and move along the characters in both paths
		//while they are equal and compare till the shorter of the two.
		while (*base1 && *base2 && (*base1 == *base2)) {
			base1++;
			base2++;
			length++;
		}
	}

	g_free(base1_alloc);
	g_free(base2_alloc);

	return length;
}

/**
 * 
 */
GString *get_differing_part(GSList *filenames, gchar *file_requested)
{
	GSList *temp_list;
	gchar buffer[1024];
	guint longest_match;
	guint match;

	longest_match = 9999;

	// Loop through and find the length of the shortest matching basepath
	// Seems to miss the first one - if that's not required, change to temp_list = filenames
	for(temp_list = filenames; temp_list!= NULL; temp_list = g_slist_next(temp_list)) {
		match = get_longest_matching_length(temp_list->data, file_requested);
		//debug("String: %s\nString: %s\nMatch: %d", temp_list->data, file_requested, match);
		if (match < longest_match) {
			longest_match = match;
		}
	}
	//debug("Match: %d", longest_match);
	if (longest_match!=9999) {
		if (*(file_requested + longest_match) == '/') {
			strcpy(buffer, (file_requested + longest_match+1));
		}
		else {
			strcpy(buffer, (file_requested + longest_match));
		}
	}
	else {
		strcpy(buffer, file_requested);
	}

	return g_string_new(buffer);
}

GString *get_differing_part_editor(Editor *editor)
{
	gchar *cwd;
	GSList *list_editors;
	GSList *list_filenames;
	Editor *data;
	gchar *str;
	GString *result;

	if (editor == NULL) 
		return NULL;
	
	cwd = g_get_current_dir();

	list_filenames = NULL;
	list_filenames = g_slist_append(list_filenames, cwd);

	for(list_editors = editors; list_editors!= NULL; list_editors = g_slist_next(list_editors)) {
		data = list_editors->data;
		if (data->type == TAB_FILE) {
			str = ((Editor *)data)->filename->str;
			list_filenames = g_slist_append(list_filenames, str);
		}
	}

	result = get_differing_part(list_filenames, editor->filename->str);
	g_free(cwd);
	return result;
}
/**
 * Update the application title when switching tabs, closing or opening
 * or opening new tabs or opening new files.
 */
void update_app_title(void)
{
	GString *title;
	gchar *dir;
	//debug("Function called");
	if (main_window.current_editor != NULL) {
		//debug("Not null");
		if (main_window.current_editor->type != TAB_HELP
		    && main_window.current_editor->filename) {
			//debug("Full Name - %s, Short Name - %s", main_window.current_editor->filename->str, main_window.current_editor->short_filename);
			//title = get_differing_part_editor(main_window.current_editor);
			title = g_string_new("");
			title = g_string_append(title, main_window.current_editor->short_filename);
			title = g_string_append(title, "(");
			dir = g_path_get_dirname(main_window.current_editor->filename->str);
			title = g_string_append(title, dir);
			g_free(dir);
			title = g_string_append(title, ")");
			//debug("Title - %s, Short name - %s", title->str, main_window.current_editor->short_filename);
			if (main_window.current_editor->saved == TRUE) {
				g_string_append(title, _(" - gPHPEdit"));
			}
			//If the content is not saved then add a * to the begining of the title
			else {
				g_string_prepend(title, "*");
				g_string_append(title, _(" - gPHPEdit"));
			}
                        update_zoom_level();
                        update_controls();
		}
		else if(main_window.current_editor->type == TAB_HELP) {
			title = g_string_new("Help: ");
			title = g_string_append(title, main_window.current_editor->help_function);
                        update_zoom_level();
                        update_controls();
		}
		//If there is no file opened set the name as gPHPEdit
		else {
			title = g_string_new(_("gPHPEdit"));
		}
	}
	//If there is no file opened set the name as gPHPEdit
	else {
		title = g_string_new(_("gPHPEdit"));
	}
	gtk_window_set_title(GTK_WINDOW(main_window.window), title->str);
	g_string_free(title, TRUE);
}

gint minimum(gint val1, gint val2)
{
	if (val1 < val2) {
		return val1;
	}
	return val2;
}


gint maximum(gint val1, gint val2)
{
	if (val1 > val2) {
		return val1;
	}
	return val2;
}

GList *Plugins = NULL;

/****/

int plugin_discover_type(GString *filename)
{
	GString *command_line;
	gchar *stdout = NULL;
	GError *error = NULL;
	gint exit_status;
	gint type = GPHPEDIT_PLUGIN_TYPE_UNKNOWN;
	gint stdout_len;
	
	command_line = g_string_new(filename->str);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' -type");
	
	if (g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) {
		stdout_len = strlen(stdout);
		//g_print("------------------------------------\nDISCOVERY\nCOMMAND: %s\nOUTPUT: %s (%d)\n", command_line->str, stdout, stdout_len);
		
		if (strncmp(stdout, "SELECTION", MIN(stdout_len, 9))==0) {
			type = GPHPEDIT_PLUGIN_TYPE_SELECTION;
		}
		else if (strncmp(stdout, "NO-INPUT", MIN(stdout_len, 8))==0) {
			type = GPHPEDIT_PLUGIN_TYPE_NOINPUT;
		}
		else if (strncmp(stdout, "FNAME", MIN(stdout_len, 5))==0) {
			type = GPHPEDIT_PLUGIN_TYPE_FILENAME;
		}
		else if (strncmp(stdout, "DEBUG", MIN(stdout_len, 5))==0) {
			type = GPHPEDIT_PLUGIN_TYPE_DEBUG;
		}
		
		//g_print("Returning Discovered type of %d\n------------------------------------\n", type);
		g_free(stdout);

	}
	else {
		g_print("Spawning %s gave error %s\n", filename->str, error->message);
	}	
	
	g_string_free(command_line, TRUE);
	
	return type;
}

gint sort_plugin_func(gconstpointer a, gconstpointer b)
{
	Plugin *plugina = (Plugin *)a;
	Plugin *pluginb = (Plugin *)b;

	if (strcmp(plugina->name, pluginb->name) < 0) {
		return -1;
	}
	return 1;
}

void plugin_discover_available(void)
{
	GDir *dir;
	const gchar *plugin_name;
	Plugin *plugin;
	GString *user_plugin_dir;
	GString *filename;
	
	user_plugin_dir = g_string_new( g_get_home_dir());
	user_plugin_dir = g_string_append(user_plugin_dir, "/.gphpedit/plugins/");
	//g_print("User plugin dir: %s\n", user_plugin_dir->str);
	if (g_file_test(user_plugin_dir->str, G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open(user_plugin_dir->str, 0,NULL);
		if (dir) {
			for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				// Recommended by __tim in #gtk+ on irc.freenode.net 27/10/2004 11:30
				plugin = g_new0 (Plugin, 1);
				plugin->name = g_strdup(plugin_name);
				// TODO: Could do with replacing ' in name with \' for spawn
				filename = g_string_new(plugin->name);
				filename = g_string_prepend(filename, user_plugin_dir->str);
				plugin->filename = filename;
				//g_print ("PLUGIN FILENAME: %s\n", plugin->filename->str);
				plugin->type = plugin_discover_type(plugin->filename);
				Plugins = g_list_append(Plugins, plugin);
				//g_print("%s\n", plugin_name);
			}
			g_dir_close(dir);			
		}
	}
	g_string_free(user_plugin_dir, TRUE);

	if (g_file_test("/usr/share/gphpedit/plugins/", G_FILE_TEST_IS_DIR)) {
		dir = g_dir_open("/usr/share/gphpedit/plugins/", 0,NULL);
		if (dir) {
			for (plugin_name = g_dir_read_name(dir); plugin_name != NULL; plugin_name = g_dir_read_name(dir)) {
				// Recommended by __tim in #gtk+ on irc.freenode.net 27/10/2004 11:30
				plugin = g_new0 (Plugin, 1);
				plugin->name = g_strdup(plugin_name);
				filename = g_string_new(plugin_name);
				filename = g_string_prepend(filename, "/usr/share/gphpedit/plugins/");
				plugin->filename = filename;
				//g_print ("PLUGIN FILENAME: %s\n", plugin->filename->str);
				plugin->type = plugin_discover_type(plugin->filename);
				Plugins = g_list_append(Plugins, plugin);
				//g_print("%s\n", plugin_name);
			}
			g_dir_close(dir);			
		}
	}

	Plugins = g_list_sort(Plugins, sort_plugin_func);

	//g_print ("FOUND ALL PLUGINS\n");
}

void plugin_create_menu_items()
{
	GList *iterator;
	Plugin *plugin;
	guint num_plugin;
	guint hide_plugin;

	GtkBin *bin = NULL;
	GtkLabel *label;
	
	num_plugin = 0;
	for (iterator = Plugins; iterator != NULL && num_plugin<NUM_PLUGINS_MAX; iterator = g_list_next(iterator)) {
		plugin = (Plugin *)(iterator->data);
		//g_print ("Plugin %d:%s\n", num_plugin, plugin->filename);
		
		//g_print("Getting child widget\n");
                
                bin = GTK_BIN(main_window.menu->plugins[num_plugin]);
		//g_print("Bin is %p\n", bin);
		if (bin) {
			label = GTK_LABEL(gtk_bin_get_child(bin));
		
			gtk_label_set_text(label, plugin->name);

                        gtk_widget_show(&main_window.menu->plugin[num_plugin]);
		}
		
		num_plugin++;
	}

	//g_print("Blanking all non-found plugin entries\n");
        for (hide_plugin=num_plugin; hide_plugin <NUM_PLUGINS_MAX; hide_plugin++) {
            gtk_widget_hide(main_window.menu->plugins[hide_plugin]);
	}
}
void plugin_exec(gint plugin_num)
{
	Plugin *plugin;
	gchar *stdout = NULL;
	GError *error = NULL;
	gint exit_status;
	GString *command_line = NULL;
	gint wordStart;
	gint wordEnd;
	gchar *current_selection;
	gint ac_length;
	gchar *data;
	
	if (main_window.current_editor == NULL) {
		return;
	}
	
	plugin = (Plugin *)g_list_nth_data(Plugins, plugin_num);
	if (!plugin) {
		g_print(_("Plugin is null!\n"));
	}
	//g_print("Plugin No: %d:%d (%s):%s\n", plugin_num, plugin->type, plugin->name, plugin->filename->str);
	command_line = g_string_new(plugin->filename->str);
	command_line = g_string_prepend(command_line, "'");
	command_line = g_string_append(command_line, "' \"");
	if (plugin->type == GPHPEDIT_PLUGIN_TYPE_SELECTION) {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		current_selection = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);
		command_line = g_string_append(command_line, g_strescape(current_selection,""));
	}
	else if (plugin->type == GPHPEDIT_PLUGIN_TYPE_FILENAME) {
		command_line = g_string_append(command_line, editor_convert_to_local(main_window.current_editor));		
	}
	command_line = g_string_append(command_line, "\"");
	//g_print("SPAWNING: %s\n", command_line->str);
	
	if (g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) {
		data = strstr(stdout, "\n");
		data++;
		
		//g_print("COMMAND: %s\nSTDOUT:%s\nOUTPUT: %s\n", command_line->str, stdout, data);
		
		if (g_ascii_strncasecmp(stdout, "INSERT", MIN(strlen(stdout), 6))==0) {
			if (data) {
				gtk_scintilla_insert_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 
					gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla)), data);
			}
		}
		else if (g_ascii_strncasecmp(stdout, "REPLACE", MIN(strlen(stdout), 7))==0) {
			if (data) {
				gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), data);
			}
		}
		else if (g_ascii_strncasecmp(stdout, "MESSAGE", MIN(strlen(stdout),7))==0) {
				info_dialog(plugin->name, data);
		}
		else if (g_ascii_strncasecmp(stdout, "OPEN", MIN(strlen(stdout), 4))==0) {
			if (DEBUG_MODE) { g_print("DEBUG: main_window.c:plugin_exec: Opening file :date: %s\n", data); }
			switch_to_file_or_open(data, 0);
		}
		else if (g_ascii_strncasecmp(stdout, "DEBUG", MIN(strlen(stdout), 5))==0) {
			debug_dump_editors();
			DEBUG_MODE = TRUE;
		}
		
		g_free(stdout);

	}
	else {
		g_print(_("Spawning %s gave error %s\n"), plugin->filename->str, error->message);
	}
}


void plugin_setup_menu(void)
{
    
	plugin_discover_available();
	if (g_list_length(Plugins)>0) {
		plugin_create_menu_items();
	}
	else {
              gtk_widget_hide(main_window.menu->plugin);
       }
    
}

/****/

void main_window_update_reopen_menu(void)
{
    
	gchar *full_filename;
	GString *key;
	//gchar *short_filename;
	guint entry;
	GtkBin *bin = NULL;
        GConfClient *config;
        config=gconf_client_get_default ();
	for (entry=0; entry<NUM_REOPEN_MAX; entry++) {
		key = g_string_new("/gPHPEdit/recent/");
		g_string_append_printf(key, "%d", entry);
		full_filename = gconf_client_get_string(config,key->str,NULL);
		g_string_free(key, TRUE);
		
		//g_print("Recent DEBUG: Entry %d: %s\n", entry, full_filename);
                if (full_filename){
                    bin = GTK_BIN(main_window.menu->recent[entry]);
			if (bin) {
				gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(bin)), full_filename);
                                gtk_widget_show(main_window.menu->recent[entry]);
			}
		}
		else {
                    gtk_widget_hide(main_window.menu->recent[entry]);
		}
	}
     
                
}

void main_window_add_to_reopen_menu(gchar *full_filename)
{
	guint entry;
	gchar *found;
	GString *key;
	guint found_id;
	GConfClient *config;
        config=gconf_client_get_default ();
	// Find current filename in list
	found_id = -1;
	for (entry=0; entry<NUM_REOPEN_MAX; entry++) {
		key = g_string_new("/gPHPEdit/recent/");
		g_string_append_printf(key, "%d", entry);
		found = gconf_client_get_string(config,key->str,NULL);
		g_string_free(key, TRUE);
                if (found){
		if (strcmp(full_filename, found)==0) {
			found_id = entry;
			break;
		}
               }
	}
	
	// if not found, drop the last one off the end (i.e. pretend it was found in the last position)
	if (found_id == -1) {
		found_id = NUM_REOPEN_MAX-1;
	}

	// replace from found_id to 1 with entry above
	for (entry=found_id; entry > 0; entry--) {
		key = g_string_new("/gPHPEdit/recent/");
		g_string_append_printf(key, "%d", entry-1);
		found = gconf_client_get_string(config,key->str,NULL);
		g_string_free(key, TRUE);

		key = g_string_new("/gPHPEdit/recent/");
		g_string_append_printf(key, "%d", entry);
		if (found){
                gconf_client_set_string (config,key->str, found,NULL);
                }
		g_string_free(key, TRUE);
	}

	// set entry 0 to be new entry
        gconf_client_set_string (config,"/gPHPEdit/recent/0", full_filename,NULL);
		
	main_window_update_reopen_menu();
}

void main_window_create(void)
{
        main_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(main_window.window), _("gPHPEdit"));
        gtk_window_set_default_size(GTK_WINDOW(main_window.window), 230, 150);
        gtk_window_set_position(GTK_WINDOW(main_window.window), GTK_WIN_POS_CENTER);
        gtk_window_set_icon(GTK_WINDOW(main_window.window), get_window_icon());
        
	preferences_apply();
        main_window_create_menu();
        main_window_create_toolbars();
        main_window_create_panes();
        main_window_fill_panes();
        main_window_create_appbar();
	main_window_update_reopen_menu();

	plugin_setup_menu();
	function_list_prepare();

	g_signal_connect (G_OBJECT (main_window.window), "delete_event", G_CALLBACK(main_window_delete_event), NULL);
	g_signal_connect (G_OBJECT (main_window.window), "destroy", G_CALLBACK (main_window_destroy_event), NULL);
	g_signal_connect (G_OBJECT (main_window.window), "key_press_event", G_CALLBACK (main_window_key_press_event), NULL);
	g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (main_window_resize), NULL);
	g_signal_connect (G_OBJECT (main_window.window), "window-state-event", G_CALLBACK (main_window_state_changed), NULL);

	main_window.clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	gtk_widget_show(main_window.window);

	update_app_title();
	// folder browser init
	folderbrowser_create(&main_window);
}

void update_controls(void){
if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    //activate toolbar items
    gtk_widget_set_sensitive (main_window.toolbar_main_button_cut, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_paste, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_undo, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_redo, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_replace, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_indent, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main_button_unindent, TRUE);
    if (main_window.current_editor->isreadonly){
        gtk_widget_set_sensitive (main_window.toolbar_main_button_save, FALSE);
    } else {
        gtk_widget_set_sensitive (main_window.toolbar_main_button_save, TRUE);
    }
    
    gtk_widget_set_sensitive (main_window.toolbar_main_button_save_as, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_find_search_entry, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_find_goto_entry, TRUE);
    //activate menu items
    gtk_widget_set_sensitive (main_window.menu->code, TRUE);
    gtk_widget_set_sensitive (main_window.menu->cut, TRUE);
    gtk_widget_set_sensitive (main_window.menu->paste, TRUE);
    if (main_window.current_editor->isreadonly){
        gtk_widget_set_sensitive (main_window.menu->save, FALSE);
    } else {
        gtk_widget_set_sensitive (main_window.menu->save, TRUE);
    }
    gtk_widget_set_sensitive (main_window.menu->saveas, TRUE);
    gtk_widget_set_sensitive (main_window.menu->reload, TRUE);
    gtk_widget_set_sensitive (main_window.menu->rename, TRUE);
    gtk_widget_set_sensitive (main_window.menu->indent, TRUE);
    gtk_widget_set_sensitive (main_window.menu->unindent, TRUE);
    gtk_widget_set_sensitive (main_window.menu->replace, TRUE);
    gtk_widget_set_sensitive (main_window.menu->plugin, TRUE);
    gtk_widget_set_sensitive (main_window.menu->undo, TRUE);
    gtk_widget_set_sensitive (main_window.menu->redo, TRUE);
    gtk_widget_set_sensitive (main_window.menu->phphelp, TRUE);
    gtk_widget_set_sensitive (main_window.menu->upper, TRUE);
    gtk_widget_set_sensitive (main_window.menu->lower, TRUE);
    
}else{
	if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
            //deactivate toolbar items
            gtk_widget_set_sensitive (main_window.toolbar_main_button_cut, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_paste, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_undo, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_redo, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_replace, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_indent, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_unindent, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_save, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_main_button_save_as, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_find_search_entry, FALSE);
            gtk_widget_set_sensitive (main_window.toolbar_find_goto_entry, FALSE);
            //deactivate menu items
            gtk_widget_set_sensitive (main_window.menu->code, FALSE);
            gtk_widget_set_sensitive (main_window.menu->cut, FALSE);
            gtk_widget_set_sensitive (main_window.menu->paste, FALSE);
            gtk_widget_set_sensitive (main_window.menu->save, FALSE);
            gtk_widget_set_sensitive (main_window.menu->saveas, FALSE);
            gtk_widget_set_sensitive (main_window.menu->reload, FALSE);
            gtk_widget_set_sensitive (main_window.menu->rename, FALSE);
            gtk_widget_set_sensitive (main_window.menu->indent, FALSE);
            gtk_widget_set_sensitive (main_window.menu->unindent, FALSE);
            gtk_widget_set_sensitive (main_window.menu->replace, FALSE);
            gtk_widget_set_sensitive (main_window.menu->plugin, FALSE);
            gtk_widget_set_sensitive (main_window.menu->undo, FALSE);
            gtk_widget_set_sensitive (main_window.menu->redo, FALSE);
            gtk_widget_set_sensitive (main_window.menu->phphelp, FALSE);
            gtk_widget_set_sensitive (main_window.menu->upper, FALSE);
            gtk_widget_set_sensitive (main_window.menu->lower, FALSE);
            }
}
}
