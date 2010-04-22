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

#include "main.h"
#include "gphpedit_ipc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <stdio.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>

#define PIPE_PREFIX         "%s-gphpedit-"
#define PIPE_PREFIX_FULL    "%s-gphpedit-%d"
#define MAX_BUFFER_SIZE 4096


typedef struct
{
    int          pipe;
    char        *pipe_name;
    GIOChannel  *io;
    guint        io_watch;
    GByteArray  *buffer;
    gboolean     ready;
} Input;

static Input *input = NULL;


static void start_ipc (void);
/* this is the function that does stuff when receives data */
static void commit (Input *self);


gboolean
poke_existing_instance (int argc, char **argv)
{
    GDir *dir;
    const char *entry, *username, *tmp_path;
    char *prefix;
    guint prefix_len;
    char *cur_dir_tmp;
    char *cur_dir;
    char *to_open;

    g_return_val_if_fail (input == NULL, FALSE);

    tmp_path = g_get_tmp_dir ();
    dir = g_dir_open (tmp_path, 0, NULL);
    g_return_val_if_fail (dir != NULL, FALSE);

    username = g_get_user_name ();
    prefix = g_strdup_printf (PIPE_PREFIX, username);
    prefix_len = strlen (prefix);

   cur_dir_tmp = g_get_current_dir();
   cur_dir=NULL;
   cur_dir = g_strdup_printf("%s/", cur_dir_tmp);
   g_free(cur_dir_tmp);

    /* if another process creates a pipe while we are doing this,
       we may not get that pipe here. dunno if it's a problem */
    while ((entry = g_dir_read_name (dir)))
    {
        if (!strncmp (entry, prefix, prefix_len))
        {
            const char *pid_string;
            pid_t pid;
            char *filename;

            //g_print ("%s\n", entry);

            pid_string = entry + prefix_len;
            errno = 0;
            /* this is not right, but should not cause real problems */
            pid = strtol (pid_string, NULL, 10);
            filename = g_build_filename (tmp_path, entry, NULL);
            //g_print ("filename: %s\n", filename);

            if (!errno && pid > 0 && !kill (pid, 0))
            {
                int i;
                /* it would be cool to check that the file is indeed a fifo,
                   but again, who cares? */
                int fd = open (filename, O_WRONLY | O_NONBLOCK);

                if (fd == -1)
                {
                    perror ("open");
                    unlink (filename);
                }
                else
                {
                    /* this is wrong too, but i am lazy to do error checking right now */
                    write (fd, "", 1);
                    for (i = 0; i < argc; ++i) {
						if (g_path_is_absolute(argv[i])){
							to_open = g_strdup ((gchar *) argv[i]);
								} else {
							GString *s_filename;
							s_filename = g_string_new(cur_dir);
							s_filename = g_string_append(s_filename, (gchar *) argv[i]);
							to_open=s_filename->str;
						g_free(s_filename);
}
						if (to_open) {
							//g_print("From %s to %s\n", argv[i], to_open);
							write (fd, to_open, strlen (to_open) + 1);
							g_free(to_open);
						}
					}
                    close (fd);
                    g_free (filename);
					g_dir_close (dir);
					g_free (prefix);
					return TRUE;                }
            }
            else
            {
                /* delete it. who cares? */
                unlink (filename);
            }

            g_free (filename);
        }
    }

    g_dir_close (dir);
    start_ipc ();
    g_free (prefix);
    g_free(cur_dir);
    return FALSE;
}


static void
commit (Input *self)
{
    g_assert (self->buffer->len > 0 && self->buffer->data[self->buffer->len-1] == 0);

    if (self->buffer->len <= 1) {
		gtk_window_present(GTK_WINDOW(main_window.window));
		//g_print("Presenting\n");
	}
    else {
		switch_to_file_or_open((char*) self->buffer->data, 0);
		//g_print("Opening %s\n", (char*) self->buffer->data);
	}

    if (self->buffer->len > MAX_BUFFER_SIZE)
    {
        g_byte_array_free (self->buffer, TRUE);
        self->buffer = g_byte_array_new ();
    }
    else
    {
        g_byte_array_set_size (self->buffer, 0);
    }
}


static gboolean
read_input (G_GNUC_UNUSED GIOChannel *source,
            GIOCondition condition,
            Input       *self)
{
    gboolean error_occured = FALSE;
    GError *err = NULL;
    gboolean again = TRUE;
    gboolean got_zero = FALSE;

    if (condition & (G_IO_ERR | G_IO_HUP))
        if (errno != EINTR && errno != EAGAIN)
            error_occured = TRUE;

    while (again && !error_occured && !err)
    {
        char c;
        int bytes_read;

        struct pollfd fd = {self->pipe, POLLIN | POLLPRI, 0};

        int res = poll (&fd, 1, 0);

        switch (res)
        {
            case -1:
                if (errno != EINTR && errno != EAGAIN)
                    error_occured = TRUE;
                perror ("poll");
                break;

            case 0:
                again = FALSE;
                break;

            case 1:
                if (fd.revents & (POLLERR))
                {
                    if (errno != EINTR && errno != EAGAIN)
                        error_occured = TRUE;
                    perror ("poll");
                }
                else
                {
                    bytes_read = read (self->pipe, &c, 1);

                    if (bytes_read == 1)
                    {
                        g_byte_array_append (self->buffer, (guint8*) &c, 1);

                        if (!c)
                        {
                            got_zero = TRUE;
                            again = FALSE;
                        }
                    }
                    else if (bytes_read == -1)
                    {
                        perror ("read");
                        if (errno != EINTR && errno != EAGAIN)
                            error_occured = TRUE;
                    }
                    else
                    {
                        again = FALSE;
                    }
                }
                break;

            default:
                g_assert_not_reached ();
        }
    }

    if (error_occured || err)
    {
        g_critical ("%s: error", G_STRLOC);

        if (err)
        {
            g_critical ("%s: %s", G_STRLOC, err->message);
            g_error_free (err);
        }

        shutdown_ipc ();
        return FALSE;
    }

    if (got_zero)
        commit (self);

    return TRUE;
}


static void
start_ipc (void)
{
    g_return_if_fail (input == NULL);

    input = g_new0 (Input, 1);

    input->pipe = -1;
    input->pipe_name = NULL;
    input->io = NULL;
    input->io_watch = 0;
    input->ready = FALSE;
    input->buffer = g_byte_array_new ();

    input->pipe_name = g_strdup_printf ("%s/" PIPE_PREFIX_FULL,
                                        g_get_tmp_dir(),
                                        g_get_user_name (),
                                        getpid ());
    unlink (input->pipe_name);

    if (mkfifo (input->pipe_name, S_IRUSR | S_IWUSR))
    {
        perror ("mkfifo");
        goto error;
    }

    input->pipe = open (input->pipe_name, O_RDWR | O_NONBLOCK);

    if (input->pipe == -1)
    {
        perror ("open");
        goto error;
    }

    input->io = g_io_channel_unix_new (input->pipe);
    g_io_channel_set_encoding (input->io, NULL, NULL);
    input->io_watch = g_io_add_watch (input->io,
                                      G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP,
                                      (GIOFunc) read_input,
                                      input);

    input->ready = TRUE;
    return;

error:
    shutdown_ipc ();
}


void
shutdown_ipc (void)
{
    if (input)
    {
        if (input->io_watch)
        {
            g_source_remove (input->io_watch);
            input->io_watch = 0;
        }

        if (input->io)
        {
            g_io_channel_shutdown (input->io, TRUE, NULL);
            g_io_channel_unref (input->io);
            input->io = NULL;
        }

        if (input->pipe_name)
        {
            input->pipe  = -1;
            unlink (input->pipe_name);
            g_free (input->pipe_name);
            input->pipe_name = NULL;
        }

        g_byte_array_free (input->buffer, TRUE);
        g_free (input);
        input = NULL;
    }
}
