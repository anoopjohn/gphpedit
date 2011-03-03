/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2010 José Rostagno(for vijona.com.ar)
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
#include "debug.h"

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "classbrowser.h"
#include "gvfs_utils.h"
#include "symbol_manager.h"
#include "preferences_manager.h"
#include "document_manager.h"


/* functions */
static void gphpedit_classbrowser_class_init (gphpeditClassBrowserClass *klass);
static void gphpedit_classbrowser_init (gphpeditClassBrowser *button);

struct _gphpeditClassBrowserPrivate
{
  GtkWidget *classbrowser;
  GtkBuilder *builder;
  SymbolManager *symbolmg;
  PreferencesManager *prefmg;
  DocumentManager *docmg;

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
  PIXBUF_COLUMN,
  N_COLUMNS
};
/*
* classbrowser types
*/
#define CB_ITEM_TYPE_CLASS 1
#define CB_ITEM_TYPE_CLASS_METHOD 2
#define CB_ITEM_TYPE_FUNCTION 3

static void classbrowser_set_sortable(GtkTreeStore *classtreestore);
static gint on_parse_current_click (GtkWidget *widget, gpointer user_data);
static void sdb_update_cb (SymbolManager *symbolmg, gpointer user_data);

static void classbrowser_function_add (gpointer data, gpointer user_data);
static void classbrowser_class_add (gpointer data, gpointer user_data);
static gint treeview_double_click(GtkWidget *widget, GdkEventButton *event, gpointer func_data);
static gint treeview_click_release(GtkWidget *widget, GdkEventButton *event, gpointer func_data);
static void classbrowser_update_selected_label(gphpeditClassBrowserPrivate *priv, gchar *filename, gint line);

#define CLASSBROWSER_BACKEND_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    GPHPEDIT_TYPE_CLASSBROWSER,\
					    gphpeditClassBrowserPrivate))

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(gphpeditClassBrowser, gphpedit_classbrowser, GTK_TYPE_VBOX);


static void gphpedit_classbrowser_dispose (GObject *object)
{
  if(!object) return;
  gphpeditClassBrowserPrivate *priv;

  priv = CLASSBROWSER_BACKEND_GET_PRIVATE(object);
  g_object_unref(priv->symbolmg);
  g_object_unref(priv->prefmg);
  g_object_unref(priv->docmg);

  G_OBJECT_CLASS (gphpedit_classbrowser_parent_class)->dispose (object);
}

static void 
gphpedit_classbrowser_class_init (gphpeditClassBrowserClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = gphpedit_classbrowser_dispose;
	
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

  priv->symbolmg = symbol_manager_new (); 
  g_signal_connect(priv->symbolmg, "update", G_CALLBACK(sdb_update_cb), priv);

  priv->prefmg = preferences_manager_new ();
  priv->docmg = document_manager_new ();
  priv->builder = gtk_builder_new ();
  GError *error = NULL;
  guint res = gtk_builder_add_from_file (priv->builder, GPHPEDIT_UI_DIR "/classbrowser.ui", &error);
  if (!res) {
    g_critical ("Unable to load the UI file!");
    g_error_free(error);
    return ;
  }

  GtkWidget *notebox = GTK_WIDGET(gtk_builder_get_object (priv->builder, "classbrowser"));
  gtk_widget_show (notebox);
  gtk_widget_reparent (notebox, GTK_WIDGET(button));

  priv->chkOnlyCurFileFuncs = GTK_WIDGET(gtk_builder_get_object (priv->builder, "only_current_file"));

  gboolean active;
  g_object_get (priv->prefmg, "parse_only_current_file", &active, NULL);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->chkOnlyCurFileFuncs), active);
  g_signal_connect (G_OBJECT (priv->chkOnlyCurFileFuncs), "clicked",
            G_CALLBACK (on_parse_current_click), priv);

  priv->treeviewlabel = GTK_WIDGET(gtk_builder_get_object (priv->builder, "treeviewlabel"));
  priv->classtreestore = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, GDK_TYPE_PIXBUF);
  /* enable sorting of the columns */
  classbrowser_set_sortable(priv->classtreestore);

  /* second model stuff */
  priv->classtreestoreback = gtk_tree_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, GDK_TYPE_PIXBUF);
  /* enable sorting of the columns */
  classbrowser_set_sortable(priv->classtreestoreback);

  priv->front=0;
  priv->classtreeview = GTK_WIDGET(gtk_builder_get_object (priv->builder, "classtreeview"));
	gtk_tree_view_set_model (GTK_TREE_VIEW (priv->classtreeview), GTK_TREE_MODEL (priv->classtreestore));

  priv->classtreeselect = gtk_tree_view_get_selection (GTK_TREE_VIEW (priv->classtreeview));
  gtk_tree_selection_set_mode (priv->classtreeselect, GTK_SELECTION_SINGLE);

  renderer =  gtk_cell_renderer_pixbuf_new ();
  column = gtk_tree_view_column_new_with_attributes ("", renderer,
                               "pixbuf", PIXBUF_COLUMN,
                                NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->classtreeview), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (_("Name"), 
           renderer, "text", NAME_COLUMN, NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (priv->classtreeview), column);
}

/*
* gphpedit_classbrowser_new
* return a new classbrowser widget
*/
GtkWidget *
gphpedit_classbrowser_new (void)
{
  return g_object_new (GPHPEDIT_TYPE_CLASSBROWSER, NULL);
}

/*
* function to refresh treeview when the parse only current file checkbox is clicked
* or when the checkbox is clicked and the files tabbar is clicked
*/
static gint on_parse_current_click (GtkWidget *widget, gpointer user_data)
{
  gphpeditClassBrowserPrivate *priv = (gphpeditClassBrowserPrivate *) user_data;
  g_object_set (priv->prefmg, "parse_only_current_file", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)), NULL);
  sdb_update_cb (priv->symbolmg, priv);
  return 0;
}


//compare function names of the iter of the gtktreeview
static gint classbrowser_compare_function_names(GtkTreeModel *model,
                    GtkTreeIter *a,
                    GtkTreeIter *b,
                    gpointer user_data)
{
  gchar *aName, *bName;
  gint retVal;
  gtk_tree_model_get(model, a, 0, &aName, -1);
  gtk_tree_model_get(model, b, 0, &bName, -1);
  retVal = g_strcmp0(aName, bName);
  #ifdef DEBUG
    gphpedit_debug_message(DEBUG_CLASSBROWSER, "* compare values %s and %s; return %d\n", aName, bName, retVal);
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

static void classbrowser_set_sortable(GtkTreeStore *classtreestore)
{
  gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    classbrowser_compare_function_names, NULL,NULL);
  gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(classtreestore), 
    0, classbrowser_compare_function_names, 
    NULL, NULL);
  gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(classtreestore),
    0,
    GTK_SORT_ASCENDING);
}

static void classbrowser_clear_model (gphpeditClassBrowserPrivate *priv)
{
  if (priv->front==0) {
    gtk_tree_store_clear(priv->classtreestore); /* clear tree */
  } else {
    gtk_tree_store_clear(priv->classtreestoreback); /* clear tree */
  }
}

static GtkTreeModel *classbrowser_get_model(gphpeditClassBrowserPrivate *priv)
{
  if (priv->front==0) {
    return GTK_TREE_MODEL(priv->classtreestore);
  } else {
    return GTK_TREE_MODEL(priv->classtreestoreback);
  }
}
/*
* sdb_update_cb (internal)
* process description:
* we process the backend signal and we load classes and function data to a model. 
* Then when load is finished we change the current model for the new one.
* doing this we avoid flicker when we update the classbrowser.
*/
static void sdb_update_cb (SymbolManager *symbolmg, gpointer user_data)
{
  gphpedit_debug(DEBUG_CLASSBROWSER);
  gphpeditClassBrowserPrivate *priv= (gphpeditClassBrowserPrivate *) user_data;
  static guint press_event = 0;
  static guint release_event = 0;
  gboolean hidden;
  GList *class_list = NULL;
  GList *func_list = NULL;

  g_object_get(priv->prefmg, "side_panel_hidden", &hidden, NULL);
  if(hidden) return ;
  if (press_event && g_signal_handler_is_connected (priv->classtreeview, press_event)) {
    g_signal_handler_disconnect(priv->classtreeview, press_event);
  }
  if (release_event && g_signal_handler_is_connected (priv->classtreeview, release_event)) {
    g_signal_handler_disconnect(priv->classtreeview,release_event);
  }
  classbrowser_clear_model (priv);

  Documentable *doc = document_manager_get_current_documentable(priv->docmg);
  guint doc_type;
  g_object_get(doc, "type", &doc_type, NULL);

  gboolean active;
  g_object_get (priv->prefmg, "parse_only_current_file", &active, NULL);

  if (active) {
    gchar *filename = documentable_get_filename(doc);
    class_list = symbol_manager_get_custom_symbols_list_by_filename (symbolmg, SYMBOL_CLASS, filename, doc_type);
    func_list = symbol_manager_get_custom_symbols_list_by_filename (symbolmg, SYMBOL_FUNCTION, filename, doc_type);
    g_free(filename);
  } else {
    class_list = symbol_manager_get_custom_symbols_list (symbolmg, SYMBOL_CLASS, doc_type);
    func_list = symbol_manager_get_custom_symbols_list (symbolmg, SYMBOL_FUNCTION, doc_type);
  }
  g_list_foreach (class_list, classbrowser_class_add, classbrowser_get_model(priv));
  g_list_foreach (func_list, classbrowser_function_add, classbrowser_get_model(priv));

  g_list_free(class_list);
  g_list_free(func_list);

  priv->new_model = classbrowser_get_model(priv);
  gtk_tree_view_set_model (GTK_TREE_VIEW( priv->classtreeview), priv->new_model);

  press_event = g_signal_connect(GTK_OBJECT(priv->classtreeview), "button_press_event",
                                   G_CALLBACK(treeview_double_click), priv);
  release_event = g_signal_connect(GTK_OBJECT(priv->classtreeview), "button_release_event",
                                     G_CALLBACK(treeview_click_release), priv);
  priv->front=!priv->front; /* change model */
}

static gboolean classbrowser_classid_to_iter(GtkTreeModel *model, guint classid, GtkTreeIter *iter)
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

static GString *get_function_decl(ClassBrowserFunction *function)
{
  GString *function_decl;
  function_decl = g_string_new(function->functionname);
//  if (function->file_type!=TAB_COBOL) { /* cobol paragraph don't have params */
    if (function->paramlist) {
      g_string_append_printf(function_decl, "(%s)", function->paramlist);
    } else {
      function_decl = g_string_append(function_decl, "()");
    }
//  }
  return function_decl;
}

/*
* classbrowser_function_add
* add a function to the tree model
*/
static void classbrowser_function_add (gpointer data, gpointer user_data)
{
  gphpedit_debug(DEBUG_CLASSBROWSER);
  ClassBrowserFunction *function= (ClassBrowserFunction *) data;
  GtkTreeIter iter;
  GtkTreeIter class_iter;
  GString *function_decl;
  guint type;
  GtkTreeStore *store= GTK_TREE_STORE(user_data);

  if (function->classname){
      classbrowser_classid_to_iter(GTK_TREE_MODEL(store), function->class_id, &class_iter);
      type = CB_ITEM_TYPE_CLASS_METHOD;
      gtk_tree_store_append (store, &iter, &class_iter);
    } else {
      type = CB_ITEM_TYPE_FUNCTION;
      gtk_tree_store_append (store, &iter, NULL);
    }
    function_decl = get_function_decl(function);
    GdkPixbuf *pixbuf;
    pixbuf = gdk_pixbuf_new_from_file(PIXMAP_DIR "/function.png", NULL);

    gtk_tree_store_set (store, &iter,
                        NAME_COLUMN, function_decl->str, LINE_NUMBER_COLUMN, function->line_number, 
                        FILENAME_COLUMN, function->filename, TYPE_COLUMN, type, 
                        ID_COLUMN, function->identifierid, PIXBUF_COLUMN, pixbuf,-1);
    g_object_unref(pixbuf);
    g_string_free(function_decl, TRUE);
}

static gboolean classbrowser_class_find_before_position(GtkTreeModel *model, gchar *classname, GtkTreeIter *iter)
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

static void classbrowser_class_add (gpointer data, gpointer user_data)
{
  gphpedit_debug(DEBUG_CLASSBROWSER);
  ClassBrowserClass *class= (ClassBrowserClass *)data;
  GtkTreeStore *store = GTK_TREE_STORE(user_data);

  GtkTreeIter iter;
  GtkTreeIter before;
  if (classbrowser_class_find_before_position(GTK_TREE_MODEL(store), class->classname, &before)) {
    gtk_tree_store_insert_before(store, &iter, NULL, &before);
  } else {
    gtk_tree_store_append (store, &iter, NULL);
  }

  GdkPixbuf *pixbuf;
  pixbuf = gdk_pixbuf_new_from_file(PIXMAP_DIR "/class.png", NULL);

  gtk_tree_store_set (GTK_TREE_STORE(store), &iter,
                   NAME_COLUMN, (class->classname), FILENAME_COLUMN, (class->filename),
                   LINE_NUMBER_COLUMN, class->line_number, TYPE_COLUMN, CB_ITEM_TYPE_CLASS, 
                   ID_COLUMN, (class->identifierid),PIXBUF_COLUMN, pixbuf,-1);
  g_object_unref(pixbuf);
}

/*
* treeview_double_click (internal)
* process double click signal
*/
static gint treeview_double_click(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
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
          document_manager_switch_to_file_or_open(priv->docmg, filename, line_number);
          g_free (filename);
        }
      }
  }
  return FALSE;
}

static gint treeview_click_release(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
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
  /* go to position */
  documentable_scroll_to_current_pos(document_manager_get_current_documentable(priv->docmg));
 
  return FALSE;
}

/*
* classbrowser_update
* start update process
*/
void classbrowser_update(gphpeditClassBrowser *classbrowser)
{
  gphpedit_debug(DEBUG_CLASSBROWSER);
  if (!classbrowser) return ;
  gphpeditClassBrowserPrivate *priv;
  priv = CLASSBROWSER_BACKEND_GET_PRIVATE(classbrowser);
  /* update FILE: label text */
  GtkTreeIter iter;
  if (!gtk_tree_selection_get_selected (priv->classtreeselect, NULL, &iter)) {
      gtk_label_set_text(GTK_LABEL(priv->treeviewlabel), _("FILE:"));
  }
  if (document_manager_get_document_count (priv->docmg)) {
    sdb_update_cb (priv->symbolmg, priv);
  }
}

static void classbrowser_update_selected_label(gphpeditClassBrowserPrivate *priv, gchar *filename, gint line)
{
  if(filename) {
    gchar *basename = filename_get_basename(filename);
    gchar *caption = g_strdup_printf("%s%s (%d)", _("FILE: "), basename, line);
    gtk_label_set_text(GTK_LABEL(priv->treeviewlabel), caption);
    g_free(basename);
    g_free(caption);
  }
}
