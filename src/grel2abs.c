/* GLIB - Library of useful routines for C programming
 * Copyright (c) 1997 Shigio Yamaguchi. All rights reserved.
 * Copyright (c) 1999 Tama Communications Corporation. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "glib.h"
#include "grel2abs.h"

/* current == "./", parent == "../" */
static char dots[] = {'.', '.', G_DIR_SEPARATOR, '\0'};
static char *parent = dots;
static char *current = dots + 1;

/*
 * g_rel2abs: convert an relative path name into absolute.
 *
 *	i)	path	relative path
 *	i)	base	base directory (must be absolute path)
 *	o)	result	result buffer
 *	i)	size	size of result buffer
 *	r)		!= NULL: absolute path
 *			== NULL: error
 */
gchar *
g_rel2abs (path, base, result, size)
const gchar *path;
const gchar *base;
gchar *result;
const size_t size;
{
	const char *pp, *bp;
	/* endp points the last position which is safe in the result buffer. */
	const char *endp = result + size - 1;
	char *rp;
	int length;

	if (*path == G_DIR_SEPARATOR) {
		if (strlen (path) >= size)
			goto erange;
		strcpy (result, path);
		goto finish;
	}
	else if (*base != G_DIR_SEPARATOR || !size) {
		errno = EINVAL;
		return (NULL);
	}
	else if (size == 1)
		goto erange;
	if (!strcmp (path, ".") || !strcmp (path, current)) {
		if (strlen (base) >= size)
			goto erange;
		strcpy (result, base);
		/* rp points the last char. */
		rp = result + strlen (base) - 1;
		if (*rp == G_DIR_SEPARATOR)
			*rp = 0;
		else
			rp++;
		/* rp point NULL char */
		if (*++path == G_DIR_SEPARATOR) {
			/* Append G_DIR_SEPARATOR to the tail of path name. */
			*rp++ = G_DIR_SEPARATOR;
			if (rp > endp)
				goto erange;
			*rp = 0;
		}
		goto finish;
	}
	bp = base + strlen (base);
	if (*(bp - 1) == G_DIR_SEPARATOR)
		--bp;
	/* up to root. */
	for (pp = path; *pp && *pp == '.';) {
		if (!strncmp (pp, parent, 3)) {
			pp += 3;
			while (bp > base && *--bp != G_DIR_SEPARATOR)
				;
		}
		else if (!strncmp (pp, current, 2)) {
			pp += 2;
		}
		else if (!strncmp (pp, "..\0", 3)) {
			pp += 2;
			while (bp > base && *--bp != G_DIR_SEPARATOR)
				;
		}
		else
			break;
	}
	/* down to leaf. */
	length = bp - base;
	if (length >= size)
		goto erange;
	strncpy (result, base, length);
	rp = result + length;
	if (*pp || *(pp - 1) == G_DIR_SEPARATOR || length == 0)
		*rp++ = G_DIR_SEPARATOR;
	if (rp + strlen (pp) > endp)
		goto erange;
	strcpy (rp, pp);
finish:
	return result;
erange:
	errno = ERANGE;
	return (NULL);
}
