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
#ifndef TEMPLATES_MANAGER_H
#define TEMPLATES_MANAGER_H

#include <gtk/gtk.h>
#include "document.h"

#define TEMPLATES_MANAGER_TYPE templates_manager_get_type()
#define TEMPLATES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), TEMPLATES_MANAGER_TYPE, TemplatesManager))
#define TEMPLATES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), TEMPLATES_MANAGER_TYPE, TemplatesManagerClass))
#define OBJECT_IS_TEMPLATES_MANAGER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TEMPLATES_MANAGER_TYPE))
#define OBJECT_IS_TEMPLATES_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), TEMPLATES_MANAGER_TYPE))
#define TEMPLATES_MANAGER_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), TEMPLATES_MANAGER_TYPE, TemplatesManagerClass))

typedef struct TemplatesManagerDetails TemplatesManagerDetails;

typedef struct
{
	GObject object;
	TemplatesManagerDetails *details;
} TemplatesManager;

typedef struct
{
	GObjectClass parent_class;

} TemplatesManagerClass;

/* Basic GObject requirements. */
GType templates_manager_get_type (void) G_GNUC_CONST;
TemplatesManager *templates_manager_new (void);
gchar *template_find(TemplatesManager *tempmg, gchar *key);
void template_find_and_insert(TemplatesManager *tempmg, Document *doc);
gchar *template_convert_to_display(gchar *content);
gchar *template_convert_to_template(gchar *content);
GList *get_templates_manager_templates_names(TemplatesManager *tempmg);
void template_delete(TemplatesManager *tempmg, gchar *key);
void template_replace(TemplatesManager *tempmg, gchar *key, gchar *value);
#endif /* TEMPLATES_MANAGER_MANAGER_H */

