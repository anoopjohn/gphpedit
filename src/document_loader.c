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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <gtk/gtk.h>
#include "debug.h"
#include "document_loader.h"
#include "tab.h"
#include "main_window.h"
#include "gphpedit-statusbar.h"
#include <gconf/gconf-client.h>
#include "gvfs_utils.h"

#define ICON_SIZE 16

#define IS_MIME(stringa,stringb) (g_content_type_equals (stringa, stringb))
#define IS_TEXT(stringa) (g_content_type_is_a (stringa, "text/*"))
#define IS_APPLICATION(stringa) (g_content_type_is_a (stringa, "application/*") && !IS_MIME(stringa,"application/x-php") && !IS_MIME(stringa,"application/javascript") && !IS_MIME(stringa,"application/x-perl"))


/* object signal enumeration */
enum {
	DONE_LOADING,
	DONE_NAVIGATE,
	DONE_REFRESH,
	LAST_SIGNAL
};

enum {
LOAD = 1,
NAVIGATE,
REFRESH
};
static guint signals[LAST_SIGNAL];

/*
* document_loader private struct
*/
struct DocumentLoaderDetails
{
  GMountOperation *gmo;
  GFile *file;
  GtkWindow *dialog_parent_window;
  gboolean converted_to_UTF8;
  Document *document;
  guint file_contents_len;
  gchar *buffer;
  gint webkit_action;
  gchar *raw_uri;
};

#define DOCUMENT_LOADER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    DOCUMENT_LOADER_TYPE,\
					    DocumentLoaderDetails))

static void document_loader_dispose (GObject *gobject);

void document_create_help(DocumentLoader *doclod);
void document_load_file(DocumentLoader *doclod);

void tab_help_load_file(DocumentLoader *doclod);
/*
 * register DocumentLoader type and returns a new GType
*/
G_DEFINE_TYPE(DocumentLoader, document_loader, G_TYPE_OBJECT);  

static void
document_loader_class_init (DocumentLoaderClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = document_loader_dispose;

/*
if load is ok return TRUE. if load isn't complete return FALSE
*/
	signals[DONE_LOADING] =
		g_signal_new ("done_loading",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, done_loading),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__BOOLEAN,
		               G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	signals[DONE_NAVIGATE] =
		g_signal_new ("done_navigate",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, done_navigate),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__BOOLEAN,
		               G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	signals[DONE_REFRESH] =
		g_signal_new ("done_refresh",
		              G_TYPE_FROM_CLASS (object_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (DocumentLoaderClass, done_navigate),
		              NULL, NULL,
		               g_cclosure_marshal_VOID__BOOLEAN,
		               G_TYPE_NONE, 1, G_TYPE_BOOLEAN, NULL);

	g_type_class_add_private (klass, sizeof (DocumentLoaderDetails));
}

static void
document_loader_init (DocumentLoader * object)
{
//	DocumentLoaderDetails *docloddet;
//	docloddet = DOCUMENT_LOADER_GET_PRIVATE(object);
}

/*
* disposes the Gobject
*/
static void document_loader_dispose (GObject *object)
{
  DocumentLoader *doclod = DOCUMENT_LOADER(object);
  DocumentLoaderDetails *docloddet;
	docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  /* free object resources*/
  g_object_unref(docloddet->dialog_parent_window);
  if (docloddet->gmo) g_object_unref(docloddet->gmo);
  if (docloddet->raw_uri) g_free(docloddet->raw_uri);

  /* Chain up to the parent class */
  G_OBJECT_CLASS (document_loader_parent_class)->dispose (object);
}

DocumentLoader *document_loader_new (Document *document, GtkWindow *dialog_parent_window)
{
	DocumentLoader *doclod;
  doclod = g_object_new (DOCUMENT_LOADER_TYPE, NULL);
  DocumentLoaderDetails *docloddet;
	docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  if (document_get_GFile(document)) docloddet->file = g_object_ref(document_get_GFile(document));
  docloddet->dialog_parent_window = g_object_ref(dialog_parent_window);
  docloddet->document = document;
  docloddet->file_contents_len = 0;
  docloddet->buffer = NULL;
  docloddet->raw_uri = NULL;
	return doclod; /* return new object */
}

void openfile_mount(GObject *source_object,GAsyncResult *res,gpointer user_data) {
  GError *error=NULL;
  if (g_file_mount_enclosing_volume_finish((GFile *)source_object,res,&error)) {
    /* open again */
    document_loader_load_document(user_data);
  } else {
    g_print(_("Error mounting volume. GIO error:%s\n"),error->message);
    g_error_free(error);
    g_signal_emit (G_OBJECT (user_data), signals[DONE_LOADING], 0, FALSE);
  }
}

gboolean mount_mountable_file(DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GError *error=NULL;
  GMount *ex= g_file_find_enclosing_mount (docloddet->file,NULL,&error);
  if (!ex){
    if (error->code== G_IO_ERROR_NOT_MOUNTED){
      gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s",
           _("Error filesystem not mounted. Mounting filesystem, this will take a few seconds...")); 
      docloddet->gmo= gtk_mount_operation_new(docloddet->dialog_parent_window);
      g_file_mount_enclosing_volume (docloddet->file, G_MOUNT_MOUNT_NONE,docloddet->gmo, NULL, openfile_mount, doclod);
      g_error_free(error);
      return TRUE;
    }
    g_print(_("Error opening file GIO error:%s\nfile:%s"),error->message, g_file_get_uri(docloddet->file));
    g_error_free(error);
    return FALSE;
  }
  return TRUE;
}

gboolean validate_GFile_exist(GFile *file){
 return g_file_query_exists (file,NULL);
}

gboolean prompt_create_file(const gchar *filename){
  if (!filename) return TRUE;
  gint result;
  GString *dialog_message;
  dialog_message = g_string_new("");
  g_string_printf(dialog_message, _("The file %s was not found.\n\nWould you like to create it as an empty document?"), filename);
  result = yes_no_dialog(_("File not found"), dialog_message->str);
  g_string_free(dialog_message, TRUE);
  if (result != -8){ //GTK_RESPONSE_YES
    return FALSE;
  }
  return TRUE;
}
gboolean document_check_supported_type(GFile *file){
  GFileInfo *info;
  GError *error=NULL;
  gboolean result = TRUE;

  info= g_file_query_info (file,"standard::content-type",G_FILE_QUERY_INFO_NONE, NULL,&error);
  if (!info){
    g_warning (_("Could not get the file info. GIO error: %s \n"), error->message);
    g_error_free(error);
    return FALSE;
  }
  const char*contenttype= g_file_info_get_content_type (info); 
  /*we could open text based types so if it not a text based content don't open and displays error*/
  if (!IS_TEXT(contenttype) && IS_APPLICATION(contenttype)){
    info_dialog (_("gPHPEdit"), _("Sorry, I can open this kind of file.\n"));
    g_print("%s\n",contenttype);
    result = FALSE;
  }
  g_object_unref(info);
  return result;
}


void document_loader_load_document(DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
    if (!docloddet->file){
        document_set_untitled(docloddet->document, TRUE);
        document_set_file_icon(docloddet->document, gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "text-plain", ICON_SIZE, 0, NULL));
        g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE); 
        return ;
    }
    if (!GFile_is_local_or_http(docloddet->file)){ /* pasarle un GFile */
        if (!mount_mountable_file(doclod)){
             return ; /* later emit load complete signal */
            } else {
             g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
             return ;
            }
    }
    if(!validate_GFile_exist(docloddet->file) && document_get_document_type(docloddet->document)!=TAB_HELP && document_get_document_type(docloddet->document)!=TAB_PREVIEW) {
       gchar *filename =g_file_get_uri(docloddet->file);
       if (!prompt_create_file(filename)){
         g_free(filename);
         g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE); 
         return;
       } else {
        document_set_file_icon(docloddet->document, gtk_icon_theme_load_icon (gtk_icon_theme_get_default (), "text-plain", ICON_SIZE, 0, NULL));       
        document_set_untitled(docloddet->document, FALSE);
        document_set_readonly(docloddet->document, FALSE, FALSE);
        document_set_content_type(docloddet->document, "text/plain");
        g_free(filename);        
        g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE); 
        return;
       }
    }
  docloddet->webkit_action= LOAD;
  if (document_get_document_type(docloddet->document) == TAB_HELP) {
      document_set_untitled(docloddet->document, FALSE);
      document_set_content_type(docloddet->document, "text/plain");
      document_create_help(doclod);
  } else if (document_get_document_type(docloddet->document)== TAB_PREVIEW) {
    document_set_content_type(docloddet->document, "text/plain");
    document_set_untitled(docloddet->document, FALSE);
    tab_help_load_file(doclod);
  } else {
        /* TAB_FILE */ 
       if (!document_check_supported_type(docloddet->file)){
          g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE); 
          return ;
        }
        document_set_untitled(docloddet->document, FALSE);
        document_load_file(doclod);
  }
}

gboolean validate_and_convert_utf8_buffer (gchar *buffer) //FIXME: multiple enconding support
{
  gboolean result;
  if (g_utf8_validate(buffer, -1, NULL)) {
    gphpedit_debug_message (DEBUG_DOCUMENT,"%s", "Valid UTF8");
    result = FALSE;
  } else {
    gchar *converted_text;
    gsize utf8_size;
    GError *error = NULL;      
    converted_text = g_locale_to_utf8(buffer, -1, NULL, &utf8_size, &error); 
    if (error != NULL) {
      // if locale isn't set
      error=NULL;
      converted_text = g_convert(buffer, -1, "UTF-8", "ISO-8859-15", NULL, &utf8_size, &error);
      if (error!=NULL){
        gphpedit_debug_message (DEBUG_DOCUMENT,_("gPHPEdit UTF-8 Error: %s"), error->message);
        g_error_free(error);
      return FALSE;
      }
    }
    gphpedit_debug_message (DEBUG_DOCUMENT,_("Converted to UTF-8 size: %u"), utf8_size);
    g_free(buffer);
    buffer = g_strdup(converted_text);
    g_free(converted_text);
    result = TRUE;
    }
 return result;
}


void tab_file_opened (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  DocumentLoader *doclod = (DocumentLoader *) user_data;
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GError *error=NULL;
  gsize size; 
  if (!g_file_load_contents_finish ((GFile *) source_object,res,&(docloddet->buffer),&size,NULL,&error)) {
    g_print("Error reading file. Gio error:%s",error->message);
    g_error_free(error);
    if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
    } else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_REFRESH], 0, FALSE);
    }
    return ;
  }

  gphpedit_debug_message (DEBUG_DOCUMENT,"Loaded %u bytes",size);
  gphpedit_debug_message (DEBUG_DOCUMENT,"BUFFER=\n%s\n-------------------------------------------", docloddet->buffer);

  docloddet->converted_to_UTF8 = validate_and_convert_utf8_buffer(docloddet->buffer);
  docloddet->file_contents_len = strlen(docloddet->buffer);
  if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE);
  } else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_REFRESH], 0, TRUE);
  }
}

#define INFO_FLAGS "standard::display-name,standard::content-type,standard::edit-name,standard::size,access::can-write,standard::icon,time::modified,time::modified-usec"

void document_load_file_helper(DocumentLoader *doclod, gint action)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  docloddet->webkit_action = action;
  GFileInfo *info;
  GError *error=NULL;
  
  // Try getting file size
  info= g_file_query_info (docloddet->file,INFO_FLAGS,G_FILE_QUERY_INFO_NONE, NULL,&error);
  if (!info){
    g_warning (_("Could not get the file info. GIO error: %s \n"), error->message);
    if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
    } else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_REFRESH], 0, FALSE);
    }
    return;
  }
  document_set_readonly(docloddet->document, !g_file_info_get_attribute_boolean (info,"access::can-write"), FALSE);
  document_set_content_type(docloddet->document, g_file_info_get_content_type (info));
  GIcon *icon= g_file_info_get_icon (info); /* get Gicon for mimetype*/
  GtkIconInfo *ificon= gtk_icon_theme_lookup_by_gicon (gtk_icon_theme_get_default (), icon, ICON_SIZE, 0);
  document_set_file_icon(docloddet->document, gtk_icon_info_load_icon (ificon, NULL));
  GTimeVal file_mtime;
  g_file_info_get_modification_time (info,&file_mtime);
  document_set_mtime(docloddet->document, file_mtime);
  g_object_unref(info);
  gtk_icon_info_free(ificon);
  /* Open file*/
  /*it's all ok, read file*/
  g_file_load_contents_async (docloddet->file, NULL, tab_file_opened, doclod); //FIXME:: cancellable???
}
void document_load_file(DocumentLoader *doclod)
{
  document_load_file_helper(doclod, LOAD);
}

void tab_help_opened (GObject *source_object, GAsyncResult *res, gpointer user_data)
{
  DocumentLoader *doclod = (DocumentLoader *) user_data;
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GError *error=NULL;
  gsize size; 
  if (!g_file_load_contents_finish ((GFile *) source_object,res,&(docloddet->buffer),&size,NULL,&error)) {
    g_print("Error reading file. Gio error:%s",error->message);
    g_error_free(error);
    if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
    }else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_NAVIGATE], 0, FALSE);
    }
    return ;
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"Loaded %u bytes",size);
  gphpedit_debug_message (DEBUG_DOCUMENT,"BUFFER=\n%s\n-------------------------------------------", docloddet->buffer);

  docloddet->converted_to_UTF8 = validate_and_convert_utf8_buffer(docloddet->buffer);
  docloddet->file_contents_len = strlen(docloddet->buffer);
  if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, TRUE);
  } else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_NAVIGATE], 0, TRUE);
  }
}

/*
* tab_help_load_file
* loads a new help file async
*/
void tab_help_load_file(DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GFileInfo *info;
  GError *error=NULL;
  info=g_file_query_info (docloddet->file,"standard::icon",0,NULL,&error);
  if (!info){
    gchar *filename = document_get_filename(docloddet->document);
    g_warning (_("Could not get file info for file %s. GIO error: %s \n"), filename,error->message);
    g_free(filename);
    if (docloddet->webkit_action==LOAD){
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
    } else {
    g_signal_emit (G_OBJECT (doclod), signals[DONE_NAVIGATE], 0, FALSE);
    }
    return ;
  }
  GIcon *icon= g_file_info_get_icon (info); /* get Gicon for mimetype*/
  GtkIconInfo *ificon= gtk_icon_theme_lookup_by_gicon (gtk_icon_theme_get_default (), icon, ICON_SIZE, 0);
  document_set_file_icon(docloddet->document, gtk_icon_info_load_icon (ificon, NULL));
  g_object_unref(info);

  /* Open file*/
  /*it's all ok, read file*/
  g_file_load_contents_async (docloddet->file, NULL, tab_help_opened, doclod); //FIXME:: cancellable???
  return ;
}

GString *tab_help_try_filename(gchar *prefix, gchar *command, gchar *suffix)
{
  GString *long_filename;
        
  long_filename = g_string_new(command);
  long_filename = g_string_prepend(long_filename, prefix);
  if (suffix) {
    long_filename = g_string_append(long_filename, suffix);
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename: %s", long_filename->str);

  if (filename_file_exist(long_filename->str)){
    return long_filename;
  }
  else {
    g_string_free(long_filename, TRUE);
  }
  long_filename = g_string_new(command);
  str_replace(long_filename->str, '_', '-');
  long_filename = g_string_prepend(long_filename, prefix);
  if (suffix) {
    long_filename = g_string_append(long_filename, suffix);
  }
  gphpedit_debug_message (DEBUG_DOCUMENT,"filename: %s", long_filename->str);

  if (filename_file_exist(long_filename->str)){
    return long_filename;
  }
  g_string_free(long_filename, TRUE);
  return NULL;
}


GString *tab_help_find_helpfile(gchar *command)
{
  GString *long_filename = NULL;
  if (!command) return long_filename;
#ifdef PHP_DOC_DIR
 //FIX: avoid duplicated call
 if (strstr(command,"/usr/")!=NULL){
 return long_filename;
 }
 gchar *temp= NULL;
 temp=g_strdup_printf ("%s/%s",PHP_DOC_DIR,"function.");
 long_filename = tab_help_try_filename(temp, command, ".html");
 g_free(temp);
 if (long_filename)
  return long_filename;
 temp=g_strdup_printf ("%s/%s",PHP_DOC_DIR,"ref.");
 long_filename = tab_help_try_filename(temp, command, ".html");
 g_free(temp);
 if (long_filename)
  return long_filename;
 temp=g_strdup_printf ("%s/",PHP_DOC_DIR);
 long_filename = tab_help_try_filename(temp, command, NULL);
 g_free(temp);
 if (long_filename)
  return long_filename;
    gphpedit_debug_message (DEBUG_DOCUMENT,_("Help for function not found: %s"), command);
 return long_filename;
#else
  long_filename = g_string_new("http://www.php.net/manual/en/function.");
  long_filename = g_string_append(long_filename, command);
  long_filename = g_string_append(long_filename, ".php");
  GFile *temp= get_gfile_from_filename(long_filename->str);
  if(g_file_query_exists(temp,NULL)){
  g_object_unref(temp);
  return long_filename;
  }else{
  g_object_unref(temp);
  gphpedit_debug_message (DEBUG_DOCUMENT,_("Help for function not found: %s"), command);
  return NULL;
  }
#endif
}

void document_create_help(DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  GString *long_filename = NULL;
  long_filename = tab_help_find_helpfile((gchar *)document_get_help_function(docloddet->document));
  if (!long_filename) {
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s",_("Could not find the required command in the online help"));
    g_signal_emit (G_OBJECT (doclod), signals[DONE_LOADING], 0, FALSE);
    return ;
  }
  else {
    gphpedit_debug_message (DEBUG_DOCUMENT,"filename:%s", long_filename->str);
    docloddet->raw_uri = g_strdup(long_filename->str);
    docloddet->file=get_gfile_from_filename(long_filename->str);
    tab_help_load_file(doclod);
  }
}

void document_loader_reload_file(DocumentLoader *doclod, Document *document)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  docloddet->document = document;
  if (document_get_document_type(docloddet->document)!=TAB_HELP && document_get_document_type(docloddet->document)!=TAB_PREVIEW){
    document_load_file_helper(doclod, REFRESH);
  }
}

void document_navigate_url(DocumentLoader *doclod, Document *document, gchar *uri)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  docloddet->document = document;
  docloddet->file_contents_len = 0;
  docloddet->buffer = NULL;
  docloddet->webkit_action= NAVIGATE;
  GString *long_filename = NULL;
  if (document_get_document_type(docloddet->document)==TAB_HELP){
    if (!g_str_has_prefix(uri, "file:")) {
      long_filename = tab_help_find_helpfile(uri);
    } else {
      long_filename = g_string_new(uri);
    }
  } else {
  long_filename = g_string_new(uri);
  }
  if (!long_filename) {
    if (document_get_document_type(docloddet->document)==TAB_HELP){
    gphpedit_statusbar_flash_message (GPHPEDIT_STATUSBAR(main_window.appbar),0,"%s",_("Could not find the required command in the online help"));
    }
    g_signal_emit (G_OBJECT (doclod), signals[DONE_NAVIGATE], 0);
    return ;
  } else {
    gphpedit_debug_message (DEBUG_DOCUMENT,"filename:%s", long_filename->str);
    docloddet->file = get_gfile_from_filename(long_filename->str);
    tab_help_load_file(doclod);
  }
}
gboolean document_loader_get_UTF8_converted (DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  return docloddet->converted_to_UTF8;
}

gboolean document_loader_get_file_content_lenght (DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  return docloddet->file_contents_len;
}

const gchar *document_loader_get_file_contents (DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  return docloddet->buffer;
}

const gchar *document_loader_get_raw_uri (DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  return docloddet->raw_uri;
}


Document *document_loader_get_document (DocumentLoader *doclod)
{
  DocumentLoaderDetails *docloddet = DOCUMENT_LOADER_GET_PRIVATE(doclod);
  return docloddet->document;
}

