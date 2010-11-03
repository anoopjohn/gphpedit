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

#include "main_window.h"
#include "debug.h"
#include "tab.h"
#include "main_window_callbacks.h"
#include "templates.h"
#include "gvfs_utils.h"
#include "gphpedit-close-button.h"
#include "gphpedit-statusbar.h"
#include "syntax_check_window.h"
#include "filebrowser_ui.h"
#include "classbrowser_ui.h"

void update_controls(Document *document);

MainWindow main_window;
GIOChannel* inter_gphpedit_io;
guint inter_gphpedit_event_id;

void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
  set_preferences_manager_window_maximized(main_window.prefmg, GDK_WINDOW_STATE_MAXIMIZED && event->new_window_state);
}

void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data) {
  if (!get_preferences_manager_window_maximized(main_window.prefmg)) {
    gint left, top;
    gtk_window_get_position(GTK_WINDOW(main_window.window), &left, &top);
    set_preferences_manager_window_size (main_window.prefmg, allocation->width, allocation->height);
    set_preferences_manager_window_position (main_window.prefmg, top, left);
  }
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
  if (gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)) != 0) {
    g_object_set(main_window.prefmg, "side_panel_size", gtk_paned_get_position(GTK_PANED(main_window.main_horizontal_pane)), NULL);
  }
  return TRUE;
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
      gphpedit_debug_message(DEBUG_IPC, "%s:%d\n", argv[i], strlen(argv[i]));
      g_io_channel_write_chars(inter_gphpedit_io, argv[i], strlen(argv[i]),
                   &bytes_written, &error);
      ++i;
    }
  }
}


gboolean channel_pass_filename_callback(GIOChannel *source, GIOCondition condition, gpointer data )
{
  gsize size;
  gchar buf[1024];
  GError *error=NULL;
  if (g_io_channel_read_chars (inter_gphpedit_io,buf,sizeof(buf), &size,&error)!=G_IO_STATUS_NORMAL){
    g_print("Error reading GIO Chanel. Error:%s\n",error->message);
  }
  gphpedit_debug_message(DEBUG_IPC, "Passed %s\n", buf);
  document_manager_add_new_document(main_window.docmg, TAB_FILE, buf, 0);
  return FALSE;
}

static void main_window_create_appbar(void)
{
  main_window.appbar = gphpedit_statusbar_new ();
  gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.appbar, FALSE, TRUE, 1);
  gphpedit_statusbar_set_zoom_level(GPHPEDIT_STATUSBAR(main_window.appbar),100);
  if (get_preferences_manager_show_statusbar(main_window.prefmg)) gtk_widget_show (main_window.appbar);
}

static void main_window_create_panes(void)
{
  main_window.main_horizontal_pane = gtk_hpaned_new ();
  gtk_widget_show (main_window.main_horizontal_pane);
  gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.main_horizontal_pane, TRUE, TRUE, 0);

  main_window.main_vertical_pane = gtk_vpaned_new ();
  gtk_widget_show (main_window.main_vertical_pane);
  gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), main_window.main_vertical_pane, FALSE, TRUE);
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (classbrowser_accept_size), NULL);
  gboolean hidden;
  gint size;
  g_object_get(main_window.prefmg, "side_panel_hidden", &hidden,"side_panel_size", &size, NULL);

  gtk_paned_set_position(GTK_PANED(main_window.main_horizontal_pane), size);
  if (hidden) classbrowser_hide();

  main_window.prin_hbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(main_window.prin_hbox);
  gtk_paned_pack2 (GTK_PANED (main_window.main_horizontal_pane), main_window.prin_hbox, TRUE, TRUE);  
}


static void create_side_panel(void){
  GtkWidget *prin_sidebox;

  prin_sidebox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(prin_sidebox);
  gtk_paned_pack1 (GTK_PANED (main_window.main_vertical_pane), prin_sidebox, FALSE, TRUE);

  /* Close button for the side bar */
  GtkWidget *close_box;

  close_box = gtk_hbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(close_box), 0);
  main_window.close_sidebar_button = gphpedit_close_button_new();
  gtk_widget_set_tooltip_text(main_window.close_sidebar_button, _("Close side panel"));
  g_signal_connect(G_OBJECT(main_window.close_sidebar_button), "clicked", G_CALLBACK (classbrowser_show_hide),NULL);
  gtk_widget_show(main_window.close_sidebar_button);
  gtk_box_pack_end(GTK_BOX(close_box), main_window.close_sidebar_button, FALSE, FALSE, 0);
  gtk_widget_show(close_box);

  /* add close button */
  gtk_box_pack_start(GTK_BOX(prin_sidebox), close_box, FALSE, TRUE, 2);
  
  main_window.notebook_manager= gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (main_window.notebook_manager), GTK_POS_BOTTOM);
  gtk_widget_set_size_request (main_window.notebook_manager, 300,400);
  gtk_widget_show (main_window.notebook_manager);
  /* add a notebook */
  gtk_box_pack_start(GTK_BOX(prin_sidebox), main_window.notebook_manager, TRUE, TRUE, 2);
  
  /* Classbrowser stuff creation */  
  main_window.classbrowser = gphpedit_classbrowser_new (); 
  gtk_widget_show(main_window.classbrowser);
  GtkWidget *classlabel;
  classlabel = gtk_image_new_from_file (PIXMAP_DIR "/classbrowser.png");
  /*set tooltip*/
  gtk_widget_set_tooltip_text (classlabel,_("Class Browser"));
  gtk_widget_show(classlabel);
  gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), main_window.classbrowser, classlabel, 0);
  /* File browser stuff creation */
  if (get_preferences_manager_show_filebrowser(main_window.prefmg)){
  main_window.folder= gphpedit_filebrowser_new();
  gtk_widget_show(main_window.folder);
  GtkWidget *label= gtk_image_new_from_icon_name ("file-manager", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_show(label);
  gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), main_window.folder, label, 1);
  }
}

static void main_window_fill_panes(void)
{
  /* create side panel */
  create_side_panel();

  main_window.notebook_editor = gtk_notebook_new ();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(main_window.notebook_editor), TRUE);
  gtk_widget_show (main_window.notebook_editor);
  gtk_box_pack_start(GTK_BOX(main_window.prin_hbox), main_window.notebook_editor, TRUE, TRUE, 2);

  gtk_widget_set_size_request (main_window.notebook_editor, 400,400);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "switch_page", G_CALLBACK (on_notebook_switch_page), NULL);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "focus-tab", G_CALLBACK (on_notebook_focus_tab), NULL);

  /* add syntax check window */
  main_window.win= gtk_syntax_check_window_new ();
  gtk_box_pack_start(GTK_BOX(main_window.prin_hbox), GTK_WIDGET(main_window.win), TRUE, TRUE, 2);
}

/**
 * Update the application title when switching tabs, closing or opening
 * or opening new tabs or opening new files.
 */
void update_app_title(Document *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gchar *title = document_get_title(document);
  update_status_combobox(document);
  update_zoom_level();
  update_controls(document);
  //If there is no file opened set the name as gPHPEdit
  if (!title) title = g_strdup(_("gPHPEdit"));
  gtk_window_set_title(GTK_WINDOW(main_window.window), title);
  g_free(title);
}

static void main_window_create_prinbox(void){
  main_window.prinbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window.window), main_window.prinbox);
  gtk_widget_show (main_window.prinbox);
}

static void set_colormap(GtkWidget *window){
  /*Set RGBA colormap*/
  GdkScreen *screen= gtk_widget_get_screen (window);
  GdkColormap *colormap= gdk_screen_get_rgba_colormap (screen);
  if (colormap && gdk_screen_is_composited (screen)) gtk_widget_set_default_colormap (colormap);
  /*End set RGBA colormap*/
}

static void create_app_main_window(const gchar *title){
  main_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window.window), title);
  gint w, h, x, y;
  if (get_preferences_manager_window_maximized(main_window.prefmg)) {
 		gtk_window_maximize(GTK_WINDOW(main_window.window));
	} else {
    get_preferences_manager_window_size (main_window.prefmg, &w, &h);
    gtk_window_set_default_size(GTK_WINDOW(main_window.window), w, h);
    get_preferences_manager_window_position (main_window.prefmg, &y, &x);
    gtk_window_move(GTK_WINDOW(main_window.window), x, y);
  }
  g_set_application_name (title);
  gtk_window_set_default_icon_name ("gphpedit");
  /* set RGBA colormap */        
  set_colormap(main_window.window);
}

void main_window_create(void){

  create_app_main_window(_("gPHPEdit"));
  main_window_create_prinbox();

  /* add menu bar to main window */
  main_window.menu=menubar_new ();
  gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.menu, FALSE, FALSE, 0);
  gtk_widget_show_all (main_window.menu);

  main_window.toolbar_main= toolbar_new ();
  gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_main, FALSE, FALSE, 0);
  if (get_preferences_manager_show_maintoolbar(main_window.prefmg)) gtk_widget_show (main_window.toolbar_main);

  main_window.toolbar_find = toolbar_find_new(menubar_get_accel_group(MENUBAR(main_window.menu)));
  gtk_box_pack_start (GTK_BOX (main_window.prinbox), main_window.toolbar_find, FALSE, FALSE, 0);
  if (get_preferences_manager_show_findtoolbar(main_window.prefmg)) gtk_widget_show (main_window.toolbar_find);


  main_window_create_panes();
  main_window_fill_panes();
  main_window_create_appbar();
  
  main_window.clltipmg = calltip_manager_new();

  g_signal_connect (G_OBJECT (main_window.window), "delete_event", G_CALLBACK(main_window_delete_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "destroy", G_CALLBACK (main_window_destroy_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "key_press_event", G_CALLBACK (main_window_key_press_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (main_window_resize), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "window-state-event", G_CALLBACK (main_window_state_changed), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "focus-in-event", G_CALLBACK (main_window_activate_focus), NULL);

  main_window.clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  gtk_widget_show(main_window.window);
  
  update_app_title(document_manager_get_current_document(main_window.docmg));

  main_window.stylemg = gtk_source_style_scheme_manager_new ();
  gchar *theme_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "themes", NULL);
  gtk_source_style_scheme_manager_prepend_search_path (main_window.stylemg, theme_dir);
  g_free(theme_dir);
}

void update_controls(Document *document){
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  if (!document) return ;
  gboolean read_only, can_modify, preview;
  g_object_get(document, "read_only", &read_only, "can_modify", &can_modify, "can_preview", &preview, NULL);
  menubar_update_controls(MENUBAR(main_window.menu), can_modify, preview, read_only);
  toolbar_update_controls(TOOLBAR(main_window.toolbar_main), can_modify, read_only);
  toolbar_update_controls(TOOLBAR(main_window.toolbar_find), can_modify, read_only);
}
