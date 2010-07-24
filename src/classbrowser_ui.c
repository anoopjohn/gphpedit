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
/* ******* CLASSBROWSER DESIGN ********
* classbrowser widget has two part. UI and backend. the Backend process open files and return a list of classes and functions in files.
* UI process backend signal and fill the classbrowser Tree.
*/
#include "classbrowser_ui.h"
#include "classbrowser_backend.h"
#include <gdk/gdkkeysyms.h>
#include "main_window.h"

/* functions */
static void gphpedit_classbrowser_class_init (gphpeditClassBrowserClass *klass);
static void gphpedit_classbrowser_init (gphpeditClassBrowser *button);

struct _gphpeditClassBrowserPrivate
{
  Classbrowser_Backend *classbackend;

  GtkWidget *classbrowser;
  //Checkbox above treeview to parse only the current tab  
  GtkWidget *chkOnlyCurFileFuncs;

  gint front; /* model flag */

  GtkWidget *scrolledwindow;
  GtkTreeStore *classtreestore;
  GtkTreeStore *classtreestoreback; /* second model */
  GtkTreeModel *new_model;
  GtkWidget *classtreeview;
  GtkTreeSelection *classtreeselect;
  GtkWidget *treeviewlabel;
  GtkWidget *label1;

  GtkTreeModel *cache_model;

  gulong  handlerid;
};

enum {
  NAME_COLUMN,
  LINE_NUMBER_COLUMN,
  FILENAME_COLUMN,
  TYPE_COLUMN,
  ID_COLUMN,
  FILE_TYPE,
  N_COLUMNS
};
/*
* classbrowser types
*/
#define CB_ITEM_TYPE_CLASS 1
#define CB_ITEM_TYPE_CLASS_METHOD 2
#define CB_ITEM_TYPE_FUNCTION 3


static gpointer gphpedit_class_browser_parent_class;
void classbrowser_set_sortable(GtkTreeStore *classtreestore);
gint on_parse_current_click (GtkWidget *widget, gpointer user_data);
void classbrowser_update_cb (Classbrowser_Backend *classback, gboolean result, gpointer user_data);

void classbrowser_function_add (gpointer data, gpointer user_data);
gboolean classbrowser_class_add (gpointer key, gpointer value, gpointer data);
gint treeview_double_click(GtkWidget *widget, GdkEventButton *event, gpointer func_data);
gint treeview_click_release(GtkWidget *widget, GdkEventButton *event, gpointer func_data);
void classbrowser_update_selected_label(gphpeditClassBrowserPrivate *priv, gchar *filename, gint line);

#define CLASSBROWSER_BACKEND_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_CLASSBROWSER,\
					    gphpeditClassBrowserPrivate))


GType
gphpedit_classbrowser_get_type (void)
{
    static GType our_type = 0;
    
    if (!our_type) {
        static const GTypeInfo our_info =
        {
            sizeof (gphpeditClassBrowserClass),
            NULL,               /* base_init */
            NULL,               /* base_finalize */
            (GClassInitFunc) gphpedit_classbrowser_class_init,
            NULL,               /* class_finalize */
            NULL,               /* class_data */
            sizeof (gphpeditClassBrowser),
            0,                  /* n_preallocs */
            (GInstanceInitFunc) gphpedit_classbrowser_init,
        };

        our_type = g_type_register_static (GTK_TYPE_VBOX, "gphpeditClassBrowser",
                                           &our_info, 0);
  }
    
    return our_type;
}

static void
gphpedit_classbrowser_destroy (GtkObject *object)
{
	gphpeditClassBrowserPrivate *priv;

	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(object);

  if (g_signal_handler_is_connected (priv->classbackend, priv->handlerid)){
   g_signal_handler_disconnect(priv->classbackend, priv->handlerid);
  }

  if (G_IS_OBJECT(priv->classbackend)) g_object_unref(priv->classbackend);

	GTK_OBJECT_CLASS (gphpedit_class_browser_parent_class)->destroy (object);
}

static void
gphpedit_classbrowser_finalize (GObject *object)
{
	gphpeditClassBrowserPrivate *priv;

	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(object);

	G_OBJECT_CLASS (gphpedit_class_browser_parent_class)->finalize (object);
}

static void 
gphpedit_classbrowser_class_init (gphpeditClassBrowserClass *klass)
{
	GObjectClass   *object_class = G_OBJECT_CLASS (klass);
	GtkObjectClass *gtkobject_class = GTK_OBJECT_CLASS (klass);
	gphpedit_class_browser_parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = gphpedit_classbrowser_finalize;
	gtkobject_class->destroy = gphpedit_classbrowser_destroy;
	
	g_type_class_add_private (object_class, sizeof(gphpeditClassBrowserPrivate));
}

/*
* gphpedit_classbrowser_init
* init a new classbrowser object
* creates all classbrowser widgets and the classbrowser backend
*/
static void
gphpedit_classbrowser_init (gphpeditClassBrowser *button)
{
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;

  gphpeditClassBrowserPrivate *priv = CLASSBROWSER_BACKEND_GET_PRIVATE(button);
  priv->classbackend= classbrowser_backend_new ();
  priv->handlerid = g_signal_connect(G_OBJECT(priv->classbackend), "done_refresh", G_CALLBACK(classbrowser_update_cb), priv);

  GtkWidget *notebox;
  notebox = gtk_vbox_new(FALSE, 0);
  gtk_widget_show(notebox);

  /* add checkbox to show only current file's classes
  the signals to be checked for the check box are onclick of the checkbox
  and the on change of the file.
  */
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  priv->chkOnlyCurFileFuncs = gtk_check_button_new_with_label(_("Parse only current file"));
  gtk_toggle_button_set_active ((GtkToggleButton *)priv->chkOnlyCurFileFuncs, get_preferences_manager_parse_only_current_file(main_window.prefmg));
  gtk_widget_show (priv->chkOnlyCurFileFuncs);
  gtk_widget_show (hbox);
  gtk_box_pack_start(GTK_BOX(hbox), priv->chkOnlyCurFileFuncs, TRUE, TRUE, 10);
  gtk_box_pack_start(GTK_BOX(notebox), hbox, FALSE, FALSE, 10);
  g_signal_connect (G_OBJECT (priv->chkOnlyCurFileFuncs), "clicked",
            G_CALLBACK (on_parse_current_click), priv->classbackend);

  priv->scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (priv->scrolledwindow);
  gtk_box_pack_start(GTK_BOX(notebox), priv->scrolledwindow, TRUE, TRUE, 0);

  GtkWidget *box2;
  box2 = gtk_hbox_new(FALSE, 0);
  gtk_widget_show(box2);
  priv->treeviewlabel = gtk_label_new(_("FILE: "));
  gtk_label_set_justify(GTK_LABEL(priv->treeviewlabel), GTK_JUSTIFY_LEFT);
  gtk_widget_show(priv->treeviewlabel);
  gtk_box_pack_start(GTK_BOX(box2), priv->treeviewlabel, FALSE, FALSE, 0);
  gtk_box_pack_end(GTK_BOX(notebox), box2, FALSE, FALSE, 4);
  
  priv->classtreestore = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
  /* enable sorting of the columns */
  classbrowser_set_sortable(priv->classtreestore);

  /* second model stuff */
  priv->classtreestoreback = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);
  /* enable sorting of the columns */
  classbrowser_set_sortable(priv->classtreestoreback);

  priv->front=0;

  priv->classtreeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (priv->classtreestore));
  gtk_widget_show (priv->classtreeview);
  gtk_container_add (GTK_CONTAINER (priv->scrolledwindow), priv->classtreeview);

  priv->classtreeselect = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->classtreeview));
  gtk_tree_selection_set_mode (priv->classtreeselect, GTK_SELECTION_SINGLE);
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"), //FIXME:: add icons
           renderer, "text", NAME_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->classtreeview), column);

  gtk_box_pack_start(GTK_BOX(button), notebox, TRUE, TRUE, 2);
}

/*
* gphpedit_classbrowser_new
* return a new classbrowser widget
*/
GtkWidget *
gphpedit_classbrowser_new (void)
{
	gphpeditClassBrowser *button;

	button = g_object_new (GPHPEDIT_TYPE_CLASSBROWSER, NULL);

	return GTK_WIDGET (button);
}

/*
* function to refresh treeview when the parse only current file checkbox is clicked
* or when the checkbox is clicked and the files tabbar is clicked
*/
gint on_parse_current_click (GtkWidget *widget, gpointer user_data)
{
  set_preferences_manager_parse_only_current_file(main_window.prefmg, gtk_toggle_button_get_active((GtkToggleButton *)widget));
  classbrowser_backend_update(user_data, editors, gtk_toggle_button_get_active((GtkToggleButton *)widget));
  return 0;
}


//compare function names of the iter of the gtktreeview
gint classbrowser_compare_function_names(GtkTreeModel *model,
                    GtkTreeIter *a,
                    GtkTreeIter *b,
                    gpointer user_data)
{
  gchar *aName, *bName;
  gint retVal;
  gtk_tree_model_get(model, a, 0, &aName, -1);
  gtk_tree_model_get(model, b, 0, &bName, -1);
  retVal = g_strcmp0(aName, bName);
  #ifdef DEBUGCLASSBROWSER
    g_message("* compare values %s and %s; return %d\n", aName, bName, retVal);
  #endif
  g_free(aName);
  g_free(bName);
  return retVal;
}

/*
 * classbrowser_set_sortable (internal)
 * enable sorting of the list
 *
 */

void classbrowser_set_sortable(GtkTreeStore *classtreestore)
{
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    classbrowser_compare_function_names,
    NULL,NULL);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    0, classbrowser_compare_function_names, 
    NULL,NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(classtreestore),
    0,
    GTK_SORT_ASCENDING);
}

static gboolean visible_func (GtkTreeModel *model, GtkTreeIter  *iter, gpointer data) {
  /* Visible if row is non-empty and name column contain filename as prefix */
  if (!main_window.current_editor) return FALSE;
  guint file_type;
  gboolean visible = FALSE;
  guint data_type= main_window.current_editor->type;
  gtk_tree_model_get (model, iter, FILE_TYPE, &file_type, -1);
  if (data_type==file_type) visible = TRUE;
 // g_print("%d -> %d (%s)\n",data_type,file_type,main_window.current_editor->filename->str);
  return visible;
}

/*
* classbrowser_update_cb (internal)
* process description:
* we process the backend signal and we load classes and function data to a model. 
* Then when load is finished we change the current model for the new one.
* doing this we avoid flicker when we update the classbrowser.
*/
void classbrowser_update_cb (Classbrowser_Backend *classback, gboolean result, gpointer user_data)
{
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) user_data;
  static guint press_event = 0;
  static guint release_event = 0;
  if (get_preferences_manager_classbrowser_status(main_window.prefmg)!=0){ /* do nothing if classbrowser is hidden */
    return;
  }
  if (press_event && g_signal_handler_is_connected (priv->classtreeview, press_event)) {
    g_signal_handler_disconnect(priv->classtreeview, press_event);
  }
  if (release_event && g_signal_handler_is_connected (priv->classtreeview, release_event)) {
    g_signal_handler_disconnect(priv->classtreeview,release_event);
  }
  if (priv->front==0){
    gtk_tree_store_clear(priv->classtreestore); /* clear tree */
  } else {
    gtk_tree_store_clear(priv->classtreestoreback); /* clear tree */
  }
  GTree *class_list = classbrowser_backend_get_class_list(classback);
  g_tree_foreach (class_list, classbrowser_class_add, user_data);

  GSList *func_list = classbrowser_backend_get_function_list(classback);
  g_slist_foreach (func_list, classbrowser_function_add, user_data);

  if (main_window.current_editor) {
  if (priv->front==0){
    priv->new_model= gtk_tree_model_filter_new (GTK_TREE_MODEL(priv->classtreestore),NULL);
  } else {
    priv->new_model= gtk_tree_model_filter_new (GTK_TREE_MODEL(priv->classtreestoreback),NULL);
  }
    gtk_tree_model_filter_set_visible_func ((GtkTreeModelFilter *) priv->new_model, visible_func, NULL, NULL);
    gtk_tree_view_set_model (GTK_TREE_VIEW( priv->classtreeview), priv->new_model);
  }

  press_event = g_signal_connect(GTK_OBJECT(priv->classtreeview), "button_press_event",
                                   G_CALLBACK(treeview_double_click), priv);
  release_event = g_signal_connect(GTK_OBJECT(priv->classtreeview), "button_release_event",
                                     G_CALLBACK(treeview_click_release), priv);
  if (priv->front == 0) priv->front=1;
    else priv->front=0;
}

gboolean classbrowser_classid_to_iter(GtkTreeModel *model, guint classid, GtkTreeIter *iter)
{
  guint id;
  gboolean found;

  if (gtk_tree_model_get_iter_first(model, iter)) {
    found = TRUE;
    while (found) {
      gtk_tree_model_get(model,iter, ID_COLUMN, &id,-1);
      if (id == classid) {
        return TRUE;
      }
      found = gtk_tree_model_iter_next(model, iter);
    }
  }
  return FALSE;
}

/*
* classbrowser_function_add
* add a function to the tree model
*/
void classbrowser_function_add (gpointer data, gpointer user_data)
{
  ClassBrowserFunction *function= (ClassBrowserFunction *) data;
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) user_data;
  GtkTreeIter iter;
  GtkTreeIter class_iter;
  GString *function_decl;
  guint type;
  GtkTreeStore *store;
  if (priv->front==0){
    store=priv->classtreestore;
  } else {
    store=priv->classtreestoreback;
  }
  gchar *keyname=g_strdup_printf("%s%s",function->classname,function->filename);
  if (function->classname){
      classbrowser_classid_to_iter(GTK_TREE_MODEL(store), function->class_id, &class_iter);
      type = CB_ITEM_TYPE_CLASS_METHOD;
      gtk_tree_store_append (store, &iter, &class_iter);
    } else {
      type = CB_ITEM_TYPE_FUNCTION;
      gtk_tree_store_append (store, &iter, NULL);
    }
    g_free(keyname);
    function_decl = g_string_new(function->functionname);
    if (function->file_type!=TAB_COBOL){ /* cobol paragraph don't have params */
      function_decl = g_string_append(function_decl, "(");
      if (function->paramlist) {
        function_decl = g_string_append(function_decl, function->paramlist);
      }
      function_decl = g_string_append(function_decl, ")");
    }
    #ifdef DEBUGCLASSBROWSER
      g_print("Filename: %s\n", filename);
    #endif
    gtk_tree_store_set (store, &iter,
                        NAME_COLUMN, function_decl->str, LINE_NUMBER_COLUMN, function->line_number, FILENAME_COLUMN, function->filename, TYPE_COLUMN, type, ID_COLUMN, function->identifierid,FILE_TYPE, function->file_type,-1);
  g_string_free(function_decl, TRUE);
  while (gtk_events_pending()) gtk_main_iteration(); /* update ui */
}

gboolean classbrowser_class_find_before_position(GtkTreeModel *model, gchar *classname, GtkTreeIter *iter)
{
  gboolean found;
  gchar *classnamefound;
  guint type;

  if (gtk_tree_model_get_iter_first(model, iter)) {
    found = TRUE;
    while (found) {
      gtk_tree_model_get(model,iter, NAME_COLUMN, &classnamefound, TYPE_COLUMN, &type, -1);
      if ((type==CB_ITEM_TYPE_CLASS) && g_strcmp0(classname, classnamefound)<0) {
        g_free(classnamefound);
        return TRUE;
      }
      if (type==CB_ITEM_TYPE_FUNCTION) {
        g_free(classnamefound);
        return TRUE;
      }
      found = gtk_tree_model_iter_next(model, iter);
      g_free(classnamefound);
    }
  }
  return FALSE;
}

/*
* classbrowser_class_add
* add a class to the tree model
*/

gboolean classbrowser_class_add (gpointer key, gpointer value, gpointer data)
{
  ClassBrowserClass *class= (ClassBrowserClass *)value;
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) data;
  GtkTreeStore *store;
  if (priv->front==0){
  store=priv->classtreestore;
  } else {
  store=priv->classtreestoreback;
  }

  GtkTreeIter iter;
  GtkTreeIter before;
  if (classbrowser_class_find_before_position(GTK_TREE_MODEL(store), class->classname, &before)) {
    gtk_tree_store_insert_before(store, &iter, NULL, &before);
  } else {
    gtk_tree_store_append (store, &iter, NULL);
  }

  gtk_tree_store_set (GTK_TREE_STORE(store), &iter,
                   NAME_COLUMN, (class->classname), FILENAME_COLUMN, (class->filename),
                   LINE_NUMBER_COLUMN, class->line_number, TYPE_COLUMN, CB_ITEM_TYPE_CLASS, ID_COLUMN, (class->identifierid), FILE_TYPE, class->file_type,-1);
  while (gtk_events_pending()) gtk_main_iteration(); /* update ui */
  return FALSE;
}

/*
* treeview_double_click (internal)
* process double click signal
*/
gint treeview_double_click(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) func_data;
  GtkTreeIter iter;
  gchar *filename = NULL;
  guint line_number;

  if (event->type==GDK_2BUTTON_PRESS ||
      event->type==GDK_3BUTTON_PRESS) {
      if (gtk_tree_selection_get_selected (priv->classtreeselect, NULL, &iter)) {
          gtk_tree_model_get (GTK_TREE_MODEL(priv->new_model), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
        if (filename) {
          switch_to_file_or_open(filename, line_number);
          g_free (filename);
        }
      }
  }
  return FALSE;
}

gint treeview_click_release(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) func_data;
  GtkTreeIter iter;
  gchar *filename = NULL;
  guint line_number;

  if (gtk_tree_selection_get_selected (priv->classtreeselect, NULL, &iter)) {
    gtk_tree_model_get (GTK_TREE_MODEL(priv->new_model), &iter, FILENAME_COLUMN, &filename, LINE_NUMBER_COLUMN, &line_number, -1);
    if (filename) {
      classbrowser_update_selected_label(priv, filename, line_number);
      g_free (filename);
    }
  }
  /* go to position stuff */
  if (main_window.current_editor) {
    if(GTK_IS_SCINTILLA(main_window.current_editor->scintilla)){
    gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
    gtk_scintilla_scroll_caret(GTK_SCINTILLA(main_window.current_editor->scintilla));
    gtk_scintilla_grab_focus(GTK_SCINTILLA(main_window.current_editor->scintilla));
    }
  }
  
  return FALSE;
}
/*
* classbrowser_update
* start update process
*/
void classbrowser_update(gphpeditClassBrowser *classbrowser){
	gphpeditClassBrowserPrivate *priv;
	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  if (editors){
    classbrowser_backend_update(priv->classbackend, editors, get_preferences_manager_parse_only_current_file(main_window.prefmg));
  }
}


void classbrowser_update_selected_label(gphpeditClassBrowserPrivate *priv, gchar *filename, gint line)
{
  GString *new_label= classbrowser_backend_get_selected_label(priv->classbackend, filename, line);
  #ifdef DEBUGCLASSBROWSER
    g_print("%d :: %s\n", num_files, new_label->str);  
  #endif
  if (new_label) {
    new_label = g_string_prepend(new_label, _("FILE: "));
    g_string_append_printf(new_label, "(%d)", line);
    gtk_label_set_text(GTK_LABEL(priv->treeviewlabel), new_label->str);
    g_string_free(new_label, TRUE);
  }
}

void classbrowser_force_label_update(gphpeditClassBrowser *classbrowser)
{
	gphpeditClassBrowserPrivate *priv;
	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  GtkTreeIter iter;
  if (!gtk_tree_selection_get_selected (priv->classtreeselect, NULL, &iter)) {
      gtk_label_set_text(GTK_LABEL(priv->treeviewlabel), _("FILE:"));
  }
}

/*
* classbrowser_get_autocomplete_php_classes_string
* return a new string with posibly classes matches
*/
GString *classbrowser_get_autocomplete_php_classes_string(gphpeditClassBrowser *classbrowser){
	gphpeditClassBrowserPrivate *priv;
	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  return classbrowser_backend_get_autocomplete_php_classes_string(priv->classbackend);
}

/*
* classbrowser_autocomplete_php_variables
* return a new string with posibly variable matches. 
*/
void classbrowser_autocomplete_php_variables(gphpeditClassBrowser *classbrowser, GtkWidget *scintilla, gint wordStart, gint wordEnd){
	gphpeditClassBrowserPrivate *priv;
	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  classbrowser_backend_autocomplete_php_variables(priv->classbackend, scintilla, wordStart, wordEnd);
}

void classbrowser_autocomplete_member_function(gphpeditClassBrowser *classbrowser, GtkWidget *scintilla, gint wordStart, gint wordEnd){
	gphpeditClassBrowserPrivate *priv;
	priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  classbrowser_backend_autocomplete_member_function(priv->classbackend, scintilla, wordStart, wordEnd);
}

gchar *classbrowser_custom_function_calltip(gphpeditClassBrowser *classbrowser, gchar *function_name)
{
	gphpeditClassBrowserPrivate *priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  return classbrowser_backend_custom_function_calltip(priv->classbackend, function_name);
}

gchar *classbrowser_add_custom_autocompletion(gphpeditClassBrowser *classbrowser, gchar *prefix, GSList *list)
{
	gphpeditClassBrowserPrivate *priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  return classbrowser_backend_add_custom_autocompletion(priv->classbackend, prefix, list);
} 
