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
  gint (*get_current_position) (Documentable *iface);
  gboolean (*search_text) (Documentable *iface, const gchar *text, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex);
  gboolean (*search_replace_text) (Documentable *iface, const gchar *text, const gchar *replace, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace);
  void (*replace_text) (Documentable *iface, gchar *new_text);
  void (*insert_text) (Documentable *iface, gchar *new_text);
  void (*replace_current_selection) (Documentable *iface, gchar *new_text);

};

GType          documentable_get_type        (void) G_GNUC_CONST;

//FIXME: poner una descripcion acerca de que hace cada metodo de la interfaz
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
gchar          *documentable_get_text (Documentable  *self) G_GNUC_WARN_UNUSED_RESULT G_GNUC_MALLOC; //se puede hacer en webkit??
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
void           documentable_save (Documentable  *self);
void           documentable_save_as (Documentable  *self, GFile *file);
void           documentable_replace_text (Documentable  *self, gchar *new_text);
void           documentable_insert_text (Documentable  *self, gchar *new_text);
void           documentable_replace_current_selection (Documentable  *self, gchar *new_text);

G_END_DECLS

#endif /* __DOCUMENTABLE_H__ */


