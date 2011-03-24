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

#include "documentable.h"

G_DEFINE_INTERFACE(Documentable, documentable, G_TYPE_INVALID)

static void
documentable_default_init (DocumentableInterface *iface)
{
}

void documentable_reload (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->reload (self);
}

void documentable_zoom_in (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->zoom_in (self);
}

void documentable_zoom_out (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->zoom_out (self);
}

void documentable_zoom_restore (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->zoom_restore (self);
}

void documentable_undo (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->undo (self);
}

void documentable_redo (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->redo (self);
}

void documentable_selection_to_lower (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->selection_to_lower (self);
}

void documentable_selection_to_upper (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->selection_to_upper (self);
}

void documentable_select_all (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->select_all (self);
}

void documentable_copy (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->copy (self);
}

void documentable_cut (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->cut (self);
}

void documentable_paste (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->paste (self);
}

void documentable_block_indent (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->block_indent (self);
}

void documentable_block_unindent (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->block_unindent (self);
}

gchar *documentable_get_filename (Documentable  *self)
{
  if (!self) return NULL;
	return DOCUMENTABLE_GET_IFACE (self)->get_filename (self);
}

gchar *documentable_get_current_selected_text (Documentable  *self)
{
  if (!self) return NULL;
	return DOCUMENTABLE_GET_IFACE (self)->get_current_selected_text (self);
}

gchar *documentable_get_session_entry (Documentable  *self)
{
  if (!self) return NULL;
	return DOCUMENTABLE_GET_IFACE (self)->get_session_entry (self);
}

void documentable_set_type (Documentable  *self, gint type)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->set_type (self, type);
}

void documentable_goto_pos (Documentable  *self, glong pos)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->goto_pos (self, pos);
}

void documentable_goto_line (Documentable  *self, gint line)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->goto_line (self, line);
}

void documentable_scroll_to_current_pos (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->scroll_to_current_pos (self);
}

gint documentable_get_current_position (Documentable  *self)
{
  if (!self) return 0;
	return DOCUMENTABLE_GET_IFACE (self)->get_current_position (self);
}

gboolean documentable_search_text (Documentable  *self, const gchar *text, gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex)
{
  if (!self) return FALSE;
	return DOCUMENTABLE_GET_IFACE (self)->search_text (self, text, checkwholedoc, checkcase, checkwholeword, checkregex);
}

gboolean documentable_search_replace_text(Documentable  *self, const gchar *text, const gchar *replace, 
                gboolean checkwholedoc, gboolean checkcase, gboolean checkwholeword, gboolean checkregex, gboolean ask_replace)
{
  if (!self) return FALSE;
	return DOCUMENTABLE_GET_IFACE (self)->search_replace_text (self, text, replace, checkwholedoc, checkcase, checkwholeword, checkregex, ask_replace);
}

void documentable_incremental_search (Documentable  *self, gchar *current_text, gboolean advancing)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->incremental_search (self, current_text, advancing);
}

void documentable_activate_incremental_search (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->activate_incremental_search (self);
}

gchar *documentable_get_text (Documentable  *self)
{
  if (!self) return NULL;
	return DOCUMENTABLE_GET_IFACE (self)->get_text (self);
}

gchar *documentable_get_current_word (Documentable  *self)
{
	return DOCUMENTABLE_GET_IFACE (self)->get_current_word (self);
}

void documentable_check_externally_modified (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->check_externally_modified (self);
}

void documentable_save (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->save (self);
}

void documentable_save_as (Documentable  *self, GFile *file)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->save_as (self, file);
}

void documentable_replace_text (Documentable  *self, gchar *new_text)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->replace_text (self, new_text);
}

void documentable_insert_text (Documentable  *self, gchar *new_text)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->insert_text (self, new_text);
}

void documentable_replace_current_selection (Documentable  *self, gchar *new_text)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->replace_current_selection (self, new_text);
}

void documentable_apply_preferences (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->apply_preferences (self);
}

void documentable_grab_focus (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->grab_focus (self);
}

void documentable_do_syntax_check (Documentable  *self)
{
  if (!self) return ;
  DOCUMENTABLE_GET_IFACE (self)->do_syntax_check (self);
}


