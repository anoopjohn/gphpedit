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

#ifndef __LANGUAGE_PROVIDER_H__
#define __LANGUAGE_PROVIDER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define IFACE_TYPE_LANGUAGE_PROVIDER             (language_provider_get_type ())
#define LANGUAGE_PROVIDER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), IFACE_TYPE_LANGUAGE_PROVIDER, Language_Provider))
#define LANGUAGE_PROVIDER_CLASS(vtable)    (G_TYPE_CHECK_CLASS_CAST ((vtable), IFACE_TYPE_LANGUAGE_PROVIDER, Language_ProviderIface))
#define IFACE_IS_LANGUAGE_PROVIDER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), IFACE_TYPE_LANGUAGE_PROVIDER))
#define IFACE_IS_LANGUAGE_PROVIDER_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE ((vtable), IFACE_TYPE_LANGUAGE_PROVIDER))
#define LANGUAGE_PROVIDER_GET_IFACE(inst)  (G_TYPE_INSTANCE_GET_INTERFACE ((inst), IFACE_TYPE_LANGUAGE_PROVIDER, Language_ProviderIface))


typedef struct _Language_Provider       Language_Provider;         /* Dummy typedef */
typedef struct _Language_ProviderIface  Language_ProviderIface;
typedef struct _Language_ProviderIface  Language_ProviderInterface;

struct _Language_ProviderIface
{
  GTypeInterface base_iface;
  void (*trigger_completion) (Language_Provider *iface, guint ch);
  void (*show_calltip) (Language_Provider *iface);
  void (*setup_lexer) (Language_Provider *iface);
};

GType          language_provider_get_type        (void) G_GNUC_CONST;

void           language_provider_trigger_completion (Language_Provider *self, guint ch);
void           language_provider_show_calltip (Language_Provider *self);
void           language_provider_setup_lexer (Language_Provider *self);

G_END_DECLS

#endif /* __LANGUAGE_PROVIDER_H__ */


