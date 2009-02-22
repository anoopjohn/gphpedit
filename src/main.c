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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnomevfs/gnome-vfs.h>
#include <stdarg.h>
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

gint debug(char *formatstring, ...)
{
  GtkWidget *dlg;
	int intReturn;
	char *buf, *temp;
	temp = (char*) calloc(500, sizeof(char));
	buf = (char*) calloc(500, sizeof(char));
	va_list arguments;
	va_start(arguments, formatstring);
	int i  ;
	for (i =0 ; formatstring[i] != '\0'; i++) {
		if (formatstring[i] != '%' ) {
			strncat(buf, formatstring + i, 1);
		}
		else {
			switch (formatstring[++i]) {
				case 's':
					strcat(buf, va_arg(arguments, char*));
					continue;
				case 'd':
					sprintf(temp, "%d", va_arg(arguments, int));
					strcat(buf, temp);
					continue;
				case 'f':
					sprintf(temp, "%f", va_arg(arguments, double));
					strcat(buf, temp);
					continue;
				case 'l':
					sprintf(temp, "%ld", va_arg(arguments, long));
					strcat(buf,temp);
					continue;
				case 'c':
					sprintf(temp, "%c", va_arg(arguments, int));
					strcat(buf,temp);
					continue;
				case 'p':
					sprintf(temp, "%p", va_arg(arguments, int*));
					strcat(buf,temp);
				continue;
			}
		}
	}
  dlg = gtk_message_dialog_new (main_window.window,
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_INFO,
                                GTK_BUTTONS_CLOSE,
                                "%s",
                                buf);
  gtk_dialog_run (GTK_DIALOG (dlg));
  gtk_widget_destroy (dlg);  
	va_end(arguments);
	return 0;
}
