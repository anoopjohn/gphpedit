/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010, 2011 José Rostagno (for vijona.com.ar) 

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
#include "main_window_callbacks.h"
#include "templates.h"
#include "gvfs_utils.h"
#include "gedit-close-button.h"
#include "gphpedit-statusbar.h"
#include "syntax_check_window.h"
#include "filebrowser_ui.h"
#include "classbrowser.h"
#include "document.h"

void update_controls(Documentable *document);

MainWindow main_window;

void main_window_state_changed(GtkWidget *widget, GdkEventWindowState *event, gpointer user_data)
{
  set_preferences_manager_window_maximized(main_window.prefmg, GDK_WINDOW_STATE_MAXIMIZED && event->new_window_state);
}

void main_window_resize(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data) 
{
  if (!get_preferences_manager_window_maximized(main_window.prefmg)) {
    gint left, top;
    gtk_window_get_position(GTK_WINDOW(main_window.window), &left, &top);
    set_preferences_manager_window_size (main_window.prefmg, allocation->width, allocation->height);
    set_preferences_manager_window_position (main_window.prefmg, top, left);
  }
}

gboolean classbrowser_accept_size(GtkPaned *paned, gpointer user_data)
{
  if (gtk_paned_get_position(GTK_PANED(main_window.pmain_horizontal_pane)) != 0) {
    gint size = gtk_paned_get_position(GTK_PANED(main_window.pmain_horizontal_pane));
    g_object_set(main_window.prefmg, "side_panel_size", size, NULL);
  }
  return TRUE;
}

static GtkWidget *get_widget_from_builder(const gchar *name){
    g_return_val_if_fail(name!=NULL, NULL);
    return GTK_WIDGET(gtk_builder_get_object (main_window.pbuilder, name));
}

static void main_window_create_appbar(MainWindow *main_window)
{
  GtkWidget *statusbox = get_widget_from_builder("statusbox");
  gtk_widget_show (statusbox);
  main_window->appbar = gphpedit_statusbar_new ();
  gtk_box_pack_start(GTK_BOX(statusbox), main_window->appbar, FALSE, TRUE, 1);
  gphpedit_statusbar_set_zoom_level(GPHPEDIT_STATUSBAR(main_window->appbar), 100);
  if (get_preferences_manager_show_statusbar(main_window->prefmg)) gtk_widget_show (main_window->appbar);
}

static void side_panel_show(void)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gint size;
  g_object_get(main_window.prefmg, "side_panel_size", &size, NULL);
  gtk_paned_set_position(GTK_PANED(main_window.pmain_horizontal_pane), size);
  g_object_set(main_window.prefmg, "side_panel_hidden", FALSE, NULL);
  classbrowser_update(GPHPEDIT_CLASSBROWSER(main_window.pclassbrowser));
}


static void side_panel_hide(void)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gtk_paned_set_position(GTK_PANED(main_window.pmain_horizontal_pane), 0);
  g_object_set(main_window.prefmg, "side_panel_hidden", TRUE, NULL);
}

static void side_panel_togle(GtkWidget *widget, gpointer user_data)
{
    MainWindow *main_window = (MainWindow *) user_data;
    side_panel_show_hide(main_window);
}

void side_panel_show_hide(MainWindow *main_window)
{
    gboolean hidden;
    g_object_get(main_window->prefmg, "side_panel_hidden", &hidden, NULL);
    menubar_set_classbrowser_status(MENUBAR(main_window->pmenu), hidden);
    if (hidden)
        side_panel_show();
    else
        side_panel_hide();
}

/*
 * tog_statusbar
 * Show/hide application statusbar
*/
void statusbar_show_hide(MainWindow *main_window, gboolean state)
{
    set_preferences_manager_show_statusbar(main_window->prefmg, state);
    gtk_widget_set_visible(main_window->appbar, state);
}

void maintoolbar_show_hide(MainWindow *main_window, gboolean state)
{
    set_preferences_manager_show_maintoolbar(main_window->prefmg, state);
    gtk_widget_set_visible(main_window->toolbar_main, state);
}

void fullscreen_show_hide(MainWindow *main_window, gboolean state)
{
    if (state) {
        gtk_window_fullscreen (GTK_WINDOW(main_window->window));
    } else {
        gtk_window_unfullscreen (GTK_WINDOW(main_window->window));
    }
}

void syntax_check_show(MainWindow *main_window)
{
  Documentable *doc = document_manager_get_current_documentable(main_window->docmg);
  gtk_syntax_check_window_run_check(GTK_SYNTAX_CHECK_WINDOW(main_window->pwin), doc, main_window);
  gtk_paned_set_position(GTK_PANED(main_window->pmain_vertical_pane), 200);
}

void syntax_check_hide(MainWindow *main_window)
{
  gint max;
  g_object_get(main_window->pmain_vertical_pane, "max-position", &max, NULL);
  gtk_paned_set_position(GTK_PANED(main_window->pmain_vertical_pane), max);
}

static void main_window_create_panes(void)
{
  gboolean hidden;
  gint size;

  main_window.pmain_horizontal_pane = get_widget_from_builder("main_horizontal_pane");
  main_window.pmain_vertical_pane = get_widget_from_builder("main_vertical_pane");
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (classbrowser_accept_size), NULL);

  g_object_get(main_window.prefmg, "side_panel_hidden", &hidden,"side_panel_size", &size, NULL);
  gtk_paned_set_position(GTK_PANED(main_window.pmain_horizontal_pane), size);
  if (hidden) side_panel_hide();
}


static void create_side_panel(void){
  /* Close button for the side bar */
  GtkWidget *close_box = get_widget_from_builder("sidepanelheader");
  main_window.pclose_sidebar_button = gedit_close_button_new();
  gtk_widget_set_tooltip_text(main_window.pclose_sidebar_button, _("Close side panel"));
  g_signal_connect(G_OBJECT(main_window.pclose_sidebar_button), "clicked", G_CALLBACK (side_panel_togle), &main_window);
  gtk_widget_show(main_window.pclose_sidebar_button);
  gtk_box_pack_end(GTK_BOX(close_box), main_window.pclose_sidebar_button, FALSE, FALSE, 0);

  main_window.notebook_manager = get_widget_from_builder("notebook_manager");
  
  /* Classbrowser stuff creation */  
  GtkWidget *classbox = get_widget_from_builder("classbox");
  main_window.pclassbrowser = gphpedit_classbrowser_new (); 
  gtk_widget_show(main_window.pclassbrowser);
  gtk_box_pack_start(GTK_BOX(classbox), main_window.pclassbrowser, TRUE, TRUE, 0);
  /* File browser stuff creation */
  if (get_preferences_manager_show_filebrowser(main_window.prefmg)){
  main_window.pfolder= gphpedit_filebrowser_new();
  gtk_widget_show(main_window.pfolder);
  GtkWidget *label= gtk_image_new_from_icon_name ("file-manager", GTK_ICON_SIZE_SMALL_TOOLBAR);
  gtk_widget_show(label);
  gtk_notebook_insert_page (GTK_NOTEBOOK(main_window.notebook_manager), main_window.pfolder, label, 1);
  }
}

static void main_window_fill_panes(void)
{
  /* create side panel */
  create_side_panel();

  GtkWidget *pribox = get_widget_from_builder("pribox");
  main_window.notebook_editor = gtk_notebook_new ();
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(main_window.notebook_editor), TRUE);
  gtk_widget_show (main_window.notebook_editor);
  gtk_box_pack_start(GTK_BOX(pribox), main_window.notebook_editor, TRUE, TRUE, 2);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "switch_page", G_CALLBACK (on_notebook_switch_page), NULL);
  g_signal_connect (G_OBJECT (main_window.notebook_editor), "focus-tab", G_CALLBACK (on_notebook_focus_tab), NULL);

  /* add syntax check window */
  GtkWidget *prin_hbox = get_widget_from_builder("prin_hbox");
  main_window.pwin = gtk_syntax_check_window_new ();
  gtk_box_pack_start(GTK_BOX(prin_hbox), GTK_WIDGET(main_window.pwin), TRUE, TRUE, 2);
  gtk_syntax_check_window_run_check(GTK_SYNTAX_CHECK_WINDOW(main_window.pwin), NULL, &main_window);
  gtk_widget_show (main_window.pwin);

  //FIXME: hack to hide syntax pane on start-up
  gtk_paned_set_position(GTK_PANED(main_window.pmain_vertical_pane), 10000);
}

void update_zoom_level(MainWindow *main_window, Documentable *doc)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  guint zoom_level = 100;
  if (doc) g_object_get(doc, "zoom_level", &zoom_level, NULL);
  gphpedit_statusbar_set_zoom_level(GPHPEDIT_STATUSBAR(main_window->appbar), zoom_level);
}

static void update_status_combobox(MainWindow *main_window, Documentable *document)
{
      if (main_window->is_app_closing) return ;
      gint type = -1;
      if (document) g_object_get(document, "type", &type, NULL);
      /* set statuscombo */
      switch(type) {
        case(TAB_PHP):   
         set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("PHP/HTML/XML"));          
         break;
        case (TAB_CSS):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("CSS"));
          break;
        case (TAB_CXX):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("C/C++"));
          break;
        case (TAB_COBOL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("Cobol"));
          break;
        case (TAB_SQL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("SQL"));
          break;
        case (TAB_PERL):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("Perl"));
          break;
        case (TAB_PYTHON):
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("Python"));
          break;
        default:
          set_status_combo_item (GPHPEDIT_STATUSBAR(main_window->appbar),_("Text-Plain"));
      }
}

/**
 * Update the application title when switching tabs, closing or opening
 * or opening new tabs or opening new files.
 */
void update_app_title(MainWindow *main_window, Documentable *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gchar *title = NULL;
  if (document) g_object_get(document, "title", &title, NULL);
  update_status_combobox(main_window, document);
  update_zoom_level(main_window, document);
  update_controls(document);
  //If there is no file opened set the name as gPHPEdit
  if (!title) title = g_strdup(_("gPHPEdit"));
  gtk_window_set_title(GTK_WINDOW(main_window->window), title);
  g_free(title);
}

static void document_manager_zoom_change_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
    MainWindow *main_window = (MainWindow *) user_data;
    update_zoom_level(main_window, doc);
}

/**
 * Close a tab in the Editor. Removes the notebook page,
 * and sets the active tab correcty
 * 
 * @param editor - The editor object corresponding to the tab that is going to be closed.
 * @return void
 */

static void close_page(MainWindow *main_window, Document *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  gint page_num;
  gint page_num_closing;
  gint current_active_tab;
  GtkWidget *document_widget;

  g_object_get(document, "editor_widget", &document_widget, NULL);
  page_num_closing = gtk_notebook_page_num(GTK_NOTEBOOK(main_window->notebook_editor), document_widget);
  current_active_tab = gtk_notebook_get_current_page(GTK_NOTEBOOK(main_window->notebook_editor));
  
  if (page_num_closing != current_active_tab) {
    page_num = current_active_tab;
  } else {
    // If there is a tab before the current one then set it as the active tab.
    page_num = page_num_closing - 1;
    // If the current tab is the 0th tab then set the current tab as 0 itself.
    // If there are are subsequent tabs, then this will set the next tab as active.
    if (page_num < 0) {
      page_num = 0;
    }  
  }

  if(document_manager_set_current_document_from_position(main_window->docmg, page_num)) {
    gtk_notebook_set_current_page(GTK_NOTEBOOK(main_window->notebook_editor), page_num);
  }
  gtk_notebook_remove_page(GTK_NOTEBOOK(main_window->notebook_editor), page_num_closing);
}

static void document_manager_close_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  MainWindow *main_window = (MainWindow *) user_data;
  close_page(main_window, DOCUMENT(doc));
  update_app_title(main_window, document_manager_get_current_documentable(docmg));
  gchar *filename = documentable_get_filename (doc);
  gint ftype;
  g_object_get(doc, "type", &ftype, NULL);
  symbol_manager_purge_file (main_window->symbolmg, filename, ftype);
  g_free(filename);
}

static void document_manager_new_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
  MainWindow *main_window = (MainWindow *) user_data;
  gint ftype;
  g_object_get(doc, "type", &ftype, NULL);
  gchar *filename = documentable_get_filename(doc);
  symbol_manager_add_file (main_window->symbolmg, filename, ftype);
  g_free(filename);
}

static void document_manager_change_document_cb (DocumentManager *docmg, Documentable *doc, gpointer user_data)
{
    MainWindow *main_window = (MainWindow *) user_data;
    if (!main_window->is_app_closing) {
        update_app_title(main_window, doc);
        documentable_check_externally_modified(doc);
    }
}

static void set_colormap(GtkWidget *window)
{
  /*Set RGBA visual*/
  GdkScreen *screen = gtk_widget_get_screen (GTK_WIDGET (window));
  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

  if (visual == NULL)
    visual = gdk_screen_get_system_visual (screen);

  gtk_widget_set_visual (window, visual);
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

void main_window_create(char **argv, gint argc)
{
  main_window.is_app_closing = FALSE;
  main_window.prefmg = preferences_manager_new();
  create_app_main_window(_("gPHPEdit"));

  main_window.pbuilder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (main_window.pbuilder, GPHPEDIT_UI_DIR "/gphpedit.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }

  GtkWidget *prinbox = get_widget_from_builder("prinbox");
  gtk_widget_reparent (prinbox, GTK_WIDGET(main_window.window));

  /* add menu bar to main window */
  GtkWidget *menubox = get_widget_from_builder("menubox");
  gtk_widget_show (menubox);

  main_window.pmenu_hints = g_hash_table_new_full (g_direct_hash, g_direct_equal, NULL, NULL);

  main_window.pmenu = menubar_new (&main_window);
  gtk_box_pack_start (GTK_BOX (menubox), main_window.pmenu, FALSE, FALSE, 0);
  gtk_widget_show_all (main_window.pmenu);

  GtkWidget *toolbox = get_widget_from_builder("toolbox");
  gtk_widget_show (toolbox);

  main_window.toolbar_main = toolbar_new ();
  gtk_box_pack_start (GTK_BOX (toolbox), main_window.toolbar_main, FALSE, FALSE, 0);
  if (get_preferences_manager_show_maintoolbar(main_window.prefmg)) gtk_widget_show (main_window.toolbar_main);

  main_window.symbolmg = symbol_manager_new();

  main_window_create_panes();
  main_window_fill_panes();
  main_window_create_appbar(&main_window);
  
  g_signal_connect (G_OBJECT (main_window.window), "delete_event", G_CALLBACK(main_window_delete_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "destroy", G_CALLBACK (main_window_destroy_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "key_press_event", G_CALLBACK (main_window_key_press_event), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "size_allocate", G_CALLBACK (main_window_resize), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "window-state-event", G_CALLBACK (main_window_state_changed), NULL);
  g_signal_connect (G_OBJECT (main_window.window), "focus-in-event", G_CALLBACK (main_window_activate_focus), NULL);

  main_window.stylemg = gtk_source_style_scheme_manager_new ();
  gchar *theme_dir = g_build_path (G_DIR_SEPARATOR_S, API_DIR, "themes", NULL);
  gtk_source_style_scheme_manager_prepend_search_path (main_window.stylemg, theme_dir);
  g_free(theme_dir);

  main_window.tempmg = templates_manager_new();
  main_window.docmg = document_manager_new_full(argv, argc, &main_window);
  g_signal_connect (G_OBJECT (main_window.docmg), "new_document", G_CALLBACK(document_manager_new_document_cb), &main_window);
  g_signal_connect (G_OBJECT (main_window.docmg), "change_document", G_CALLBACK(document_manager_change_document_cb), &main_window);
  g_signal_connect (G_OBJECT (main_window.docmg), "close_document", G_CALLBACK(document_manager_close_document_cb), &main_window);
  g_signal_connect (G_OBJECT (main_window.docmg), "zoom_change", G_CALLBACK(document_manager_zoom_change_cb), &main_window);

  update_app_title(&main_window, document_manager_get_current_documentable(main_window.docmg));

  gtk_widget_show(main_window.window);
}

void update_controls(Documentable *document)
{
  gphpedit_debug(DEBUG_MAIN_WINDOW);
  if (!document) return ;
  gboolean read_only, can_modify, preview;
  g_object_get(document, "read_only", &read_only, "can_modify", &can_modify, "can_preview", &preview, NULL);
  menubar_update_controls(MENUBAR(main_window.pmenu), can_modify, preview, read_only);
  toolbar_update_controls(TOOLBAR(main_window.toolbar_main), can_modify, read_only);
}

/*
 *show_hint
 * Show a new menu hint in the statusbar, and set widget state to prelight
*/
static gboolean show_hint(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data)
{
    MainWindow *main_window = (MainWindow *) user_data;
    gtk_widget_set_state (widget, GTK_STATE_PRELIGHT);
    const gchar *message = (const gchar *) g_hash_table_lookup (main_window->pmenu_hints, widget);
    if (!message)
    {
        gphpedit_debug_message (DEBUG_MAIN_WINDOW,"%s", "Error menu hint is Null, skipping");
        return FALSE;
    }
    main_window->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR(main_window->appbar), message);
    main_window->message_id= gtk_statusbar_push (GTK_STATUSBAR(main_window->appbar), main_window->context_id, message);
    return FALSE;
}

/*
 *delete_hint
 * deletes the menu hint from the statusbar, and set widget state to normal
*/
static gboolean delete_hint(GtkWidget *widget, GdkEventCrossing *event, gpointer user_data){
    MainWindow *main_window = (MainWindow *) user_data;
    gtk_widget_set_state (widget, GTK_STATE_NORMAL);
    gtk_statusbar_remove (GTK_STATUSBAR(main_window->appbar), main_window->context_id, main_window->message_id);
    return FALSE;
}

/*
 * install_menu_hint
 * connect menu hint signals
*/
void main_window_install_menu_hint(MainWindow *main_window, GtkWidget *widget, gchar *message)
{
    g_return_if_fail(widget!=NULL);
    g_return_if_fail(message!=NULL);

    g_hash_table_insert (main_window->pmenu_hints, widget, message);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(show_hint), main_window);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(delete_hint), main_window);
}
