/* This file is part of gPHPEdit, a GNOME PHP Editor.

   Copyright (C) 2011 José Rostagno (for vijona.com.ar)

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

#ifndef __DOCUMENTABLE_H__
#define __DOCUMENTABLE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IFACE_TYPE_DOCUMENTABLE             (documentable_get_type ())
#define DOCUMENTABLE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), IFACE_TYPE_DOCUMENTABLE, Documentable))
#define DOCUMENTABLE_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), IFACE_TYPE_DOCUMENTABLE, DocumentableIface))
#define IFACE_IS_DOCUMENTABLE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IFACE_TYPE_DOCUMENTABLE))
#define IFACE_IS_DOCUMENTABLE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), IFACE_TYPE_DOCUMENTABLE))
#define DOCUMENTABLE_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), IFACE_TYPE_DOCUMENTABLE, DocumentableIface))


typedef struct _Documentable       Documentable;         /* Dummy typedef */
typedef struct _DocumentableIface  DocumentableIface;
typedef struct _DocumentableIface  DocumentableInterface;

struct _DocumentableIface
{
  GTypeInterface base_iface;
  void (*zoom_in) (Documentable *iface);
  void (*zoom_out) (Documentable *iface);
  void (*zoom_restore) (Documentable *iface);
  void (*undo) (Documentable *iface);
  void (*redo) (Documentable *iface);
  void (*selection_to_lower) (Documentable *iface);
  void (*selection_to_upper) (Documentable *iface);
  void (*select_all) (Documentable *iface);
  void (*copy) (Documentable *iface);
  void (*cut) (Documentable *iface);
  void (*paste) (Documentable *iface);
  void (*block_indent) (Documentable *iface);
  void (*block_unindent) (Documentable *iface);
  gchar *(*get_filename) (Documentable *iface);
  gchar *(*get_current_selected_text) (Documentable *iface);
  gchar *(*get_session_entry) (Documentable *iface);
  gchar *(*get_text) (Documentable *iface);
  gchar *(*get_current_word) (Documentable *iface);
  void (*set_type) (Documentable *iface, gint type);
  void (*goto_pos) (Documentable *iface, glong pos);
  void (*goto_line) (Documentable *iface, gint line);
  void (*scroll_to_current_pos) (Documentable *iface);
  void (*reload) (Documentable *iface);
  void (*check_externally_modified) (Documentable *iface);
  void (*save) (Documentable *iface);
  void (*save_as) (Documentable *iface, GFile *file);
  void (*incremental_search) (Documentable *iface, gchar *current_text, gboolean advancing);
  void (*activate_incremental_search) (Documentable *iface);
  gint (*get_current_position) (Documentable *iface);
  gboolean (*search_text) (Documentable *iface, const gchar *text, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex);
  gboolean (*search_replace_text) (Documentable *iface, const gchar *text, const gchar *replace, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace);
  void (*replace_text) (Documentable *iface, gchar *new_text);
  void (*insert_text) (Documentable *iface, gchar *new_text);
  void (*replace_current_selection) (Documentable *iface, gchar *new_text);
  void (*apply_preferences) (Documentable *iface);
  void (*grab_focus) (Documentable *iface);
};

GType          documentable_get_type        (void) G_GNUC_CONST;

void           documentable_reload (Documentable  *self);
void           documentable_zoom_in (Documentable  *self);
void           documentable_zoom_out (Documentable  *self);
void           documentable_zoom_restore (Documentable  *self);
void           documentable_undo (Documentable  *self);
void           documentable_redo (Documentable  *self);
void           documentable_selection_to_lower (Documentable  *self);
void           documentable_selection_to_upper (Documentable  *self);
void           documentable_select_all (Documentable  *self);
void           documentable_copy (Documentable  *self);
void           documentable_cut (Documentable  *self);
void           documentable_paste (Documentable  *self);
void           documentable_block_indent (Documentable  *self);
void           documentable_block_unindent (Documentable  *self);
gchar          *documentable_get_filename (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
gchar          *documentable_get_current_selected_text (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
gchar          *documentable_get_session_entry (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
gchar          *documentable_get_text (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
gchar          *documentable_get_current_word (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC;
void           documentable_set_type (Documentable  *self, gint type);
void           documentable_goto_pos (Documentable  *self, glong pos);
void           documentable_goto_line (Documentable  *self, gint line);
void           documentable_scroll_to_current_pos (Documentable  *self);
void           documentable_check_externally_modified (Documentable  *self);
gint           documentable_get_current_position (Documentable  *self);
gboolean       documentable_search_text (Documentable  *self, const gchar *text, gboolean checkwholedoc, 
                gboolean checkcase, gboolean checkwholeword, gboolean checkregex);
gboolean       documentable_search_replace_text(Documentable  *self, const gchar *text, const gchar *replace, 
                gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace);
void           documentable_incremental_search (Documentable  *self, gchar *current_text, gboolean advancing);
void           documentable_activate_incremental_search (Documentable  *self);
void           documentable_save (Documentable  *self);
void           documentable_save_as (Documentable  *self, GFile *file);
void           documentable_replace_text (Documentable  *self, gchar *new_text);
void           documentable_insert_text (Documentable  *self, gchar *new_text);
void           documentable_replace_current_selection (Documentable  *self, gchar *new_text);
void           documentable_apply_preferences (Documentable  *self);
void           documentable_grab_focus (Documentable  *self);

G_END_DECLS

#endif /* __DOCUMENTABLE_H__ */


