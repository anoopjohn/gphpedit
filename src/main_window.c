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
#include "gvfs_utils.h"
#include "syntax_check.h"

MainWindow main_window;
GIOChannel* inter_gphpedit_io;
guint inter_gphpedit_event_id;
gboolean DEBUG_MODE = FALSE;

void create_untitled_if_empty(void)
{
  if (!editors) {
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
  GError *error=NULL;
  GFile *config;
  gchar *uri=g_strdup_printf("%s/%s",g_get_home_dir(),".gphpedit");
  if (!filename_file_exist(uri)){
    config=get_gfile_from_filename (uri);
    if (!g_file_make_directory (config, NULL, &error)){
      if (error->code !=2){
        g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code,error->message);
        exit(-1);
        }
        g_error_free(error);
      }
    g_object_unref(config);
    error=NULL;
  }
  g_free(uri);
  gchar *plu=g_strdup_printf("%s/%s/%s",g_get_home_dir(),".gphpedit","plugins");
  if (!filename_file_exist(plu)){
    GFile *plugin=get_gfile_from_filename (plu);
    if (!g_file_make_directory (plugin, NULL, &error)){
      //if error code = 2 dir already exist
      if (error->code !=2){
        g_print(_("Unable to create ~/.gphpedit/ (%d) %s"), error->code,error->message);
        exit(-1);
      }
      g_error_free(error);
    }
    g_object_unref(plugin);
  }
  g_free(plu);
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
  main_window.main_horizontal_pane = gtk_hpaned_new ();
  gtk_widget_show (main_window.main_horizontal_pane);
  gtk_box_pack_start(GTK_BOX(main_window.prinbox), main_window.main_horizontal_pane, TRUE, TRUE, 0);

  main_window.main_vertical_pane = gtk_vpaned_new ();
  gtk_widget_show (main_window.main_vertical_pane);
  gtk_paned_pack1 (GTK_PANED (main_window.main_horizontal_pane), main_window.main_vertical_pane, FALSE, TRUE);
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (classbrowser_accept_size), NULL);
  move_classbrowser_position();
  GConfClient *config;
  config=gconf_client_get_default ();
        
  if (gconf_client_get_int (config,"/gPHPEdit/main_window/classbrowser_hidden",NULL) == 1)
    classbrowser_hide();
  g_object_unref(config);

  main_window.prin_hbox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(main_window.prin_hbox);
  gtk_paned_pack2 (GTK_PANED (main_window.main_horizontal_pane), main_window.prin_hbox, TRUE, TRUE);  
}


static void create_side_panel(void){
  GtkWidget *prin_sidebox;
  GtkWidget *box2;
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  prin_sidebox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(prin_sidebox);
  gtk_paned_pack1 (GTK_PANED (main_window.main_vertical_pane), prin_sidebox, FALSE, TRUE);

  /* Close button for the side bar */
  GtkWidget *close_box;
  close_box = gtk_hbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(close_box), 0);
  main_window.close_image = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_misc_set_padding(GTK_MISC(main_window.close_image), 0, 0);
  main_window.close_sidebar_button = gtk_button_new();
  gtk_widget_set_tooltip_text(main_window.close_sidebar_button, _("Close side panel"));
  gtk_button_set_image(GTK_BUTTON(main_window.close_sidebar_button), main_window.close_image);
  gtk_button_set_relief(GTK_BUTTON(main_window.close_sidebar_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click(GTK_BUTTON(main_window.close_sidebar_button), FALSE);
  g_signal_connect(G_OBJECT(main_window.close_sidebar_button), "clicked", G_CALLBACK (classbrowser_show_hide),NULL);
  gtk_widget_show(main_window.close_image);
  gtk_widget_show(main_window.close_sidebar_button);
  gtk_box_pack_end(GTK_BOX(close_box), main_window.close_sidebar_button, FALSE, FALSE, 0);
  gtk_widget_show(close_box);

  /*agrego el boton de cerrar*/
  gtk_box_pack_start(GTK_BOX(prin_sidebox), close_box, FALSE, TRUE, 2);
  
  main_window.notebook_manager= gtk_notebook_new ();
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (main_window.notebook_manager), GTK_POS_BOTTOM);
  gtk_widget_set_size_request (main_window.notebook_manager, 200,400);
  gtk_widget_show (main_window.notebook_manager);
  /* agrego el notebook */
  gtk_box_pack_start(GTK_BOX(prin_sidebox), main_window.notebook_manager, TRUE, TRUE, 2);
  
  /* creo un tab y lo agrego el notebook */  
  GtkWidget *notebox;
  notebox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(notebox);

  main_window.classlabel = gtk_image_new_from_file (PIXMAP_DIR "/classbrowser.png");
  /*set tooltip*/
  gtk_widget_set_tooltip_text (main_window.classlabel,_("Class Browser"));
  gtk_widget_show(main_window.classlabel);

  gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), notebox, main_window.classlabel, 0);

  /* add checkbox to show only current file's classes
  the signals to be checked for the check box are onclick of the checkbox
  and the on change of the file.
  */
  GtkWidget *hbox;
  hbox = gtk_hbox_new(FALSE, 0);
  main_window.chkOnlyCurFileFuncs = gtk_check_button_new_with_label(_("Parse only current file"));
  GConfClient *config;
  config=gconf_client_get_default ();
  gtk_toggle_button_set_active ((GtkToggleButton *)main_window.chkOnlyCurFileFuncs,gconf_client_get_int (config,"/gPHPEdit/classbrowser/onlycurrentfile",NULL));
  gtk_widget_show (main_window.chkOnlyCurFileFuncs);
  gtk_widget_show (hbox);
  gtk_box_pack_start(GTK_BOX(hbox), main_window.chkOnlyCurFileFuncs, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(notebox), hbox, FALSE, FALSE, 10);
  g_signal_connect (G_OBJECT (main_window.chkOnlyCurFileFuncs), "clicked",
            G_CALLBACK (on_parse_current_click), NULL);

  main_window.scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (main_window.scrolledwindow3);
  gtk_box_pack_start(GTK_BOX(notebox), main_window.scrolledwindow3, TRUE, TRUE, 0);

  box2 = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(box2);
  main_window.treeviewlabel = gtk_label_new(_("FILE: "));
  gtk_label_set_justify(GTK_LABEL(main_window.treeviewlabel), GTK_JUSTIFY_LEFT);
  gtk_widget_show(main_window.treeviewlabel);
  gtk_box_pack_start(GTK_BOX(box2), main_window.treeviewlabel, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(notebox), box2, FALSE, FALSE, 4);
  
  main_window.classtreestore = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
  /* enable sorting of the columns */
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
}

static void main_window_fill_panes(void)
{
  /* create side panel */
  create_side_panel();

  main_window.notebook_editor = gtk_notebook_new ();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(main_window.notebook_editor), TRUE);
  /*
    GTK_WIDGET_UNSET_FLAGS (main_window.notebook_editor, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT);
   Fix to scrollable list of tabs, however it then messes up grabbing of focus
   Hence the focus-tab event (which GTK doesn't seem to recognise
  */
  gtk_widget_set_receives_default (main_window.notebook_editor,FALSE);
  GTK_WIDGET_UNSET_FLAGS(main_window.notebook_editor, GTK_CAN_FOCUS);
  gtk_widget_show (main_window.notebook_editor);
  gtk_box_pack_start(GTK_BOX(main_window.prin_hbox), main_window.notebook_editor, TRUE, TRUE, 2);

  gtk_widget_set_size_request (main_window.notebook_editor, 400,400);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "switch_page", G_CALLBACK (on_notebook_switch_page), NULL);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "focus-tab", G_CALLBACK (on_notebook_focus_tab), NULL);


  main_window.scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start(GTK_BOX(main_window.prin_hbox), main_window.scrolledwindow1, TRUE, TRUE, 2);
  main_window.lint_view = gtk_tree_view_new ();
  gtk_container_add (GTK_CONTAINER (main_window.scrolledwindow1), main_window.lint_view);
  main_window.lint_renderer = gtk_cell_renderer_text_new ();
  main_window.lint_column = gtk_tree_view_column_new_with_attributes (_("Syntax Check Output"),
                main_window.lint_renderer, "text", 0, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (main_window.lint_view), main_window.lint_column);
  gtk_widget_set_size_request (main_window.lint_view, 80,80);
  main_window.lint_select = gtk_tree_view_get_selection (GTK_TREE_VIEW (main_window.lint_view));
  gtk_tree_selection_set_mode (main_window.lint_select, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (main_window.lint_select), "changed",
            G_CALLBACK (lint_row_activated), NULL);
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
  if (main_window.current_editor != NULL) {
    if (main_window.current_editor->type != TAB_HELP && main_window.current_editor->type != TAB_PREVIEW
        && main_window.current_editor->filename) {
      //debug("Full Name - %s, Short Name - %s", main_window.current_editor->filename->str, main_window.current_editor->short_filename);
      char *str = NULL;
      title = g_string_new(str);
      gchar *tmp=filename_parent_uri(main_window.current_editor->filename->str);
      dir = filename_get_relative_path(tmp);
      g_free(tmp);
      if (dir) {
        unquote(dir);
      } else {
        dir=g_strdup(".");
      }
      g_string_printf (title,"%s (%s)",main_window.current_editor->short_filename,dir);
      g_free(dir);
      //debug("Title - %s, Short name - %s", title->str, main_window.current_editor->short_filename);
      g_string_append(title, _(" - gPHPEdit"));
      if (!main_window.current_editor->saved) {
      //If the content is not saved then add a * to the begining of the title
        g_string_prepend(title, "*");
      }
      update_zoom_level();
      update_controls();
    }
    else if(main_window.current_editor->type == TAB_HELP) {
      title = g_string_new(_("Help: "));
      title = g_string_append(title, main_window.current_editor->help_function);
                        update_zoom_level();
                        update_controls();
    }
    else if(main_window.current_editor->type == TAB_PREVIEW) {
      title = g_string_new(_("Preview: "));
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

/****/

void main_window_update_reopen_menu(void)
{
  guint entry=0;
  GtkRecentManager *manager;
  manager = gtk_recent_manager_get_default ();
  GList *recent_items= gtk_recent_manager_get_items (manager);

  gtk_widget_show(main_window.menu->reciente);

  GList *walk = NULL;//recent_items;
  for (walk = recent_items;walk!=NULL;walk = g_list_next (walk)){
    if (entry==NUM_REOPEN_MAX) break;
    GtkRecentInfo *recent_info=(GtkRecentInfo *)walk->data;
    if (gtk_recent_info_has_application (recent_info,"gPHPEdit")){ /* only show our files */
      const gchar *full_filename= gtk_recent_info_get_uri (recent_info);
      if (full_filename){
        unquote((gchar *)full_filename);
        gtk_menu_item_set_label ((GtkMenuItem *)main_window.menu->recent[entry],full_filename);
        gtk_widget_show(main_window.menu->recent[entry]);
      }
      entry++;
    }
    gtk_recent_info_unref(recent_info);
  }
  g_list_free(recent_items);
  /* hide other items */
  guint hideitem;
  for (hideitem=entry;hideitem<NUM_REOPEN_MAX;hideitem++){
      gtk_widget_hide(main_window.menu->recent[hideitem]);
  }
  if (!gtk_widget_get_visible (main_window.menu->recent[0])){
    gtk_widget_hide(main_window.menu->reciente);
  }
}

void main_window_add_to_reopen_menu(gchar *full_filename)
{
  GtkRecentManager *manager;
  manager = gtk_recent_manager_get_default ();
  gtk_recent_manager_add_item (manager, full_filename);
  main_window_update_reopen_menu();
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
static void create_infobar(void){
  /* set up info bar */
  main_window.infobar= gtk_info_bar_new_with_buttons(_("Reload"), 1, _("Cancel"), 2, NULL);
  gtk_info_bar_set_message_type (GTK_INFO_BAR(main_window.infobar), GTK_MESSAGE_WARNING);
  main_window.infolabel = gtk_label_new("");
  gtk_label_set_line_wrap(GTK_LABEL (main_window.infolabel), TRUE);
  gtk_label_set_justify(GTK_LABEL (main_window.infolabel), GTK_JUSTIFY_FILL);
  GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR (main_window.infobar));
  gtk_box_set_spacing(GTK_BOX (content_area), 0);
  gtk_box_pack_start(GTK_BOX (content_area), main_window.infolabel, FALSE, FALSE, 0);
  gtk_widget_show(main_window.infolabel);
  g_signal_connect(main_window.infobar, "response", G_CALLBACK (process_external), main_window.current_editor);
  gtk_box_pack_start(GTK_BOX(main_window.prin_hbox), main_window.infobar, FALSE, FALSE, 0);
}

static void create_app_main_window(const gchar *title, gint height, gint width){
  main_window.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(main_window.window), title);
  gtk_window_set_default_size(GTK_WINDOW(main_window.window), height, width);
  /* center the window in the screen */
  gtk_window_set_position(GTK_WINDOW(main_window.window), GTK_WIN_POS_CENTER);
  g_set_application_name (title);
  gtk_window_set_default_icon_name ("gphpedit");
  /* set RGBA colormap */        
  set_colormap(main_window.window);
}

void main_window_create(void){
  create_app_main_window(_("gPHPEdit"), 230, 150);
  preferences_apply();
  main_window_create_prinbox();
  main_window_create_menu();
  main_window_create_maintoolbar();
  main_window_create_findtoolbar();

  main_window_create_panes();
  create_infobar();
  main_window_fill_panes();
  main_window_create_appbar();
  main_window_update_reopen_menu();
  
  plugin_setup_menu();
  function_list_prepare();
  css_function_list_prepare();

  g_signal_connect (G_OBJECT (main_window.window), "delete_event", G_CALLBACK(main_window_delete_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "destroy", G_CALLBACK (main_window_destroy_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "key_press_event", G_CALLBACK (main_window_key_press_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (main_window_resize), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "window-state-event", G_CALLBACK (main_window_state_changed), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "focus-in-event", G_CALLBACK (main_window_activate_focus), NULL);
  
  main_window.clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  gtk_widget_show(main_window.window);
  
  update_app_title();
  // folder browser init
  if (preferences.showfolderbrowser) folderbrowser_create(&main_window);
}

void update_controls(void){
  if (GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    //activate toolbar items
    gtk_widget_set_sensitive (main_window.toolbar_main->button_cut, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_paste, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_undo, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_redo, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_replace, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_indent, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_main->button_unindent, TRUE);
    if (main_window.current_editor->isreadonly){
      gtk_widget_set_sensitive (main_window.toolbar_main->button_save, FALSE);
    } else {
      gtk_widget_set_sensitive (main_window.toolbar_main->button_save, TRUE);
    }
      
    gtk_widget_set_sensitive (main_window.toolbar_main->button_save_as, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_find->search_entry, TRUE);
    gtk_widget_set_sensitive (main_window.toolbar_find->goto_entry, TRUE);
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
    /* only show preview in html files */
    if (main_window.current_editor->contenttype){
      if (strcmp(main_window.current_editor->contenttype,"text/html")==0){
        gtk_widget_set_sensitive (main_window.menu->preview, TRUE);
      } else {
        gtk_widget_set_sensitive (main_window.menu->preview, FALSE);
      }
    } else {
      gtk_widget_set_sensitive (main_window.menu->preview, FALSE);
    }
      
  }else{
    if (WEBKIT_IS_WEB_VIEW(main_window.current_editor->help_view)){
      //deactivate toolbar items
      gtk_widget_set_sensitive (main_window.toolbar_main->button_cut, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_paste, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_undo, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_redo, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_replace, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_indent, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_unindent, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_save, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_main->button_save_as, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_find->search_entry, FALSE);
      gtk_widget_set_sensitive (main_window.toolbar_find->goto_entry, FALSE);
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
      gtk_widget_set_sensitive (main_window.menu->preview, FALSE);
      }
  }
}
