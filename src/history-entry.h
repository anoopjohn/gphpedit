/*
 * gphpedit-history-entry.h
 * This file was adapted from gedit
 *
 * Copyright (C) 2006 - Paolo Borelli
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, 
 * Boston, MA 02111-1307, USA.
 */
 
/*
 * Modified by the gphpedit Team, 2010.
 */

#ifndef __GPHPEDIT_HISTORY_ENTRY_H__
#define __GPHPEDIT_HISTORY_ENTRY_H__


G_BEGIN_DECLS

#define GPHPEDIT_TYPE_HISTORY_ENTRY             (gphpedit_history_entry_get_type ())
#define GPHPEDIT_HISTORY_ENTRY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GPHPEDIT_TYPE_HISTORY_ENTRY, GphpeditHistoryEntry))
#define GPHPEDIT_HISTORY_ENTRY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GPHPEDIT_TYPE_HISTORY_ENTRY, GphpeditHistoryEntryClass))
#define GPHPEDIT_IS_HISTORY_ENTRY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GPHPEDIT_TYPE_HISTORY_ENTRY))
#define GPHPEDIT_IS_HISTORY_ENTRY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GPHPEDIT_TYPE_HISTORY_ENTRY))
#define GPHPEDIT_HISTORY_ENTRY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GPHPEDIT_TYPE_HISTORY_ENTRY, GphpeditHistoryEntryClass))


typedef struct _GphpeditHistoryEntry        GphpeditHistoryEntry;
typedef struct _GphpeditHistoryEntryClass   GphpeditHistoryEntryClass;
typedef struct _GphpeditHistoryEntryPrivate GphpeditHistoryEntryPrivate;

struct _GphpeditHistoryEntryClass
{
	GtkComboBoxEntryClass parent_class;
};

struct _GphpeditHistoryEntry
{
	GtkComboBoxEntry parent_instance;

	GphpeditHistoryEntryPrivate *priv;
};

GType		 gphpedit_history_entry_get_type	(void) G_GNUC_CONST;

GtkWidget	*gphpedit_history_entry_new		(const gchar       *history_id,
							 gboolean           enable_completion);

void		 gphpedit_history_entry_prepend_text	(GphpeditHistoryEntry *entry,
							 const gchar       *text);

void		 gphpedit_history_entry_append_text	(GphpeditHistoryEntry *entry,
							 const gchar       *text);

void		 gphpedit_history_entry_clear		(GphpeditHistoryEntry *entry);

void		 gphpedit_history_entry_set_history_length	(GphpeditHistoryEntry *entry,
							 guint              max_saved);

guint		 gphpedit_history_entry_get_history_length	(GphpeditHistoryEntry *gentry);

gchar		*gphpedit_history_entry_get_history_id	(GphpeditHistoryEntry *entry);

void             gphpedit_history_entry_set_enable_completion 
							(GphpeditHistoryEntry *entry,
							 gboolean           enable);
							 
gboolean         gphpedit_history_entry_get_enable_completion 
							(GphpeditHistoryEntry *entry);

GtkWidget	*gphpedit_history_entry_get_entry		(GphpeditHistoryEntry *entry);

typedef gchar * (* GphpeditHistoryEntryEscapeFunc) (const gchar *str);
void		gphpedit_history_entry_set_escape_func	(GphpeditHistoryEntry *entry,
							 GphpeditHistoryEntryEscapeFunc escape_func);

G_END_DECLS

#endif /* __GPHPEDIT_HISTORY_ENTRY_H__ */
