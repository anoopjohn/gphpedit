/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno(for vijona.com.ar)

   For more information or to find the latest release, visit our
   website at http://www.gphpedit.org/

   gPHPEdit is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   gPHPEdit is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.

   The GNU General Public License is contained in the file COPYING.
*/
/* ******* FILEBROWSER DESIGN ********
File browser has a treeview to show the directory struct. This treeview has a column for the pixmap
and the name (both shown to the user), but also the mimetype of the file (not shown). 

The filebrowser has two parts: the ui and the backend. the backend do all file operations.

File browser has the following features:
-Remember last folder
-Autorefresh when directory contents changes
-Popup menu
-AutoSort (directories go before files, files are sorted by name and at last by extension)
-Double click in treeview open files and expand/collapse treerow for directories
-Keypress capture: press delete will delete current selected file/folder; press enter will open current selected file/folder
-Only displays files which could be opened by the editor
-Drag and drop: if you drop uri into the file browser these files will be copied to current file browser folder
*/
#include <gdk/gdkkeysyms.h>

#include "filebrowser_ui.h"
#include "filebrowser_backend.h"
#include "main_window.h"
#include "debug.h"
/* functions */
static void gphpedit_file_browser_class_init (gphpeditFileBrowserClass *klass);
static void gphpedit_filebrowser_init (gphpeditFileBrowser *button);
gint _filebrowser_sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b, gpointer user_data);
void tree_double_clicked(GtkTreeView *tree_view,GtkTreePath *path,GtkTreeViewColumn *column,gpointer user_data);
gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data);
GtkWidget *_get_image_button(const gchar *type,const gchar *tooltip);
void _go_home_cb (GtkButton *button, gpointer   user_data);
void _go_up_cb (GtkButton *button, gpointer   user_data); 
void _button_refresh (GtkButton *button, gpointer   user_data);
void pressed_button_file_chooser(GtkButton *widget, gpointer data);
static inline void search_control_sensible(gphpeditFileBrowserPrivate *priv, gboolean status);
void change_folder_cb (gpointer instance, const gchar *current_folder, gpointer user_data);
gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) ;
gboolean view_onPopupMenu (GtkWidget *treeview, gpointer userdata);
void popup_delete_file(void);
gboolean  cancel_process (GtkWidget *widget, GdkEvent  *event, gpointer   user_data);
static void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data);
static void on_search_press (const gchar *filename, gpointer user_data);
static void search_typed (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data);
static void search_activate(GtkEntry *entry,gpointer user_data);
void     print_files        (FilebrowserBackend         *directory, gpointer user_data);

void fb_file_v_drag_data_received(GtkWidget * widget, GdkDragContext * context, gint x,  gint y, GtkSelectionData * data, guint info, guint time,gpointer user_data);
#define MIME_ISDIR(string) (g_strcmp0(string, "inode/directory")==0)

static gchar *get_mime_from_tree(GtkTreeView *tree_view);
static gchar *get_path_from_tree(GtkTreeView *tree_view, gchar *root_path);

struct _gphpeditFileBrowserPrivate
{
  FilebrowserBackend *fbbackend;

  GtkWidget *folder;  //filebrowser
  GtkTreeStore *pTree; 
  GtkWidget *pListView;
  GtkWidget *button_dialog;
  GtkWidget *button_home;
  GtkWidget *button_up;
  GtkWidget *searchentry;
  GtkWidget *button_refresh;
  GtkWidget *image_refresh;

  GtkTreeModel *cache_model;
  gulong  handlerid;
  gulong  handleridchange;
};

enum {
  ICON_COLUMN,
  FILE_COLUMN,
  MIME_COLUMN,
  N_COL
};

enum {
  TARGET_URI_LIST,
  TARGET_STRING
};

const GtkTargetEntry drag_dest_types[] = {
    {"text/uri-list", 0, TARGET_URI_LIST},
    {"STRING", 0, TARGET_STRING},
};

typedef struct {
  gchar *filename;
  const gchar *mime;
  FilebrowserBackend *fbbackend;
} POPUPDATA;
POPUPDATA pop;

static gpointer gphpedit_file_browser_parent_class;

#define FILEBROWSER_BACKEND_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_FILEBROWSER,\
					    gphpeditFileBrowserPrivate))


GType
gphpedit_filebrowser_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (gphpeditFileBrowserClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gphpedit_file_browser_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (gphpeditFileBrowser),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gphpedit_filebrowser_init,
        };

        our_type = g_type_register_static (GTK_TYPE_VBOX, "gphpeditFileBrowser",
                                           &our_info, 0);
  }
    
    return our_type;
}

static void
gphpedit_file_browser_destroy (GtkObject *object)
{
	gphpeditFileBrowserPrivate *priv;

	priv = FILEBROWSER_BACKEND_GET_PRIVATE(object);
//	gphpedit_file_browser_set_enable_completion (GPHPEDIT_FILEBROWSER (object), FALSE);
  if (g_signal_handler_is_connected (priv->fbbackend, priv->handlerid)){
   g_signal_handler_disconnect(priv->fbbackend, priv->handlerid);
  }
  if (g_signal_handler_is_connected (priv->fbbackend, priv->handleridchange)){
   g_signal_handler_disconnect(priv->fbbackend,   priv->handleridchange);
  }
  filebrowser_backend_cancel (priv->fbbackend);

	GTK_OBJECT_CLASS (gphpedit_file_browser_parent_class)->destroy (object);
}

static void
gphpedit_file_browser_finalize (GObject *object)
{
	gphpeditFileBrowserPrivate *priv;

	priv = FILEBROWSER_BACKEND_GET_PRIVATE(object);

	G_OBJECT_CLASS (gphpedit_file_browser_parent_class)->finalize (object);
}

static void 
gphpedit_file_browser_class_init (gphpeditFileBrowserClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
	gphpedit_file_browser_parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = gphpedit_file_browser_finalize;
	gtkobject_class->destroy = gphpedit_file_browser_destroy;
	
	g_type_class_add_private (object_class, sizeof(gphpeditFileBrowserPrivate));
}
/*
* gphpedit_filebrowser_init
* init a new filebrowser object
* creates all filebrowser widgets and the filebrowser backend
*/
static void
gphpedit_filebrowser_init (gphpeditFileBrowser *button)
{
  gphpeditFileBrowserPrivate *priv = FILEBROWSER_BACKEND_GET_PRIVATE(button);
  gchar *fol = g_strdup(get_preferences_manager_filebrowser_last_folder(main_window.prefmg));
  priv->fbbackend= filebrowser_backend_new (fol);//get_preferences_manager_filebrowser_last_folder(main_window.prefmg));
  g_free(fol);
  priv->handlerid = g_signal_connect(G_OBJECT(priv->fbbackend), "done_loading", G_CALLBACK(print_files), priv);
  priv->handleridchange = g_signal_connect(G_OBJECT(priv->fbbackend), "change_folder", G_CALLBACK(change_folder_cb), priv);
  priv->folder = gtk_vbox_new(FALSE, 0);

  GtkTreeViewColumn *pColumn;
  GtkCellRenderer  *pCellRenderer;
  priv->pTree = gtk_tree_store_new(N_COL, GDK_TYPE_PIXBUF, G_TYPE_STRING,G_TYPE_STRING);
  priv->pListView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(priv->pTree));
  pCellRenderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(priv->pListView),-1,"",pCellRenderer, "pixbuf", ICON_COLUMN, NULL);

  GtkWidget *pScrollbar = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(pScrollbar),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(pScrollbar), priv->pListView);

  //renderer for text
  g_signal_connect(G_OBJECT(priv->pListView), "row-activated", G_CALLBACK(tree_double_clicked), priv);
  g_signal_connect(G_OBJECT(priv->pListView), "button-press-event", (GCallback) view_onButtonPressed, priv->fbbackend);
  g_signal_connect(G_OBJECT(priv->pListView), "popup-menu", (GCallback) view_onPopupMenu, priv->fbbackend);
  g_signal_connect(G_OBJECT(priv->pListView), "key-press-event", G_CALLBACK(key_press), NULL);
  gtk_drag_dest_set(priv->pListView, (GTK_DEST_DEFAULT_ALL), drag_dest_types, 2,
    (GDK_ACTION_DEFAULT | GDK_ACTION_COPY));
  g_signal_connect(G_OBJECT(priv->pListView), "drag_data_received", G_CALLBACK(fb_file_v_drag_data_received), priv->fbbackend);

  pCellRenderer=NULL;
  pCellRenderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(priv->pListView),-1,_("File"),pCellRenderer, "text", FILE_COLUMN, NULL);
  pCellRenderer=NULL;
  pCellRenderer = gtk_cell_renderer_text_new();
  pColumn = gtk_tree_view_column_new_with_attributes(_("Mime"), pCellRenderer,"text",MIME_COLUMN,NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(priv->pListView), pColumn);
  gtk_tree_view_column_set_visible (pColumn,FALSE);
  gtk_widget_show(priv->folder);
  priv->button_dialog = gtk_button_new_with_label (get_filebrowser_backend_current_folder(priv->fbbackend));
  g_signal_connect(G_OBJECT(priv->button_dialog), "pressed", G_CALLBACK(pressed_button_file_chooser), priv->fbbackend);
  gtk_widget_set_size_request (priv->pListView,80,450);

  /* home and up buttons*/
  GtkWidget *hbox2;
  hbox2 = gtk_hbox_new(FALSE, 0);
  /* home button */
  priv->button_home= _get_image_button(GTK_STOCK_HOME,_("Go Home Dir"));
  gtk_widget_show(priv->button_home);
  gtk_box_pack_start(GTK_BOX(hbox2), priv->button_home, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(priv->button_home), "clicked", G_CALLBACK (_go_home_cb),priv->fbbackend);

  /* up button */
  priv->button_up= _get_image_button(GTK_STOCK_GO_UP,_("Go Up one level"));
  gtk_widget_show(priv->button_up);
  gtk_box_pack_start(GTK_BOX(hbox2), priv->button_up, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(priv->button_up), "clicked", G_CALLBACK (_go_up_cb), priv->fbbackend);

  /* refresh button */
  priv->button_refresh= _get_image_button(GTK_STOCK_REFRESH,_("Refresh Filebrowser"));
  gtk_widget_show(priv->button_refresh);
  gtk_box_pack_start(GTK_BOX(hbox2), priv->button_refresh, TRUE, TRUE, 0);
  g_signal_connect(G_OBJECT(priv->button_refresh), "clicked", G_CALLBACK (_button_refresh), priv->fbbackend);

  priv->searchentry = gtk_entry_new();
  gtk_widget_show(priv->searchentry);
  gtk_widget_set_tooltip_text (priv->searchentry,_("Search Files in Folder Browser"));
  gtk_entry_set_icon_from_stock (GTK_ENTRY(priv->searchentry),GTK_ENTRY_ICON_SECONDARY,GTK_STOCK_CLEAR);
  gtk_entry_set_icon_from_stock (GTK_ENTRY(priv->searchentry),GTK_ENTRY_ICON_PRIMARY,GTK_STOCK_FIND);
  g_signal_connect (G_OBJECT (priv->searchentry), "icon-press", G_CALLBACK (on_cleanicon_press), priv);
  g_signal_connect_after(G_OBJECT(priv->searchentry), "insert_text", G_CALLBACK(search_typed), priv);
  g_signal_connect_after(G_OBJECT(priv->searchentry), "backspace", G_CALLBACK(search_activate), priv);
  g_signal_connect_after(G_OBJECT(priv->searchentry), "activate", G_CALLBACK(search_activate), priv);

  search_control_sensible(priv,TRUE);
  gtk_box_pack_start(GTK_BOX(hbox2), priv->searchentry, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(priv->folder), hbox2, FALSE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(priv->folder), priv->button_dialog, FALSE, FALSE, 2);
  gtk_box_pack_start(GTK_BOX(priv->folder), pScrollbar, TRUE, TRUE, 2);
  gtk_widget_show(priv->button_dialog);
  gtk_widget_show_all(priv->folder);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(priv->pTree), 1, _filebrowser_sort_func, NULL, NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(priv->pTree), 1, GTK_SORT_ASCENDING);
  gtk_box_pack_start(GTK_BOX(button), priv->folder, TRUE, TRUE, 2);
}
/*
* gphpedit_filebrowser_new
* return a new filebrowser widget
*/
GtkWidget *
gphpedit_filebrowser_new ()
{
	gphpeditFileBrowser *button;

	button = g_object_new (GPHPEDIT_TYPE_FILEBROWSER, NULL);

	return GTK_WIDGET (button);
}

/*
 * filebrowser_sort_func (internal)
 *
 * this function is the sort function, and has the following features:
 * - directories go before files
 * - files are first sorted without extension, only equal names are sorted by extension
 *
 */
gint _filebrowser_sort_func(GtkTreeModel * model, GtkTreeIter * a, GtkTreeIter * b,
               gpointer user_data)
{
  gchar *namea, *nameb, *mimea, *mimeb;
  gboolean isdira, isdirb;
  gint retval = 0;
  gtk_tree_model_get(GTK_TREE_MODEL (model), a, FILE_COLUMN, &namea,MIME_COLUMN, &mimea,-1);
  gtk_tree_model_get(GTK_TREE_MODEL (model), b, FILE_COLUMN, &nameb,MIME_COLUMN, &mimeb,-1);
  isdira = (mimea && MIME_ISDIR(mimea));
  isdirb = (mimeb && MIME_ISDIR(mimeb));
  gphpedit_debug_message(DEBUG_FILEBROWSER, "isdira=%d, mimea=%s, isdirb=%d, mimeb=%s",isdira,mimea,isdirb,mimeb);
  if (isdira == isdirb) {    /* both files, or both directories */
    if (namea == nameb) {
      retval = 0;      /* both NULL */
    } else if (namea == NULL || nameb == NULL) {
      retval = (namea - nameb);
    } else {        /* sort by name, first without extension */
      gchar *dota, *dotb;
      dota = strrchr(namea, '.');
      dotb = strrchr(nameb, '.');
      if (dota)
        *dota = '\0';
      if (dotb)
        *dotb = '\0';
      retval = strcmp(namea, nameb);
      if (retval == 0) {
        if (dota)
          *dota = '.';
        if (dotb)
          *dotb = '.';
        retval = strcmp(namea, nameb);
      }
    }
  } else {          /* a directory and a file */
    retval = (isdirb - isdira);
  }
  g_free(namea);
  g_free(nameb);
  g_free(mimea);
  g_free(mimeb);
  return retval;
}

/*
* _get_image_button (internal)
* helper function to create button with an image
*/
GtkWidget *_get_image_button(const gchar *type,const gchar *tooltip)
{
  GtkWidget *button= gtk_button_new();
  GtkWidget *image_button = gtk_image_new_from_stock (type,GTK_ICON_SIZE_BUTTON);
  gtk_widget_show(image_button);
  gtk_button_set_image(GTK_BUTTON(button), image_button);
  gtk_widget_show(button);
  gtk_widget_set_tooltip_text (button,tooltip);
  return button;
}
/*
* _go_home_cb(internal)
* go to the current editor folder.
*/
void _go_home_cb (GtkButton *button, gpointer   user_data) 
{
    /*if there is a file open set file folder as home dir*/
    gchar *folderpath= document_get_filename(document_manager_get_current_document(main_window.docmg));
    filebrowser_backend_go_folder_home (FILEBROWSER_BACKEND(user_data), folderpath);
    if (folderpath) g_free(folderpath);
}

/*
* go up one level in directory tree (internal)
*/
void _go_up_cb (GtkButton *button, gpointer   user_data) 
{
  filebrowser_backend_go_folder_up (FILEBROWSER_BACKEND(user_data));
}
/*
* refresh filebrowser (internal)
*/
void _button_refresh (GtkButton *button, gpointer   user_data) 
{
filebrowser_backend_refresh_folder (FILEBROWSER_BACKEND(user_data));
}


/*
 * tree_double_clicked (internal)
 *
 *   this function handles double click signal of filebrowser treeview:
 *   if selected item is a file then open it
 *   if selected file is a directory force expand/collapse treerow
 */
void tree_double_clicked(GtkTreeView *tree_view,GtkTreePath *path,GtkTreeViewColumn *column,gpointer user_data)
 {
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  gchar *mime=get_mime_from_tree(tree_view);
  gchar *folderpath= (gchar*)get_filebrowser_backend_current_folder(priv->fbbackend);
  gchar *file_name=get_path_from_tree(tree_view,folderpath);
  gphpedit_debug_message(DEBUG_FILEBROWSER,"DOUBLECLICK\t mime:%s\tname:%s\n",mime,file_name);
  if (!MIME_ISDIR(mime)){
      filebrowser_backend_open_file (priv->fbbackend, file_name);
  } else {
    filebrowser_backend_update_folder(priv->fbbackend, file_name);
  }
  g_free(mime);
  g_free(file_name);
}
/*
* process keypress event in file browser (internal)
*/

gboolean key_press (GtkWidget *widget, GdkEventKey *event, gpointer user_data){
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  gphpedit_debug_message(DEBUG_FILEBROWSER, "keyval:%d \n",event->keyval);

  if (event->keyval==GDK_Delete || event->keyval==GDK_Return){
    gchar *mime=get_mime_from_tree(GTK_TREE_VIEW(widget));
    gchar *path=(gchar*)get_filebrowser_backend_current_folder(priv->fbbackend);
    gchar *file_name=get_path_from_tree(GTK_TREE_VIEW(widget),path);
    if (event->keyval==GDK_Delete){
      //delete file
      pop.fbbackend=priv->fbbackend;
      pop.filename=file_name;
      pop.mime=mime;
      popup_delete_file();
    }else {
      //open file
      if (!MIME_ISDIR(mime))
        filebrowser_backend_open_file (priv->fbbackend, file_name);
    }
    return TRUE;
  }
  return FALSE;
}

static gchar *get_path_from_tree(GtkTreeView *tree_view, gchar *root_path){
  GtkTreeModel *model;
  GtkTreeSelection *select;
  GtkTreeIter iter;
  select = gtk_tree_view_get_selection(tree_view);
  if(gtk_tree_selection_get_selected (select, &model, &iter)) {
    gchar *nfile;
    gtk_tree_model_get (model, &iter,FILE_COLUMN, &nfile, -1);
    GtkTreeIter* parentiter=(GtkTreeIter*)g_malloc(sizeof(GtkTreeIter));
    while(gtk_tree_model_iter_parent(model,parentiter,&iter)){
      gchar *rom;
      gtk_tree_model_get (model, parentiter, FILE_COLUMN, &rom, -1);
      nfile = g_build_path (G_DIR_SEPARATOR_S, rom, nfile, NULL);
      iter=*parentiter;
      g_free(parentiter);
      parentiter=(GtkTreeIter*)g_malloc(sizeof(GtkTreeIter));
     g_free(rom);
    }
    g_free(parentiter);
    gchar* file_name = g_build_path (G_DIR_SEPARATOR_S, root_path, nfile, NULL);
    g_free(nfile);
    return file_name;
  }
  return NULL;
}

static gchar *get_mime_from_tree(GtkTreeView *tree_view){
  GtkTreeModel *model;
  GtkTreeSelection *select;
  GtkTreeIter iter;
  gchar *tmime;
  select = gtk_tree_view_get_selection(tree_view);
  if(gtk_tree_selection_get_selected (select, &model, &iter)) {
    gtk_tree_model_get (model, &iter,MIME_COLUMN,&tmime, -1);
  return tmime;
  }
  return NULL;
}


static inline void search_control_sensible(gphpeditFileBrowserPrivate *priv, gboolean status){
    if (!priv) return ;
    gtk_widget_set_sensitive (priv->searchentry, status);
    gtk_entry_set_icon_sensitive (GTK_ENTRY(priv->searchentry), GTK_ENTRY_ICON_PRIMARY, status);
    gtk_entry_set_icon_sensitive (GTK_ENTRY(priv->searchentry), GTK_ENTRY_ICON_SECONDARY, status);
}

void pressed_button_file_chooser(GtkButton *widget, gpointer user_data) {

  GtkWidget *pFileSelection;

  pFileSelection = gtk_file_chooser_dialog_new("Open...", GTK_WINDOW(main_window.window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
  gtk_window_set_modal(GTK_WINDOW(pFileSelection), TRUE);
  gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER(pFileSelection), FALSE);
  gboolean res;
  res=gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(pFileSelection), get_filebrowser_backend_current_folder(FILEBROWSER_BACKEND(user_data)));
  gchar *sChemin=NULL;

  switch(gtk_dialog_run(GTK_DIALOG(pFileSelection))) {
         case GTK_RESPONSE_OK:
             sChemin = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(pFileSelection));
             break;
         default:
             break;
  }
  gtk_widget_destroy(pFileSelection);
  if(sChemin){
    /*store folder in config*/
    filebrowser_backend_update_folder (FILEBROWSER_BACKEND(user_data), sChemin);
    g_free(sChemin);
   }
}

/* POPUP menu functions */

void popup_open_file(void){
  filebrowser_backend_open_file (pop.fbbackend, pop.filename);
}

/*
 * popup_delete_file (internal)
 *
 *   This function is the delete function of the filebrowser popup menu, and has the following  features:
 * - Promp before delete the file
 * - Send file to trash if filesystem support that feature
 * - Delete file if filesystem don't support send to trash feature
 *
 */

void popup_delete_file(void){
  filebrowser_backend_delete_file(pop.fbbackend, (gchar *)pop.filename);
}

void popup_rename_file(void){
  gchar *current_name=filebrowser_backend_get_display_name(pop.fbbackend, (gchar *)pop.filename);
  GtkWidget *window;
  window = gtk_dialog_new_with_buttons(_("Rename File"), GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  GtkWidget *vbox1 = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))),vbox1);
  GtkWidget *hbox1 = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (vbox1),hbox1);
  GtkWidget *label1 = gtk_label_new (_("New Filename"));
  gtk_widget_show (label1);
  gtk_container_add (GTK_CONTAINER (hbox1),label1);
  GtkWidget *text_filename = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(text_filename),20);
  gtk_entry_set_width_chars(GTK_ENTRY(text_filename),21);
  gtk_entry_set_text (GTK_ENTRY(text_filename),current_name);
  gtk_widget_show (text_filename);
  gtk_container_add (GTK_CONTAINER (hbox1),text_filename);
  gint res=gtk_dialog_run(GTK_DIALOG(window));
  const char *name=gtk_entry_get_text (GTK_ENTRY(text_filename));
  if (res==GTK_RESPONSE_ACCEPT){
    filebrowser_backend_rename_file(pop.fbbackend, (gchar *) pop.filename,current_name, (gchar *)name);
  }
  gtk_widget_destroy(window);
  g_free(current_name);
}


void popup_create_dir(void){
  GtkWidget *window;
  window = gtk_dialog_new_with_buttons(_("New Dir"), GTK_WINDOW(main_window.window), GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
  GtkWidget *vbox1 = gtk_vbox_new (FALSE, 8);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (gtk_dialog_get_content_area(GTK_DIALOG(window))),vbox1);
  GtkWidget *hbox1 = gtk_hbox_new (FALSE, 8);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (vbox1),hbox1);
  GtkWidget *label1 = gtk_label_new (_("Directory Name"));
  gtk_widget_show (label1);
  gtk_container_add (GTK_CONTAINER (hbox1),label1);
  GtkWidget *text_filename = gtk_entry_new();
  gtk_entry_set_max_length (GTK_ENTRY(text_filename),20);
  gtk_entry_set_width_chars(GTK_ENTRY(text_filename),21);
  gtk_widget_show (text_filename);
  gtk_container_add (GTK_CONTAINER (hbox1),text_filename);
  gint res=gtk_dialog_run(GTK_DIALOG(window));
  const char *name=gtk_entry_get_text (GTK_ENTRY(text_filename));
  if (res==GTK_RESPONSE_ACCEPT && name){
    filebrowser_backend_create_dir(pop.fbbackend, (gchar *)pop.filename, (gchar *)name, MIME_ISDIR(pop.mime));
    }
    gtk_widget_destroy(window);
}


/*
 * view_popup_menu
 *
 *   This function shows a popup menu with the following features:
 * - Open File
 * - Rename File
 * - Delete File
 * - Create New directory
 *
 */

void view_popup_menu (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
  GtkWidget *menu, *menuopen,*menurename,*menudelete,*menucreate,*sep;
  menu = gtk_menu_new();

  menuopen = gtk_menu_item_new_with_label(_("Open file"));
  if (MIME_ISDIR(pop.mime)){
    gtk_widget_set_state (menuopen,GTK_STATE_INSENSITIVE);
  }else {
    g_signal_connect(menuopen, "activate", (GCallback) popup_open_file, &pop);
  }
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuopen);

  menurename = gtk_menu_item_new_with_label(_("Rename File"));
  g_signal_connect(menurename, "activate", (GCallback) popup_rename_file, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menurename);

  menudelete = gtk_menu_item_new_with_label(_("Delete file"));
  g_signal_connect(menudelete, "activate", (GCallback) popup_delete_file, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menudelete);

  sep = gtk_separator_menu_item_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), sep);
  
  menucreate = gtk_menu_item_new_with_label(_("Create New Directory"));
  g_signal_connect(menucreate, "activate", (GCallback) popup_create_dir, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu), menucreate);

  gtk_widget_show_all(menu);

  /* Note: event can be NULL here when called from view_onPopupMenu;
   *  gdk_event_get_time() accepts a NULL argument */
  gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
                   (event != NULL) ? event->button : 0,
                   gdk_event_get_time((GdkEvent*)event));
}


gboolean view_onButtonPressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
  /* single click with the right mouse button? */
  if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3){
    /* select row if no row is selected */
    GtkTreeSelection *selection;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    /* Note: gtk_tree_selection_count_selected_rows() does not
    *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
    if (gtk_tree_selection_count_selected_rows(selection)  <= 1){
      GtkTreePath *path;
      /* Get tree path for row that was clicked */
      if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview),
                                             (gint) event->x,
                                             (gint) event->y,
                                             &path, NULL, NULL, NULL)) {
      gtk_tree_selection_unselect_all(selection);
      gtk_tree_selection_select_path(selection, path);
      gtk_tree_path_free(path);
      }
    }
    gchar *mime=get_mime_from_tree(GTK_TREE_VIEW(treeview));
    gchar *path=(gchar*)get_filebrowser_backend_current_folder(userdata);
    gchar *file_name=get_path_from_tree(GTK_TREE_VIEW(treeview),path);
    pop.filename=file_name;
    pop.mime=mime;
    pop.fbbackend= userdata;
    view_popup_menu(treeview, event, NULL);
      return TRUE; /* we handled this */
    }
  return FALSE; /* we did not handle this */
}

gboolean view_onPopupMenu (GtkWidget *treeview, gpointer userdata){
  view_popup_menu(treeview, NULL, userdata);
  return TRUE; /* we handled this */
}

/*
* process backend signal and populate files tree
*/
void print_files (FilebrowserBackend *directory, gpointer user_data){
  if (!directory || !user_data) return ;
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  gphpedit_debug(DEBUG_FILEBROWSER);

  GSList *l;
  GtkTreeIter iter2;
  GtkTreeIter* iter=NULL;
  gtk_tree_store_clear(priv->pTree);
  /* get default icon theme */
  GtkIconTheme *theme= gtk_icon_theme_get_default();
    for (l = get_filebrowser_backend_files(directory); l != NULL; l = g_slist_next (l)) {
      FOLDERFILE *current=(FOLDERFILE *) l->data;
      gphpedit_debug_message(DEBUG_FILEBROWSER,"File added -> name: '%s' \tmime: '%s'",current->display_name,current->mime);
      GdkPixbuf *p_file_image = NULL;
      /* get icon of size menu */
      GIcon *icon= g_icon_new_for_string (current->icon,NULL);
      GtkIconInfo *ificon= gtk_icon_theme_lookup_by_gicon (theme, icon, 24, 0);
      g_object_unref(icon);
      p_file_image = gtk_icon_info_load_icon (ificon, NULL);
      gtk_tree_store_insert_with_values(GTK_TREE_STORE(priv->pTree), &iter2, iter, 0, ICON_COLUMN, p_file_image, FILE_COLUMN, current->display_name,MIME_COLUMN,current->mime,-1);
      g_object_unref(p_file_image);
       gtk_icon_info_free(ificon);
    }
  if (get_filebrowser_backend_number_files(priv->fbbackend)!=0){
    priv->cache_model=gtk_tree_view_get_model (GTK_TREE_VIEW(priv->pListView));
      search_control_sensible(priv,TRUE);
  } else {
      search_control_sensible(priv,FALSE);
  }
}

void change_folder_cb (gpointer instance, const gchar *current_folder, gpointer user_data){
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  gtk_button_set_label(GTK_BUTTON(priv->button_dialog), current_folder);
}

void  cancel_filebrowser_process (GtkWidget	 *widget){
 gphpeditFileBrowserPrivate *	priv = FILEBROWSER_BACKEND_GET_PRIVATE(widget);
 filebrowser_backend_cancel (priv->fbbackend);
}


static gboolean visible_func (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data) {
  /* Visible if row is non-empty and name column contain filename as prefix */
  gchar *str;
  gboolean visible = FALSE;
  const char *filename= (const char *) data;
  gtk_tree_model_get (model, iter, 1, &str, -1);
  if (str && g_str_has_prefix(str,filename)) visible = TRUE;
  g_free (str);
  return visible;
}

/**
* on_search_press
* process icon search press signals
*/
static void on_search_press (const gchar *filename, gpointer user_data){
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  GtkTreeModel *new_model= gtk_tree_model_filter_new (priv->cache_model,NULL);
  gtk_tree_model_filter_set_visible_func ((GtkTreeModelFilter *) new_model, visible_func, (gpointer) filename, NULL);
  gtk_tree_view_set_model (GTK_TREE_VIEW(priv->pListView), new_model);
}

/**
* on_cleanicon_press
* process icon press signals 
*/
static void on_cleanicon_press (GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, gpointer user_data){
  gphpeditFileBrowserPrivate *priv = (gphpeditFileBrowserPrivate *) user_data;
  if (icon_pos==GTK_ENTRY_ICON_SECONDARY){    
    gtk_entry_set_text (entry,"");
  } else {
    const gchar *find_string=gtk_entry_get_text (entry);
    gphpedit_debug_message(DEBUG_FILEBROWSER, "Search for:'%s'\n",find_string);
    if (find_string && get_filebrowser_backend_number_files(priv->fbbackend)!=0)
      on_search_press (gtk_entry_get_text (entry), priv);
    else
      /* nothing to search sets default model */
      gtk_tree_view_set_model (GTK_TREE_VIEW(priv->pListView),priv->cache_model);
  }
}

static void search_typed (GtkEntry *entry, const gchar *text, gint length, gint *position, gpointer data) {
  on_cleanicon_press (entry, GTK_ENTRY_ICON_PRIMARY, NULL, data);
}

static void search_activate(GtkEntry *entry,gpointer user_data) {
  on_cleanicon_press (entry, GTK_ENTRY_ICON_PRIMARY, NULL, user_data);
}

/*
* drag and drop function
*/

void fb_file_v_drag_data_received(GtkWidget * widget, GdkDragContext * context, gint x,  gint y, GtkSelectionData * data, guint info, guint time,gpointer user_data)
{
  gchar *stringdata;
  g_signal_stop_emission_by_name(widget, "drag_data_received");
  if ((gtk_selection_data_get_length(data) == 0) || (gtk_selection_data_get_format(data) != 8)
    || ((info != TARGET_STRING) && (info != TARGET_URI_LIST))) {
    gtk_drag_finish(context, FALSE, TRUE, time);
    return;
  }
  stringdata = g_strndup((const gchar *) gtk_selection_data_get_data(data), gtk_selection_data_get_length(data));
  gphpedit_debug_message(DEBUG_FILEBROWSER, "stringdata='%s', len=%d\n", stringdata, gtk_selection_data_get_length(data));
  gtk_drag_finish(context, filebrowser_backend_process_drag_drop(user_data, stringdata), TRUE, time);
  g_free(stringdata);
}
