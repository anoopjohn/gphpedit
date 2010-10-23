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

#include <stdlib.h>
#include <stdarg.h>
#include <gio/gio.h>
#include "main.h"
#include "main_window.h"
#include "main_window_callbacks.h"
#include "gphpedit_ipc.h"
#include "templates.h"
#include "classbrowser_ui.h"

#include "debug.h"

#ifdef ENABLE_NLS
#include <locale.h>
#endif              /* ENABLE_NLS */

int main (int argc, char **argv)
{
#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif /* ENABLE_NLS */

  gtk_init(&argc, &argv);
#if GLIB_CHECK_VERSION (2, 24, 0)
  g_type_init();
#else 
  if (!g_thread_supported()) g_thread_init(NULL);
#endif

  gphpedit_debug_init ();
        
  main_window.prefmg = preferences_manager_new();

  gboolean single_instance;

  g_object_get(main_window.prefmg, "single_instance_only", &single_instance, NULL);
  /* Start of IPC communication */
  if (single_instance && poke_existing_instance (argc - 1, argv + 1)) return 0;

  main_window_create();
  main_window.tempmg = templates_manager_new();
  main_window.docmg = document_manager_new_full(argv, argc);
  gtk_main();
        
  /* it makes sense to install sigterm handler that would call this too */
  shutdown_ipc ();

  return 0;
}
