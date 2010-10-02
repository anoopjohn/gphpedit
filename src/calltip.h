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
#ifndef CALLTIP_MANAGER_H
#define CALLTIP_MANAGER_H

#include <gtk/gtk.h>
#include "tab.h"


#define CALLTIP_MANAGER_TYPE calltip_manager_get_type()
#define CALLTIP_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), CALLTIP_MANAGER_TYPE, CalltipManager))
#define CALLTIP_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), CALLTIP_MANAGER_TYPE, CalltipManagerClass))
#define OBJECT_IS_CALLTIP_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CALLTIP_MANAGER_TYPE))
#define OBJECT_IS_CALLTIP_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), CALLTIP_MANAGER_TYPE))
#define CALLTIP_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), CALLTIP_MANAGER_TYPE, CalltipManagerClass))

typedef struct CalltipManagerDetails CalltipManagerDetails;

typedef struct
{
	GObject object;
	CalltipManagerDetails *details;
} CalltipManager;

typedef struct
{
	GObjectClass parent_class;

} CalltipManagerClass;

/* Basic GObject requirements. */
GType calltip_manager_get_type (void) G_GNUC_CONST;
CalltipManager *calltip_manager_new (void);
gchar *calltip_manager_show_call_tip(CalltipManager *calltipmg, gint type, gchar *prefix);
gchar *calltip_manager_autocomplete_word(CalltipManager *calltipmg, gint type, gchar *buffer);
#endif /* CALLTIP_MANAGER_H */

