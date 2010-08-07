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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with gPHPEdit.  If not, see <http://www.gnu.org/licenses/>.
 
   The GNU General Public License is contained in the file COPYING.
*/

/* a lot of the code in this unit is from:
  http://www-106.ibm.com/developerworks/linux/library/l-glib2.html
*/


#include "tab.h"
#include "main_window_callbacks.h"
#include "templates.h"
#include "gvfs_utils.h"

#define BACKSLASH 92
#define NEWLINE 10
#define TAB 9


GHashTable *templates = NULL;
gboolean templates_updated = FALSE;

gboolean template_db_create(gchar *filename)
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
  GFile *file =get_gfile_from_filename(filename);
  if(!g_file_replace_contents (file,contents->str,contents->len,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
    g_error_free (error);
    result=FALSE;
  }
  g_object_unref(file);
  return result;
}

GString *template_get_filename(void)
{
  GString *ret;
  ret = g_string_new( g_get_home_dir());
  ret = g_string_append(ret, "/.gphpedit/templates.txt");
  
  return ret;
}

void template_db_open(void)
{
  GString *template_filename;
  
  template_filename = template_get_filename();
   
  templates = g_hash_table_new(g_str_hash, g_str_equal);
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
      if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
          /* insert the new value */
          g_hash_table_insert(templates, g_strdup(key), g_strdup(value));

          /* just free the key and value */
          g_free(old_key);
          g_free(old_value);
      } else {
          /* insert the new value */
          g_hash_table_insert(templates, g_strdup(key), g_strdup(value));
      }
      i++;
    }
    g_strfreev (strings);
    g_free(content);
  
  g_string_free(template_filename, TRUE);
}
  GString *file_contents=NULL;
static void template_save_entry(gpointer key, gpointer value, gpointer user_data)
{
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

void template_db_save(void)
{
  GString *template_filename;
  
  template_filename = template_get_filename();
    
  g_hash_table_foreach(templates, template_save_entry, NULL);
  if (file_contents){
    GFile *file=get_gfile_from_filename(template_filename->str);
    GError *error=NULL;
    if(!g_file_replace_contents (file,file_contents->str,file_contents->len,NULL,FALSE,G_FILE_CREATE_NONE,NULL,NULL,&error)){
      g_print(_("CANNOT Create templates file:: %s\n"),error->message);
      g_error_free (error);
    }
    g_string_free(file_contents,TRUE);
    g_object_unref(file);
  }
  
  g_string_free(template_filename, TRUE);
}


static void free_a_hash_table_entry(gpointer key, gpointer value, gpointer user_data)
{
  g_free(key);
  g_free(value);
}

void template_db_close(void)
{
  if (templates) {
    if (templates_updated) {
      template_db_save();  
    }

    g_hash_table_foreach(templates, free_a_hash_table_entry, NULL);
    g_hash_table_destroy(templates);
  }
}

gchar *template_find(gchar *key)
{
  return g_hash_table_lookup(templates, key);
}

void template_replace(gchar *key, gchar *value)
{
  // ALSO USED TO INSERT
  gpointer *old_key=NULL, *old_value=NULL;
  
  /* try looking up this key */
    if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
      /* insert the new value */
        g_hash_table_insert(templates, g_strdup(key), g_strdup(value));

        /* just free the key and value */
        g_free(old_key);
        g_free(old_value);
    }
  else {
        /* insert the new value */
        g_hash_table_insert(templates, g_strdup(key), g_strdup(value));
  }
  templates_updated = TRUE;
}

void template_delete(gchar *key)
{
  gpointer *old_key=NULL, *old_value=NULL;

  if (key == NULL) {
    return;
  }
  
  /* try looking up this key */
  if(g_hash_table_lookup_extended(templates, key, old_key, old_value)) {
      /* remove the entry in the hash table */
    g_hash_table_remove(templates, key);

    /* just free the key and value */
    g_free(old_key);
    g_free(old_value);
    templates_updated = TRUE;
  }
}

void template_find_and_insert()
{
  gchar *buffer = NULL;
  gchar *template = NULL;
 
  buffer = document_get_current_word(main_window.current_document);
  template = template_find(buffer);
  if (template) {
    document_insert_template(main_window.current_document, template);
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
