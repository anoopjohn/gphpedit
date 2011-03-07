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

#include "language_provider.h"

G_DEFINE_INTERFACE(Language_Provider, language_provider, G_TYPE_INVALID)

static void
language_provider_default_init (Language_ProviderInterface *iface)
{
}

void language_provider_trigger_completion (Language_Provider  *self, guint ch)
{
  if (!self) return ;
  LANGUAGE_PROVIDER_GET_IFACE (self)->trigger_completion (self, ch);
}

void language_provider_show_calltip (Language_Provider *self)
{
  if (!self) return ;
  LANGUAGE_PROVIDER_GET_IFACE (self)->show_calltip (self);
}

void language_provider_setup_lexer (Language_Provider *self)
{
  if (!self) return ;
  LANGUAGE_PROVIDER_GET_IFACE (self)->setup_lexer (self);
}
