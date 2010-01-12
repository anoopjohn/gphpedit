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
//#include "main_window_menu.h"
#include "tab.h"
#include "main_window_callbacks.h"
#include "preferences.h"
#include "classbrowser.h"
#include "plugin.h"
#include "templates.h"
#define PLUGINSTARTPOS 6

MainWindow main_window;
Mainmenu menu;
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

	g_io_channel_read(inter_gphpedit_io, buf, sizeof(buf), &size);
	//g_print("Passed %s\n", buf);
	tab_create_new(TAB_FILE, g_string_new(buf));
	return FALSE;
}



/*void main_window_create_unix_socket(void)
{
	struct sockaddr_un name;
	int sock;
	size_t size;
 
	// Create the socket. 

	sock = socket (PF_UNIX, SOCK_STREAM, 0);
	if (sock < 0) {
		g_print ("Socket creation failed\n");
		exit (EXIT_FAILURE);
	}
 
	// Bind a name to the socket. 
 
	name.sun_family = AF_FILE;
	strcpy (name.sun_path, "/tmp/gphpedit.sock");
 
	// The size of the address is the offset of the start of the filename, plus its length, plus one for the terminating null byte.
	size = (offsetof (struct sockaddr_un, sun_path)
					+ strlen (name.sun_path) + 1);
 
	if (bind (sock, (struct sockaddr *) &name, size) < 0) {
		perror ("bind");
		exit (EXIT_FAILURE);
	}
 
	unix_socket = sock;
}*/

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
	main_window.toolbar_main_button_new = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_NEW));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_new), _("New File"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_new), -1);
	gtk_widget_show (main_window.toolbar_main_button_new);

	main_window.toolbar_main_button_open = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_OPEN));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_open), _("Open File"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_open), -1);
	gtk_widget_show (main_window.toolbar_main_button_open);

	main_window.toolbar_main_button_save = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_SAVE));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_save), _("Save File"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_save), -1);
	gtk_widget_show (main_window.toolbar_main_button_save);

	main_window.toolbar_main_button_save_as = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_save_as), _("Save File As..."));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_save_as), -1);
	gtk_widget_show (main_window.toolbar_main_button_save_as);

	main_window.toolbar_main_button_close = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_CLOSE));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_close), _("Close File"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_close), -1);
	gtk_widget_show (main_window.toolbar_main_button_close);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_open), "clicked", G_CALLBACK (on_open1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_new), "clicked", G_CALLBACK (on_new1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_save), "clicked", G_CALLBACK (on_save1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_save_as), "clicked", G_CALLBACK (on_save_as1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_close), "clicked", G_CALLBACK (on_close1_activate), NULL);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	// Add the Undo operations to the Main Toolbar

	main_window.toolbar_main_button_undo = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_UNDO));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_undo), _("Undo last change"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_undo), -1);
	gtk_widget_show (main_window.toolbar_main_button_undo);

	main_window.toolbar_main_button_redo = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_REDO));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_redo), _("Redo last change"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_redo), -1);
	gtk_widget_show (main_window.toolbar_main_button_redo);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_undo), "clicked", G_CALLBACK (on_undo1_activate), NULL);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_redo), "clicked", G_CALLBACK (on_redo1_activate), NULL);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));

	// Add the Clipboard operations to the Main Toolbar

	main_window.toolbar_main_button_cut = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_CUT));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_cut), _("Cut current selection"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_cut), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_cut), "clicked", G_CALLBACK (on_cut1_activate), NULL);
	gtk_widget_show (main_window.toolbar_main_button_cut);

	main_window.toolbar_main_button_copy = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_COPY));
  	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_copy), _("Copy current selection"));
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_copy), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_copy), "clicked", G_CALLBACK (on_copy1_activate), NULL);
	gtk_widget_show (main_window.toolbar_main_button_copy);

	main_window.toolbar_main_button_paste = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_PASTE));
  	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_paste), _("Paste current selection"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_paste), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_paste), "clicked", G_CALLBACK (on_paste1_activate), NULL);
	gtk_widget_show (main_window.toolbar_main_button_paste);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	// Add the Search operations to the Main Toolbar

	main_window.toolbar_main_button_find = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_FIND));
  	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_find), _("Find Text"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_find), -1);
	gtk_widget_show (main_window.toolbar_main_button_find);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_find), "clicked", G_CALLBACK (on_find1_activate), NULL);

	main_window.toolbar_main_button_replace = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_FIND_AND_REPLACE));
  	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_replace), _("Find/Replace text"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_replace), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_replace), "clicked", G_CALLBACK (on_replace1_activate), NULL);
	gtk_widget_show (main_window.toolbar_main_button_replace);
	
	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));

	// Add the indent/unindent operations to the Main Toolbar	
	/*indent block*/

	main_window.toolbar_main_button_indent = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_INDENT));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_indent), _("Indent block"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_indent), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_indent), "clicked", G_CALLBACK (block_indent), NULL);
	gtk_widget_show (main_window.toolbar_main_button_indent);
	/*unindent block*/
	main_window.toolbar_main_button_unindent = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_UNINDENT));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_unindent), _("Unindent block"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_unindent), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_unindent), "clicked", G_CALLBACK (block_unindent), NULL);
	gtk_widget_show (main_window.toolbar_main_button_unindent);

	main_window.toolbar_separator=gtk_separator_tool_item_new();
	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_separator), -1);
	gtk_widget_show (GTK_WIDGET(main_window.toolbar_separator));
	//zoom in
	main_window.toolbar_main_button_zoom_in = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_IN));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_zoom_in), _("Zoom in"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_zoom_in), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_zoom_in), "clicked", G_CALLBACK (zoom_in), NULL);
	gtk_widget_show (main_window.toolbar_main_button_zoom_in);
	//zoom out
	main_window.toolbar_main_button_zoom_out = GTK_WIDGET(gtk_tool_button_new_from_stock(GTK_STOCK_ZOOM_OUT));
	gtk_tool_item_set_tooltip_text(GTK_TOOL_ITEM (main_window.toolbar_main_button_zoom_out), _("Zoom out"));
  	gtk_toolbar_insert(GTK_TOOLBAR(main_window.toolbar_main), GTK_TOOL_ITEM (main_window.toolbar_main_button_zoom_out), -1);
	g_signal_connect (G_OBJECT (main_window.toolbar_main_button_zoom_out), "clicked", G_CALLBACK (zoom_out), NULL);
	gtk_widget_show (main_window.toolbar_main_button_zoom_out);

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

	main_window.toolbar_find_goto_entry = gtk_entry_new_with_max_length(8);
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
	
}


static void main_window_create_appbar(void)
{
    main_window.appbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.appbar, FALSE, TRUE, 1);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (main_window.appbar), FALSE);
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
	//Commented out as panes aren't used yet AJ 2003-01-21 TODO: replace old style code with new main_window.* code
	GtkWidget *notebook_manager;
	notebook_manager = gtk_notebook_new ();
	main_window.notebook_manager= gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (main_window.notebook_manager), GTK_POS_BOTTOM);
	gtk_widget_set_usize(main_window.notebook_manager,200,400);
	gtk_widget_show (main_window.notebook_manager);
	box = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(box);
//	gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), box, FALSE, TRUE);
	gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), main_window.notebook_manager, FALSE, TRUE);

	main_window.classlabel = gtk_label_new ("Class Browser");
	gtk_widget_show(main_window.classlabel);
	pos=gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), box, main_window.classlabel, 0);

	//add checkbox to show only current file's classes
	//the signals to be checked for the check box are onclick of the checkbox 
	//and the on change of the file.
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
	//
	main_window.chkOnlyCurFileFuncs = gtk_check_button_new_with_label(_("Parse only current file")); 
	gtk_widget_show (main_window.chkOnlyCurFileFuncs);
	gtk_box_pack_start(GTK_BOX(hbox), main_window.chkOnlyCurFileFuncs, FALSE, FALSE, 10);
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
	// file browser code
	folderbrowser_create(&main_window);
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
	gtk_widget_set_usize(main_window.lint_view,80,80);
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
        gtk_widget_set_usize(main_window.notebook_editor,400,400);
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
//	GString *dir;
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
		}
		else if(main_window.current_editor->type == TAB_HELP) {
			title = g_string_new("Help: ");
			title = g_string_append(title, main_window.current_editor->help_function);
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
                
                bin = GTK_BIN(menu.plugins[num_plugin]);
		//g_print("Bin is %p\n", bin);
		if (bin) {
			label = GTK_LABEL(gtk_bin_get_child(bin));
		
			gtk_label_set_text(label, plugin->name);

                        gtk_widget_show(&menu.plugin[num_plugin]);
		}
		
		num_plugin++;
	}

	//g_print("Blanking all non-found plugin entries\n");
        for (hide_plugin=num_plugin; hide_plugin <NUM_PLUGINS_MAX; hide_plugin++) {
            gtk_widget_hide(menu.plugins[hide_plugin]);
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
	command_line = g_string_append(command_line, "' '");
	
	if (plugin->type == GPHPEDIT_PLUGIN_TYPE_SELECTION) {
		wordStart = gtk_scintilla_get_selection_start(GTK_SCINTILLA(main_window.current_editor->scintilla));
		wordEnd = gtk_scintilla_get_selection_end(GTK_SCINTILLA(main_window.current_editor->scintilla));
		current_selection = gtk_scintilla_get_text_range (GTK_SCINTILLA(main_window.current_editor->scintilla), wordStart, wordEnd, &ac_length);
		
		command_line = g_string_append(command_line, current_selection);
	}
	else if (plugin->type == GPHPEDIT_PLUGIN_TYPE_FILENAME) {
		command_line = g_string_append(command_line, editor_convert_to_local(main_window.current_editor));		
	}
	command_line = g_string_append(command_line, "'");
	
	//g_print("SPAWNING: %s\n", command_line->str);
	
	if (g_spawn_command_line_sync(command_line->str,&stdout,NULL, &exit_status,&error)) {
		data = strstr(stdout, "\n");
		data++;
		
		//g_print("COMMAND: %s\nSTDOUT:%s\nOUTPUT: %s\n", command_line->str, stdout, data);
		
		if (g_strncasecmp(stdout, "INSERT", MIN(strlen(stdout), 6))==0) {
			if (data) {
				gtk_scintilla_insert_text(GTK_SCINTILLA(main_window.current_editor->scintilla), 
					gtk_scintilla_get_current_pos(GTK_SCINTILLA(main_window.current_editor->scintilla)), data);
			}
		}
		else if (g_strncasecmp(stdout, "REPLACE", MIN(strlen(stdout), 7))==0) {
			if (data) {
				gtk_scintilla_replace_sel(GTK_SCINTILLA(main_window.current_editor->scintilla), data);
			}
		}
		else if (g_strncasecmp(stdout, "MESSAGE", MIN(strlen(stdout),7))==0) {
				info_dialog(plugin->name, data);
		}
		else if (g_strncasecmp(stdout, "OPEN", MIN(strlen(stdout), 4))==0) {
			if (DEBUG_MODE) { g_print("DEBUG: main_window.c:plugin_exec: Opening file :date: %s\n", data); }
			switch_to_file_or_open(data, 0);
		}
		else if (g_strncasecmp(stdout, "DEBUG", MIN(strlen(stdout), 5))==0) {
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
              gtk_widget_hide(menu.plugin);
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
                    bin = GTK_BIN(menu.recent[entry]);
			if (bin) {
				gtk_label_set_text(GTK_LABEL(gtk_bin_get_child(bin)), full_filename);
                                gtk_widget_show(menu.recent[entry]);
			}
		}
		else {
                    gtk_widget_hide(menu.recent[entry]);
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

GdkPixbuf *create_pixbuf(const gchar * filename)
{
   GdkPixbuf *pixbuf;
   GError *error = NULL;
   pixbuf = gdk_pixbuf_new_from_file(filename, &error);
   if(!pixbuf) {
      fprintf(stderr, "%s\n", error->message);
      g_error_free(error);
   }

   return pixbuf;
}

/*
 *classbrowser_status
 *return 0  if classbrowser is hidden
 *return 1 if classbrowser is show
*/
int classbrowser_status(void){
	gint hidden;
	GConfClient *config;
        config=gconf_client_get_default ();
	hidden = gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_hidden",NULL);
	return hidden;
}

void tog_classbrowser(GtkCheckMenuItem *checkmenuitem, gpointer user_data){
    classbrowser_show_hide(NULL);
}

void tog_statusbar(GtkWidget *widget, gpointer user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
    gtk_widget_show(main_window.appbar);
  } else {
    gtk_widget_hide(main_window.appbar);
  }
}

void tog_maintoolbar(GtkWidget *widget, gpointer user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
    gtk_widget_show(main_window.toolbar_main);
  } else {
    gtk_widget_hide(main_window.toolbar_main);
  }
}

void tog_findtoolbar(GtkWidget *widget, gpointer user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
    gtk_widget_show(main_window.toolbar_find);
  } else {
    gtk_widget_hide(main_window.toolbar_find);
  }
}
void tog_fullscreen(GtkWidget *widget, gpointer user_data)
{
  if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
    gtk_window_fullscreen (GTK_WINDOW(main_window.window));
  } else {
    gtk_window_unfullscreen (GTK_WINDOW(main_window.window));
  }
}

void main_window_create_menu(void){
    int i;
main_window.prinbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window.window), main_window.prinbox);
  gtk_widget_show (main_window.prinbox);

GtkAccelGroup *accel_group = NULL;

  //create menu bar
  menu.menubar = gtk_menu_bar_new();
  //create menus
  menu.menunew = gtk_menu_new();
  menu.menuedit = gtk_menu_new();
  menu.menuview = gtk_menu_new();
  menu.menucode = gtk_menu_new();
  menu.menuplugin = gtk_menu_new();
  menu.menuhelp = gtk_menu_new();
    
  accel_group = gtk_accel_group_new();
  gtk_window_add_accel_group(GTK_WINDOW(main_window.window), accel_group);
  //Menu file
  menu.file = gtk_menu_item_new_with_mnemonic(_("_File"));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.file), menu.menunew);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.file);
  menu.newi = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.newi);
  g_signal_connect(G_OBJECT(menu.newi), "activate", G_CALLBACK(on_new1_activate), NULL);
  gtk_widget_add_accelerator(menu.newi, "activate", accel_group, GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  menu.open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
  g_signal_connect(G_OBJECT(menu.open), "activate", G_CALLBACK(on_open1_activate), NULL);
  gtk_widget_add_accelerator(menu.open, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.open);
  menu.opensel = gtk_menu_item_new_with_mnemonic(_("_Open selected file"));
  g_signal_connect(G_OBJECT(menu.opensel), "activate", G_CALLBACK(on_openselected1_activate), NULL);
  gtk_widget_add_accelerator(menu.opensel, "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.opensel);
  
  menu.reciente = gtk_menu_item_new_with_mnemonic(_("_Recent Files"));
  gtk_container_add (GTK_CONTAINER (menu.menunew), menu.reciente);
  menu.menureciente = gtk_menu_new();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu.reciente), menu.menureciente);

  for (i=0;i<NUM_REOPEN_MAX;i++){
  menu.recent[i]= gtk_menu_item_new_with_mnemonic(_("_Recent"));
  g_signal_connect(G_OBJECT(menu.recent[i]), "activate", G_CALLBACK(reopen_recent), (gpointer)i);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menureciente), menu.recent[i]);
  }
  
  menu.reload = gtk_menu_item_new_with_mnemonic(_("_Reload current file"));
  g_signal_connect(G_OBJECT(menu.reload), "activate", G_CALLBACK(on_reload1_activate), NULL);
  gtk_widget_add_accelerator(menu.reload, "activate", accel_group, GDK_r, GDK_SHIFT_MASK | GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.reload);

  menu.sep = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.sep);

  menu.save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, NULL);
  g_signal_connect(G_OBJECT(menu.save), "activate", G_CALLBACK(on_save1_activate), NULL);
  gtk_widget_add_accelerator(menu.save, "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.save);

  menu.saveas = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE_AS, NULL);
  g_signal_connect(G_OBJECT(menu.saveas), "activate", G_CALLBACK(on_save_as1_activate), NULL);
  gtk_widget_add_accelerator(menu.saveas, "activate", accel_group, GDK_s, GDK_SHIFT_MASK | GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.saveas);
  
  menu.saveall = gtk_menu_item_new_with_mnemonic(_("Save A_ll"));
  g_signal_connect(G_OBJECT(menu.saveall), "activate", G_CALLBACK(on_saveall1_activate), NULL);
  gtk_widget_add_accelerator(menu.saveall, "activate", accel_group, GDK_a, GDK_SHIFT_MASK | GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.saveall);

  menu.rename = gtk_menu_item_new_with_mnemonic(_("_Remane"));
  g_signal_connect(G_OBJECT(menu.rename), "activate", G_CALLBACK(on_rename1_activate), NULL);
  gtk_widget_add_accelerator(menu.rename, "activate", accel_group, GDK_r, GDK_SHIFT_MASK | GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.rename);

  menu.close = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, NULL);
  g_signal_connect(G_OBJECT(menu.close), "activate", G_CALLBACK(on_close1_activate), NULL);
  gtk_widget_add_accelerator(menu.close, "activate", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.close);

  menu.sep1 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.sep1);

  menu.quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, accel_group);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menunew), menu.quit);
  g_signal_connect(G_OBJECT(menu.quit), "activate", G_CALLBACK(on_quit1_activate), NULL);
  gtk_widget_add_accelerator(menu.quit, "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  /*Menu edit*/
  menu.edit = gtk_menu_item_new_with_mnemonic("_Edit");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.edit), menu.menuedit);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.edit);
   
  menu.undo = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNDO, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.undo);
  g_signal_connect(G_OBJECT(menu.undo), "activate", G_CALLBACK(on_undo1_activate), NULL);
  gtk_widget_add_accelerator(menu.undo, "activate", accel_group, GDK_z, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  menu.redo = gtk_image_menu_item_new_from_stock(GTK_STOCK_REDO, NULL);
  g_signal_connect(G_OBJECT(menu.redo), "activate", G_CALLBACK(on_redo1_activate), NULL);
  gtk_widget_add_accelerator(menu.redo, "activate", accel_group, GDK_z, GDK_SHIFT_MASK | GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.redo);

  menu.sep2 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit),menu.sep2);
  
  menu.cut = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, NULL);
  g_signal_connect(G_OBJECT(menu.cut), "activate", G_CALLBACK(on_cut1_activate), NULL);
  gtk_widget_add_accelerator(menu.cut, "activate", accel_group, GDK_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.cut);
  menu.copy = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, NULL);
  g_signal_connect(G_OBJECT(menu.copy), "activate", G_CALLBACK(on_copy1_activate), NULL);
  gtk_widget_add_accelerator(menu.copy, "activate", accel_group, GDK_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.copy);
  menu.paste = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, NULL);
  g_signal_connect(G_OBJECT(menu.paste), "activate", G_CALLBACK(on_paste1_activate), NULL);
  gtk_widget_add_accelerator(menu.paste, "activate", accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.paste);
  menu.selectall = gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_ALL, NULL);
  g_signal_connect(G_OBJECT(menu.selectall), "activate", G_CALLBACK(on_selectall1_activate), NULL);
  gtk_widget_add_accelerator(menu.selectall, "activate", accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.selectall);
 menu.sep3 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.sep3);

  menu.find = gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND, NULL);
  g_signal_connect(G_OBJECT(menu.find), "activate", G_CALLBACK(on_find1_activate), NULL);
  gtk_widget_add_accelerator(menu.find, "activate", accel_group, GDK_f, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.find);

  menu.replace = gtk_image_menu_item_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, NULL);
  g_signal_connect(G_OBJECT(menu.replace), "activate", G_CALLBACK(on_replace1_activate), NULL);
  gtk_widget_add_accelerator(menu.replace, "activate", accel_group, GDK_h, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.replace);

  menu.sep4 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.sep4);

  menu.indent = gtk_image_menu_item_new_from_stock(GTK_STOCK_INDENT, NULL);
  g_signal_connect(G_OBJECT(menu.indent), "activate", G_CALLBACK(block_indent), NULL);
  gtk_widget_add_accelerator(menu.indent, "activate", accel_group, GDK_i, GDK_SHIFT_MASK | GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.indent);

  menu.unindent = gtk_image_menu_item_new_from_stock(GTK_STOCK_UNINDENT, NULL);
  g_signal_connect(G_OBJECT(menu.unindent), "activate", G_CALLBACK(block_unindent), NULL);
  gtk_widget_add_accelerator(menu.unindent, "activate", accel_group, GDK_i, GDK_SHIFT_MASK | GDK_CONTROL_MASK |GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.unindent);

  menu.preferences = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
  g_signal_connect(G_OBJECT(menu.preferences), "activate", G_CALLBACK(on_preferences1_activate), NULL);
  gtk_widget_add_accelerator(menu.preferences, "activate", accel_group, GDK_F5, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuedit), menu.preferences);

/* view menu */
  menu.view = gtk_menu_item_new_with_mnemonic(_("_View"));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.view), menu.menuview);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.view);

  menu.viewstatusbar = gtk_check_menu_item_new_with_label(_("Statusbar"));
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.viewstatusbar), TRUE);
  g_signal_connect(G_OBJECT(menu.viewstatusbar), "activate", G_CALLBACK(tog_statusbar),NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.viewstatusbar);

  menu.viewmaintoolbar = gtk_check_menu_item_new_with_label(_("Main Toolbar"));
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.viewmaintoolbar), TRUE);
  g_signal_connect(G_OBJECT(menu.viewmaintoolbar), "activate", G_CALLBACK(tog_maintoolbar),NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.viewmaintoolbar);

  menu.viewfindtoolbar = gtk_check_menu_item_new_with_label(_("Find Toolbar"));
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.viewfindtoolbar), TRUE);
  g_signal_connect(G_OBJECT(menu.viewfindtoolbar), "activate", G_CALLBACK(tog_findtoolbar),NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.viewfindtoolbar);

  menu.sep6 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.sep6);
  
  menu.tog_class = gtk_check_menu_item_new_with_label(_("Show Side Panel"));
  if (classbrowser_status()==0){
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.tog_class), TRUE);
  }else {
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.tog_class), FALSE);
  }
  g_signal_connect(G_OBJECT(menu.tog_class), "activate", G_CALLBACK(tog_classbrowser),NULL);
  gtk_widget_add_accelerator(menu.tog_class, "activate", accel_group, GDK_F8, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.tog_class);

  menu.viewfullscreen = gtk_check_menu_item_new_with_label(_("Fullscreen"));
  gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(menu.viewfullscreen), FALSE);
  g_signal_connect(G_OBJECT(menu.viewfullscreen), "activate", G_CALLBACK(tog_fullscreen),NULL);
  gtk_widget_add_accelerator(menu.viewfullscreen, "activate", accel_group, GDK_F11, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuview), menu.viewfullscreen);

  menu.code = gtk_menu_item_new_with_mnemonic(_("_Code"));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.code), menu.menucode);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.code);

  menu.syntax = gtk_menu_item_new_with_mnemonic(_("_Syntax check"));
  g_signal_connect(G_OBJECT(menu.syntax), "activate", G_CALLBACK(syntax_check), NULL);
  gtk_widget_add_accelerator(menu.syntax, "activate", accel_group, GDK_F9, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menucode), menu.syntax);

  menu.clearsyntax= gtk_menu_item_new_with_mnemonic(_("_Clear Syntax check"));
  g_signal_connect(G_OBJECT(menu.clearsyntax), "activate", G_CALLBACK(syntax_check_clear), NULL);
  gtk_widget_add_accelerator(menu.clearsyntax, "activate", accel_group, GDK_F9, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menucode), menu.clearsyntax);

  menu.sep5 = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menucode), menu.sep5);

  menu.record= gtk_menu_item_new_with_mnemonic(_("_Record keyboard macro start/stop"));
  g_signal_connect(G_OBJECT(menu.record), "activate", G_CALLBACK(keyboard_macro_startstop), NULL);
  gtk_widget_add_accelerator(menu.record, "activate", accel_group, GDK_k, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menucode), menu.record);

  menu.playback= gtk_menu_item_new_with_mnemonic(_("_Playback keyboard macro"));
  g_signal_connect(G_OBJECT(menu.playback), "activate", G_CALLBACK(keyboard_macro_playback), NULL);
  gtk_widget_add_accelerator(menu.playback, "activate", accel_group, GDK_k, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menucode), menu.playback);

  menu.force = gtk_menu_item_new_with_mnemonic(_("_Force"));
  gtk_container_add (GTK_CONTAINER (menu.menucode), menu.force);
  menu.menuforce = gtk_menu_new();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu.force), menu.menuforce);

  menu.forcephp= gtk_menu_item_new_with_mnemonic(_("_PHP/HTML/XML"));
  g_signal_connect(G_OBJECT(menu.forcephp), "activate", G_CALLBACK(force_php), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forcephp);

  menu.forcecss= gtk_menu_item_new_with_mnemonic(_("_CSS"));
  g_signal_connect(G_OBJECT(menu.forcecss), "activate", G_CALLBACK(force_css), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forcecss);
  
  menu.forcecxx= gtk_menu_item_new_with_mnemonic(_("C/C_++"));
  g_signal_connect(G_OBJECT(menu.forcecxx), "activate", G_CALLBACK(force_cxx), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forcecxx);

  menu.forcesql= gtk_menu_item_new_with_mnemonic(_("_SQL"));
  g_signal_connect(G_OBJECT(menu.forcesql), "activate", G_CALLBACK(force_sql), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forcesql);

  menu.forceperl= gtk_menu_item_new_with_mnemonic(_("_Perl"));
  g_signal_connect(G_OBJECT(menu.forceperl), "activate", G_CALLBACK(force_perl), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forceperl);

  menu.forcepython= gtk_menu_item_new_with_mnemonic(_("P_ython"));
  g_signal_connect(G_OBJECT(menu.forcepython), "activate", G_CALLBACK(force_python), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuforce), menu.forcepython);

  
  /*aca debe ir el menu proyecto*/
  menu.plugin = gtk_menu_item_new_with_mnemonic(_("_Plugin"));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.plugin), menu.menuplugin);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.plugin);
  for (i=0;i<NUM_PLUGINS_MAX;i++){
  menu.plugins[i]= gtk_menu_item_new_with_mnemonic(_("_Plugin"));
  g_signal_connect(G_OBJECT(menu.plugins[i]), "activate", G_CALLBACK(run_plugin), (gpointer)i);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuplugin), menu.plugins[i]);
  }
  menu.help = gtk_menu_item_new_with_mnemonic(_("_Help"));
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu.help), menu.menuhelp);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menubar), menu.help);

  menu.phphelp= gtk_menu_item_new_with_mnemonic(_("_PHP Help"));
  g_signal_connect(G_OBJECT(menu.phphelp), "activate", G_CALLBACK(context_help), NULL);
  gtk_widget_add_accelerator(menu.phphelp, "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuhelp), menu.phphelp);

  menu.abouthelp= gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
  g_signal_connect(G_OBJECT(menu.abouthelp), "activate", G_CALLBACK(on_about1_activate), NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu.menuhelp), menu.abouthelp);
 
  gtk_box_pack_start (GTK_BOX (main_window.prinbox), menu.menubar, FALSE, FALSE, 0);
  gtk_widget_show_all (menu.menubar);

}
void main_window_create(void)
{

        main_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(main_window.window), _("gPHPEdit"));
        gtk_window_set_default_size(GTK_WINDOW(main_window.window), 230, 150);
        gtk_window_set_position(GTK_WINDOW(main_window.window), GTK_WIN_POS_CENTER);
        gtk_window_set_icon(GTK_WINDOW(main_window.window), create_pixbuf(PIXMAP_DIR "/" GPHPEDIT_PIXMAP_ICON));
        
	preferences_apply();
        main_window_create_menu();
        main_window_create_toolbars();
        main_window_create_panes();
        main_window_fill_panes();
        main_window_create_appbar();
        //install_menu_hints();

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
}
