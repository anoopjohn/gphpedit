/* This file is part of gPHPEdit, a GNOME2 PHP Editor.

   Copyright (C) 2003, 2004, 2005 Andy Jeffries
      andy@gphpedit.org

   For more information or to find the latest release, visit our
   website at http://www.gphpedit.org/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>
#include "main.h"
#include "main_window.h"
#include "main_window_callbacks.h"
#include "gphpedit_ipc.h"
#include "templates.h"


GnomeProgram* gphpedit_program;


int main (int argc, char **argv)
{
	//GError *error;
	gboolean vfs_inited;

	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	gphpedit_program = gnome_program_init ("gPHPEdit", VERSION, LIBGNOMEUI_MODULE,
	                                       argc, argv, NULL);

	vfs_inited = gnome_vfs_init();
	g_assert(vfs_inited);

	preferences_load();

    /* Start of IPC communication */
    if (preferences.single_instance_only && poke_existing_instance (argc - 1, argv + 1))
        return 0;

	main_window_create();

	force_config_folder();

	template_db_open();
	main_window_open_command_line_files(argv, argc);

	if (main_window.current_editor == NULL) {
		session_reopen();
	}

	create_untitled_if_empty();

	gtk_main();

	/* it makes sense to install sigterm handler that would call this too */
    shutdown_ipc ();

	return 0;
}
