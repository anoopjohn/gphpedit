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

#include <config.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "main.h"

#include "debug.h"
#include "templates.h"
#include "gvfs_utils.h"

/*
* templates_manager private struct
*/
struct TemplatesManagerDetails
{
  GHashTable *templates;
  gboolean templates_updated;

};

#define BACKSLASH 92
#define NEWLINE 10
#define TAB 9


#define TEMPLATES_MANAGER_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    TEMPLATES_MANAGER_TYPE,\
					    TemplatesManagerDetails))

static gpointer parent_class;
static void templates_manager_finalize (GObject  *object);
static void  templates_manager_class_init (TemplatesManagerClass *klass);
static void template_db_open(TemplatesManagerDetails *tempmgdet);

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(TemplatesManager, templates_manager, G_TYPE_OBJECT);  

/*
* overide default contructor to make a singleton.
* see http://blogs.gnome.org/xclaesse/2010/02/11/how-to-make-a-gobject-singleton/
*/
static GObject* 
templates_manager_constructor (GType type,
                 guint n_construct_params,
                 GObjectConstructParam *construct_params)
{
  static GObject *self = NULL;

  if (self == NULL)
    {
      self = G_OBJECT_CLASS (parent_class)->constructor (
          type, n_construct_params, construct_params);
      g_object_add_weak_pointer (self, (gpointer) &self);
      return self;
    }

  return g_object_ref (self);
}

static void
templates_manager_class_init (TemplatesManagerClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
	object_class->finalize = templates_manager_finalize;
  object_class->constructor = templates_manager_constructor;
	g_type_class_add_private (klass, sizeof (TemplatesManagerDetails));
}

static void
templates_manager_init (TemplatesManager  *object)
{
	TemplatesManagerDetails *tempmgdet;
	tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(object);
  
  tempmgdet->templates = NULL;
  tempmgdet->templates_updated = FALSE;
  
  template_db_open(tempmgdet);
}

static GString *template_get_filename(void)
{
  GString *ret;
  ret = g_string_new( g_get_home_dir());
  ret = g_string_append(ret, "/.gphpedit/templates.txt");
  
  return ret;
}

static void template_save_entry(gpointer key, gpointer value, gpointer user_data)
{
  GString *file_contents= (GString *) user_data;
  if (file_contents){
  file_contents = g_string_new(key);
  } else{
  file_contents = g_string_append(file_contents,key);
  }
  file_contents = g_string_append(file_contents, "\t");
  file_contents = g_string_append(file_contents, value);
  if (!g_str_has_suffix(value,"#")){
  file_contents = g_string_append(file_contents, "#");
  }
}

static void template_db_save(TemplatesManagerDetails *tempmgdet)
{
  GString *template_filename;
  
  template_filename = template_get_filename();

  GString *file_contents=NULL;
      
  g_hash_table_foreach(tempmgdet->templates, template_save_entry, file_contents);
  if (file_contents){
    GFile *file=g_file_new_for_commandline_arg(template_filename->str);
    GError *error=NULL;
    if(!g_file_replace_contents (file, file_contents->str, file_contents->len,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
      g_print(_("CANNOT Create templates file:: %s\n"), error->message);
      g_error_free (error);
    }
    g_string_free(file_contents,TRUE);
    g_object_unref(file);
  }
  
  g_string_free(template_filename, TRUE);
}

static void template_db_close(TemplatesManagerDetails *tempmgdet)
{
  if (tempmgdet->templates) {
    if (tempmgdet->templates_updated) {
      template_db_save(tempmgdet);  
    }

    g_hash_table_destroy(tempmgdet->templates);
  }
}

static void
templates_manager_finalize (GObject *object)
{
  TemplatesManager *tempmg = TEMPLATES_MANAGER(object);
  TemplatesManagerDetails *tempmgdet;
	tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(tempmg);
  /* free object resources*/
  template_db_close(tempmgdet);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}


TemplatesManager *templates_manager_new (void)
{
	TemplatesManager *tempmg;
  tempmg = g_object_new (TEMPLATES_MANAGER_TYPE, NULL);
  
	return tempmg; /* return new object */
}

GList *get_templates_manager_templates_names(TemplatesManager *tempmg){
  g_return_val_if_fail (OBJECT_IS_TEMPLATES_MANAGER(tempmg), 0);
  TemplatesManagerDetails *tempmgdet;
	tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(tempmg);
  return g_hash_table_get_keys (tempmgdet->templates);
}


static gboolean template_db_create(gchar *filename)
{
  gboolean result=TRUE;
  GError *error=NULL;
  GString *contents=g_string_new(NULL);
  contents = g_string_append(contents,"class\tclass |\\n{\\n\\t\\n}\n#");
  contents = g_string_append(contents,"for\tfor (|; ; )\n{\\n\\t\\n}\n#");
  contents = g_string_append(contents,"function\tfunction |\\n{\\n\\t\\n}\n#");
  contents = g_string_append(contents,"if\tif (|) {\\n\\t\\n}\n#");
  contents = g_string_append(contents,"ife\tif (|) {\\n\\t\\n}\\nelse {\\n\\t\\n}\n#");
  contents = g_string_append(contents,"switch\tswitch(|) {\\n\\tcase ():;break;\\n\\tcase ():;break;\\n}\n#");
  contents = g_string_append(contents,"while\twhile (|)\\n{\\n\\t\\n}\n#");
  contents = g_string_append(contents,"declare\tdeclare(|);\n#");
  GFile *file =g_file_new_for_commandline_arg(filename);
  if(!g_file_replace_contents (file,contents->str,contents->len,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
    g_error_free (error);
    result=FALSE;
  }
  g_object_unref(file);
  return result;
}

static void template_db_open(TemplatesManagerDetails *tempmgdet)
{
  GString *template_filename;
  
  template_filename = template_get_filename();
   
  tempmgdet->templates = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify) g_free,  (GDestroyNotify) g_free);
    if (!filename_file_exist(template_filename->str)){
      if (!template_db_create(template_filename->str)){
      g_print(_("CANNOT Create templates file: %s\n"), template_filename->str);
      return;
    }
  }
  
  gchar *content=read_text_file_sync(template_filename->str);
    gchar **strings;
    strings = g_strsplit (content,"#",0);
    int i=0;
    while (strings[i] && strings[i][0]!=0){
      gchar *key, *value;
      gpointer *old_key=NULL, *old_value=NULL;
        
    /* get the first and the second field */
      key = (gpointer)strtok(strings[i], "\t");
      if(!key) continue;
        value = (gpointer)strtok(NULL, "\t");
      if(!value) continue;

    /* try looking up this key */
      if(g_hash_table_lookup_extended(tempmgdet->templates, key, old_key, old_value)) {
          /* insert the new value */
          g_hash_table_insert(tempmgdet->templates, g_strdup(key), g_strdup(value));

          /* just free the key and value */
          g_free(old_key);
          g_free(old_value);
      } else {
          /* insert the new value */
          g_hash_table_insert(tempmgdet->templates, g_strdup(key), g_strdup(value));
      }
      i++;
    }
    g_strfreev (strings);
    g_free(content);
  
  g_string_free(template_filename, TRUE);
}

gchar *template_find(TemplatesManager *tempmg, gchar *key)
{
  TemplatesManagerDetails *tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(tempmg);
  return g_hash_table_lookup(tempmgdet->templates, key);
}

void template_find_and_insert(TemplatesManager *tempmg, Document *doc)
{
  gchar *buffer = NULL;
  gchar *template = NULL;
 
  buffer = document_get_current_word(doc);
  template = template_find(tempmg, buffer);
  if (template) {
    document_insert_template(doc, template);
  }
  g_free(buffer);
}

gchar *template_convert_to_display(gchar *content)
{
  char buf_in[16384];
  char buf_out[16384];
  gint read_buffer_pos;
  gint write_buffer_pos;
  gchar *ret;
  
  strncpy(buf_in, content, 16383);
  read_buffer_pos = 0;
  write_buffer_pos = 0;
  while (buf_in[read_buffer_pos] && read_buffer_pos<16380) {// 16384 - a few to account for lookaheads
    if (buf_in[read_buffer_pos] == BACKSLASH && buf_in[read_buffer_pos+1] == 'n') { 
      buf_out[write_buffer_pos] = NEWLINE;
      read_buffer_pos++; 
    }
    else if (buf_in[read_buffer_pos] == BACKSLASH && buf_in[read_buffer_pos+1] == 't') { 
      buf_out[write_buffer_pos] = TAB;
      read_buffer_pos++;
    }
    else {
      buf_out[write_buffer_pos] = buf_in[read_buffer_pos];
    }
    write_buffer_pos++;
    read_buffer_pos++;
  }
  buf_out[write_buffer_pos] = 0;
  ret = g_malloc(write_buffer_pos+1);
  memcpy(ret, buf_out, write_buffer_pos+1);
  
  return ret;
}

gchar *template_convert_to_template(gchar *content)
{
  char buf_in[16384];
  char buf_out[16384];
  gint read_buffer_pos;
  gint write_buffer_pos;
  gchar *ret;
  
  strncpy(buf_in, content, 16383);
  read_buffer_pos = 0;
  write_buffer_pos = 0;
  while (buf_in[read_buffer_pos] && read_buffer_pos<16380) {// 16384 - a few to account for lookaheads
    if (buf_in[read_buffer_pos] == NEWLINE) { 
      buf_out[write_buffer_pos] = BACKSLASH;
      buf_out[write_buffer_pos+1] = 'n';
      write_buffer_pos++; 
    }
    else if (buf_in[read_buffer_pos] == TAB) { 
      buf_out[write_buffer_pos] = BACKSLASH;
      buf_out[write_buffer_pos+1] = 't';
      write_buffer_pos++; 
    }
    else {
      buf_out[write_buffer_pos] = buf_in[read_buffer_pos];
    }
    write_buffer_pos++;
    read_buffer_pos++;
  }
  if (buf_out[write_buffer_pos-1] != NEWLINE) {
    buf_out[write_buffer_pos] = NEWLINE;
    write_buffer_pos++;
  }
  buf_out[write_buffer_pos] = 0;
  ret = g_malloc(write_buffer_pos+1);
  memcpy(ret, buf_out, write_buffer_pos+1);

  return ret;
}

void template_replace(TemplatesManager *tempmg, gchar *key, gchar *value)
{
  TemplatesManagerDetails *tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(tempmg);
  // ALSO USED TO INSERT
  gpointer *old_key=NULL, *old_value=NULL;
  
  /* try looking up this key */
    if(g_hash_table_lookup_extended(tempmgdet->templates, key, old_key, old_value)) {
      /* insert the new value */
        g_hash_table_insert(tempmgdet->templates, g_strdup(key), g_strdup(value));

        /* just free the key and value */
        g_free(old_key);
        g_free(old_value);
    }
  else {
        /* insert the new value */
        g_hash_table_insert(tempmgdet->templates, g_strdup(key), g_strdup(value));
  }
  tempmgdet->templates_updated = TRUE;
}

void template_delete(TemplatesManager *tempmg, gchar *key)
{
  TemplatesManagerDetails *tempmgdet = TEMPLATES_MANAGER_GET_PRIVATE(tempmg);
  gpointer *old_key=NULL, *old_value=NULL;

  if (key == NULL) {
    return;
  }
  
  /* try looking up this key */
  if(g_hash_table_lookup_extended(tempmgdet->templates, key, old_key, old_value)) {
    /* remove the entry in the hash table */
    g_hash_table_remove(tempmgdet->templates, key);

    tempmgdet->templates_updated = TRUE;
  }
}
