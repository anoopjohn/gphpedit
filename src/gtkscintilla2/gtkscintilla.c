/* GtkScintilla2: Wrapper widget for the Scintilla editing component.
 *
 * Copyright (c) 2010  Jose Rostagno <joserostagno@hotmail.com>
 * Copyright (c) 2002  Dennis J Houy <djhouy@paw.co.za>
 * Copyright (c) 2001  Michele Campeotto <micampe@micampe.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "gtkscintilla.h"
#include "marshal.h"

#include "scintilla/include/ScintillaWidget.h"
#define GPOINTER_TO_LONG(p)  ((glong) (p))
/* Handled Signals */
enum {
    STYLE_NEEDED,
    CHAR_ADDED,
    SAVE_POINT_REACHED,
    SAVE_POINT_LEFT,
    MODIFY_ATTEMPT_RO,
    KEY,
    DOUBLE_CLICK,
    UPDATE_UI,
    MODIFIED,
    MACRO_RECORD,
    MARGIN_CLICK,
    NEED_SHOWN,
    PAINTED,
    USER_LIST_SELECTION,
    URI_DROPPED,
    DWELL_START,
    DWELL_END,
    ZOOM,
    LAST_SIGNAL
};
static void notify_cb                (GtkWidget         *w,
                                      gint               param,
                                      gpointer           notif,
                                      gpointer           data);
static void pass_throug_key          (GtkScintilla      *sci,
                                      gint               ch,
                                      gint               modifiers);

static guint signals[LAST_SIGNAL] = { 0 };

/* http://library.gnome.org/devel/gobject/unstable/gobject-Type-Information.html#G-DEFINE-TYPE:CAPS */
G_DEFINE_TYPE(GtkScintilla, gtk_scintilla, GTK_TYPE_FRAME);  

/*
 *  gtk_scintilla_class_init
 *  initializes the new class and connects the signals
 */
static void
gtk_scintilla_class_init (GtkScintillaClass *klass)
{

  GObjectClass *object_class = G_OBJECT_CLASS (klass);	

    signals[STYLE_NEEDED] =
        g_signal_new ("style_needed",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, style_needed),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[CHAR_ADDED] =
        g_signal_new ("char_added",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, char_added),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[SAVE_POINT_REACHED] =
        g_signal_new ("save_point_reached",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, save_point_reached),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[SAVE_POINT_LEFT] =
        g_signal_new ("save_point_left",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, save_point_left),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[MODIFY_ATTEMPT_RO] =
        g_signal_new ("modify_attempt_ro",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, modify_attempt_ro),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[KEY] =
        g_signal_new ("key",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, key),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_INT);

    signals[DOUBLE_CLICK] =
        g_signal_new ("double_click",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, double_click),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[UPDATE_UI] =
        g_signal_new ("update_ui",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, update_ui),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[MODIFIED] =
        g_signal_new ("modified",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, modified),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT_STRING_INT_INT_INT_INT_INT,
                      G_TYPE_NONE, 8,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
                      G_TYPE_INT, G_TYPE_INT);

    signals[MACRO_RECORD] =
        g_signal_new ("macro_record",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, macro_record),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_ULONG_LONG,
                      G_TYPE_NONE, 3,
                      G_TYPE_INT, G_TYPE_ULONG, G_TYPE_LONG);

    signals[MARGIN_CLICK] =
        g_signal_new ("margin_click",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, margin_click),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT_INT,
                      G_TYPE_NONE, 3,
                      G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

    signals[NEED_SHOWN] =
        g_signal_new ("need_shown",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, need_shown),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_INT,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_INT);

    signals[PAINTED] =
        g_signal_new ("painted",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, painted),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);

    signals[USER_LIST_SELECTION] =
        g_signal_new ("user_list_selection",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, user_list_selection),
                      NULL, NULL,
                      scintilla_marshal_VOID__INT_STRING,
                      G_TYPE_NONE, 2,
                      G_TYPE_INT, G_TYPE_STRING);

    signals[URI_DROPPED] =
        g_signal_new ("uri_dropped",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, uri_dropped),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__STRING,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    signals[DWELL_START] =
        g_signal_new ("dwell_start",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, dwell_start),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[DWELL_END] =
        g_signal_new ("dwell_end",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, dwell_end),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__INT,
                      G_TYPE_NONE, 1,
                      G_TYPE_INT);

    signals[ZOOM] =
        g_signal_new ("zoom",
                      G_OBJECT_CLASS_TYPE (object_class),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GtkScintillaClass, zoom),
                      NULL, NULL,
                      g_cclosure_marshal_VOID__VOID,
                      G_TYPE_NONE, 0);
}
/*
* gtk_scintilla_init
* Initializes a new object instance, connects the scintilla notification signal 
* and makes visible the scintilla component
*/
static void
gtk_scintilla_init (GtkScintilla *sci)
{
    sci->scintilla = GTK_WIDGET (scintilla_new ());
    g_signal_connect (G_OBJECT (sci->scintilla), "sci-notify",
                      G_CALLBACK (notify_cb), GTK_WIDGET (sci));
    
    gtk_container_add (GTK_CONTAINER (sci), sci->scintilla);
    
    gtk_widget_show_all (GTK_WIDGET(sci));
}

/*
* gtk_scintilla_new
* Creates a new instance of GtkScintilla object and returns a GtkWidget
*/
GtkWidget *gtk_scintilla_new (void)
{
    return gtk_widget_new (GTK_TYPE_SCINTILLA, NULL);
}

glong
gtk_scintilla_find_text (GtkScintilla *sci, gint flags, gchar *text,
                         glong chrg_min, glong chrg_max,
                         glong *text_min, glong *text_max)
{
    glong find_pos;
    struct TextToFind ft = {{0, 0}, 0, {0, 0}};
    
    ft.chrg.cpMin = chrg_min;
    ft.chrg.cpMax = chrg_max;
    ft.chrgText.cpMin = *text_min;
    ft.chrgText.cpMax = *text_max;
    ft.lpstrText = text;
    
    find_pos = scintilla_send_message (SCINTILLA (sci->scintilla),
                                       SCI_FINDTEXT,
                                       (long) flags,
                                       GPOINTER_TO_LONG (&ft));
    
    *text_min = ft.chrgText.cpMin;
    *text_max = ft.chrgText.cpMax;
    
    return find_pos;
}
/**
 * gtk_scintilla_get_text_range:
 * @sci: A #GtkScintilla
 * @start: Start position
 * @end: End position
 * @length: text length
 *
 * return a text range from scintilla.
 * the return value must be freed with g_free when no longer needed.
 */
gchar * gtk_scintilla_get_text_range (GtkScintilla *sci,
                              gint start, gint end, gint *length)
{
    gchar *buffer = NULL;
    struct TextRange tr = {{0, 0}, 0};

    buffer = g_strnfill (end - start + 2, 0);

    tr.chrg.cpMin = (long) start;
    tr.chrg.cpMax = (long) end;
    tr.lpstrText = (char *) buffer;

    *length = scintilla_send_message (SCINTILLA (sci->scintilla),
                                      SCI_GETTEXTRANGE, 0,
                                      GPOINTER_TO_LONG (&tr));

    return buffer;
}

/**
 * gtk_scintilla_get_full_text:
 * @sci: A #GtkScintilla
 *
 * return the full text from scintilla.
 * the return value must be freed with g_free when no longer needed.
 */
gchar * gtk_scintilla_get_full_text (GtkScintilla *sci)
{

    gchar *buffer = NULL;
    gsize length;

    length = scintilla_send_message(SCINTILLA(sci->scintilla),
                                      2006, 0, 0);

    length +=2;
    buffer = g_malloc0(length); /* Include terminating null */
    if (!buffer) return NULL;

    scintilla_send_message(SCINTILLA(sci->scintilla),
                                      2182, (gulong) length,
                                      (gulong) buffer);
    return buffer;
}

gchar * gtk_scintilla_get_current_selected_text(GtkScintilla *sci)
{
  if (!GTK_IS_SCINTILLA(sci)) return NULL;

  gchar *buffer = NULL;
  gsize length;

  length = scintilla_send_message(SCINTILLA(sci->scintilla),
                                      2161, 0, (gulong) NULL);

  if (length)
  {

    buffer = g_malloc0(length);
    if (!buffer) return NULL;
    scintilla_send_message(SCINTILLA(sci->scintilla),
                                      2161, 0, (gulong) buffer);
  }

  return buffer;

}

/* --- Start of autogenerated code --- */

void gtk_scintilla_add_text(GtkScintilla *sci, gint length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2001, (gulong) length, (gulong) text);
}

void gtk_scintilla_insert_text(GtkScintilla *sci, glong pos, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2003, (gulong) pos, (gulong) text);
}

void gtk_scintilla_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2004, 0, 0);
}

void gtk_scintilla_clear_document_style(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2005, 0, 0);
}

gint gtk_scintilla_get_length(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2006, 0, 0);
}

gint gtk_scintilla_get_char_at(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2007, (gulong) pos, 0);
}

glong gtk_scintilla_get_current_pos(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2008, 0, 0);
}

glong gtk_scintilla_get_anchor(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2009, 0, 0);
}

gint gtk_scintilla_get_style_at(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2010, (gulong) pos, 0);
}

void gtk_scintilla_redo(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2011, 0, 0);
}

void gtk_scintilla_set_undo_collection(GtkScintilla *sci, gboolean collect_undo)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2012, (gulong) collect_undo, 0);
}

void gtk_scintilla_select_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2013, 0, 0);
}

void gtk_scintilla_set_save_point(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2014, 0, 0);
}

gboolean gtk_scintilla_can_redo(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2016, 0, 0);
}

gint gtk_scintilla_marker_line_from_handle(GtkScintilla *sci, gint handle)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2017, (gulong) handle, 0);
}

void gtk_scintilla_marker_delete_handle(GtkScintilla *sci, gint handle)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2018, (gulong) handle, 0);
}

gboolean gtk_scintilla_get_undo_collection(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2019, 0, 0);
}

gint gtk_scintilla_get_view_ws(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2020, 0, 0);
}

void gtk_scintilla_set_view_ws(GtkScintilla *sci, gint view_ws)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2021, (gulong) view_ws, 0);
}

glong gtk_scintilla_position_from_point(GtkScintilla *sci, gint x, gint y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2022, (gulong) x, (gulong) y);
}

glong gtk_scintilla_position_from_point_close(GtkScintilla *sci, gint x, gint y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2023, (gulong) x, (gulong) y);
}

void gtk_scintilla_goto_line(GtkScintilla *sci, gint line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2024, (gulong) line, 0);
}

void gtk_scintilla_goto_pos(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2025, (gulong) pos, 0);
}

void gtk_scintilla_set_anchor(GtkScintilla *sci, glong pos_anchor)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2026, (gulong) pos_anchor, 0);
}

gint gtk_scintilla_get_cur_line(GtkScintilla *sci, gint length, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2027, (gulong) length, (gulong) text);
}

glong gtk_scintilla_get_end_styled(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2028, 0, 0);
}

void gtk_scintilla_convert_eols(GtkScintilla *sci, gint eol_mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2029, (gulong) eol_mode, 0);
}

gint gtk_scintilla_get_eol_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2030, 0, 0);
}

void gtk_scintilla_set_eol_mode(GtkScintilla *sci, gint eol_mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2031, (gulong) eol_mode, 0);
}

void gtk_scintilla_start_styling(GtkScintilla *sci, glong pos, gint mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2032, (gulong) pos, (gulong) mask);
}

void gtk_scintilla_set_styling(GtkScintilla *sci, gint length, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2033, (gulong) length, (gulong) style);
}

gboolean gtk_scintilla_get_buffered_draw(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2034, 0, 0);
}

void gtk_scintilla_set_buffered_draw(GtkScintilla *sci, gboolean buffered)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2035, (gulong) buffered, 0);
}

void gtk_scintilla_set_tab_width(GtkScintilla *sci, gint tab_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2036, (gulong) tab_width, 0);
}

gint gtk_scintilla_get_tab_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2121, 0, 0);
}

void gtk_scintilla_set_code_page(GtkScintilla *sci, gint code_page)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2037, (gulong) code_page, 0);
}

void gtk_scintilla_set_use_palette(GtkScintilla *sci, gboolean use_palette)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2039, (gulong) use_palette, 0);
}

void gtk_scintilla_marker_define(GtkScintilla *sci, gint marker_number, gint marker_symbol)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2040, (gulong) marker_number, (gulong) marker_symbol);
}

void gtk_scintilla_marker_set_fore(GtkScintilla *sci, gint marker_number, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2041, (gulong) marker_number, (gulong) fore);
}

void gtk_scintilla_marker_set_back(GtkScintilla *sci, gint marker_number, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2042, (gulong) marker_number, (gulong) back);
}

void gtk_scintilla_marker_set_back_selected(GtkScintilla *sci, gint marker_number, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2292, (gulong) marker_number, (gulong) back);
}

void gtk_scintilla_marker_enable_highlight(GtkScintilla *sci, gboolean enabled)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2293, (gulong) enabled, 0);
}

gint gtk_scintilla_marker_add(GtkScintilla *sci, gint line, gint marker_number)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2043, (gulong) line, (gulong) marker_number);
}

void gtk_scintilla_marker_delete(GtkScintilla *sci, gint line, gint marker_number)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2044, (gulong) line, (gulong) marker_number);
}

void gtk_scintilla_marker_delete_all(GtkScintilla *sci, gint marker_number)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2045, (gulong) marker_number, 0);
}

gint gtk_scintilla_marker_get(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2046, (gulong) line, 0);
}

gint gtk_scintilla_marker_next(GtkScintilla *sci, gint line_start, gint marker_mask)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2047, (gulong) line_start, (gulong) marker_mask);
}

gint gtk_scintilla_marker_previous(GtkScintilla *sci, gint line_start, gint marker_mask)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2048, (gulong) line_start, (gulong) marker_mask);
}

void gtk_scintilla_marker_define_pixmap(GtkScintilla *sci, gint marker_number, const gchar * pixmap)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2049, (gulong) marker_number, (gulong) pixmap);
}

void gtk_scintilla_marker_add_set(GtkScintilla *sci, gint line, gint set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2466, (gulong) line, (gulong) set);
}

void gtk_scintilla_marker_set_alpha(GtkScintilla *sci, gint marker_number, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2476, (gulong) marker_number, (gulong) alpha);
}

void gtk_scintilla_set_margin_type_n(GtkScintilla *sci, gint margin, gint margin_type)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2240, (gulong) margin, (gulong) margin_type);
}

gint gtk_scintilla_get_margin_type_n(GtkScintilla *sci, gint margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2241, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_width_n(GtkScintilla *sci, gint margin, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2242, (gulong) margin, (gulong) pixel_width);
}

gint gtk_scintilla_get_margin_width_n(GtkScintilla *sci, gint margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2243, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_mask_n(GtkScintilla *sci, gint margin, gint mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2244, (gulong) margin, (gulong) mask);
}

gint gtk_scintilla_get_margin_mask_n(GtkScintilla *sci, gint margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2245, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_sensitive_n(GtkScintilla *sci, gint margin, gboolean sensitive)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2246, (gulong) margin, (gulong) sensitive);
}

gboolean gtk_scintilla_get_margin_sensitive_n(GtkScintilla *sci, gint margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2247, (gulong) margin, 0);
}

void gtk_scintilla_set_margin_cursor_n(GtkScintilla *sci, gint margin, gint cursor)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2248, (gulong) margin, (gulong) cursor);
}

gint gtk_scintilla_get_margin_cursor_n(GtkScintilla *sci, gint margin)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2249, (gulong) margin, 0);
}

void gtk_scintilla_style_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2050, 0, 0);
}

void gtk_scintilla_style_set_fore(GtkScintilla *sci, gint style, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2051, (gulong) style, (gulong) fore);
}

void gtk_scintilla_style_set_back(GtkScintilla *sci, gint style, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2052, (gulong) style, (gulong) back);
}

void gtk_scintilla_style_set_bold(GtkScintilla *sci, gint style, gboolean bold)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2053, (gulong) style, (gulong) bold);
}

void gtk_scintilla_style_set_italic(GtkScintilla *sci, gint style, gboolean italic)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2054, (gulong) style, (gulong) italic);
}

void gtk_scintilla_style_set_size(GtkScintilla *sci, gint style, gint size_points)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2055, (gulong) style, (gulong) size_points);
}

void gtk_scintilla_style_set_font(GtkScintilla *sci, gint style, const gchar * font_name)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2056, (gulong) style, (gulong) font_name);
}

void gtk_scintilla_style_set_eol_filled(GtkScintilla *sci, gint style, gboolean filled)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2057, (gulong) style, (gulong) filled);
}

void gtk_scintilla_style_reset_default(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2058, 0, 0);
}

void gtk_scintilla_style_set_underline(GtkScintilla *sci, gint style, gboolean underline)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2059, (gulong) style, (gulong) underline);
}

glong gtk_scintilla_style_get_fore(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2481, (gulong) style, 0);
}

glong gtk_scintilla_style_get_back(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2482, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_bold(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2483, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_italic(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2484, (gulong) style, 0);
}

gint gtk_scintilla_style_get_size(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2485, (gulong) style, 0);
}

gint gtk_scintilla_style_get_font(GtkScintilla *sci, gint style, gchar * font_name)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2486, (gulong) style, (gulong) font_name);
}

gboolean gtk_scintilla_style_get_eol_filled(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2487, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_underline(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2488, (gulong) style, 0);
}

gint gtk_scintilla_style_get_case(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2489, (gulong) style, 0);
}

gint gtk_scintilla_style_get_character_set(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2490, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_visible(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2491, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_changeable(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2492, (gulong) style, 0);
}

gboolean gtk_scintilla_style_get_hot_spot(GtkScintilla *sci, gint style)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2493, (gulong) style, 0);
}

void gtk_scintilla_style_set_case(GtkScintilla *sci, gint style, gint case_force)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2060, (gulong) style, (gulong) case_force);
}

void gtk_scintilla_style_set_character_set(GtkScintilla *sci, gint style, gint character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2066, (gulong) style, (gulong) character_set);
}

void gtk_scintilla_style_set_hot_spot(GtkScintilla *sci, gint style, gboolean hotspot)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2409, (gulong) style, (gulong) hotspot);
}

void gtk_scintilla_set_sel_fore(GtkScintilla *sci, gboolean use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2067, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_set_sel_back(GtkScintilla *sci, gboolean use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2068, (gulong) use_setting, (gulong) back);
}

gint gtk_scintilla_get_sel_alpha(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2477, 0, 0);
}

void gtk_scintilla_set_sel_alpha(GtkScintilla *sci, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2478, (gulong) alpha, 0);
}

gboolean gtk_scintilla_get_sel_eol_filled(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2479, 0, 0);
}

void gtk_scintilla_set_sel_eol_filled(GtkScintilla *sci, gboolean filled)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2480, (gulong) filled, 0);
}

void gtk_scintilla_set_caret_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2069, (gulong) fore, 0);
}

void gtk_scintilla_clear_all_cmd_keys(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2072, 0, 0);
}

void gtk_scintilla_set_styling_ex(GtkScintilla *sci, gint length, const gchar * styles)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2073, (gulong) length, (gulong) styles);
}

void gtk_scintilla_style_set_visible(GtkScintilla *sci, gint style, gboolean visible)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2074, (gulong) style, (gulong) visible);
}

gint gtk_scintilla_get_caret_period(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2075, 0, 0);
}

void gtk_scintilla_set_caret_period(GtkScintilla *sci, gint period_milliseconds)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2076, (gulong) period_milliseconds, 0);
}

void gtk_scintilla_set_word_chars(GtkScintilla *sci, const gchar * characters)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2077, 0, (gulong) characters);
}

void gtk_scintilla_begin_undo_action(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2078, 0, 0);
}

void gtk_scintilla_end_undo_action(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2079, 0, 0);
}

void gtk_scintilla_indic_set_style(GtkScintilla *sci, gint indic, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2080, (gulong) indic, (gulong) style);
}

gint gtk_scintilla_indic_get_style(GtkScintilla *sci, gint indic)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2081, (gulong) indic, 0);
}

void gtk_scintilla_indic_set_fore(GtkScintilla *sci, gint indic, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2082, (gulong) indic, (gulong) fore);
}

glong gtk_scintilla_indic_get_fore(GtkScintilla *sci, gint indic)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2083, (gulong) indic, 0);
}

void gtk_scintilla_indic_set_under(GtkScintilla *sci, gint indic, gboolean under)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2510, (gulong) indic, (gulong) under);
}

gboolean gtk_scintilla_indic_get_under(GtkScintilla *sci, gint indic)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2511, (gulong) indic, 0);
}

void gtk_scintilla_set_whitespace_fore(GtkScintilla *sci, gboolean use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2084, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_set_whitespace_back(GtkScintilla *sci, gboolean use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2085, (gulong) use_setting, (gulong) back);
}

void gtk_scintilla_set_whitespace_size(GtkScintilla *sci, gint size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2086, (gulong) size, 0);
}

gint gtk_scintilla_get_whitespace_size(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2087, 0, 0);
}

void gtk_scintilla_set_style_bits(GtkScintilla *sci, gint bits)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2090, (gulong) bits, 0);
}

gint gtk_scintilla_get_style_bits(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2091, 0, 0);
}

void gtk_scintilla_set_line_state(GtkScintilla *sci, gint line, gint state)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2092, (gulong) line, (gulong) state);
}

gint gtk_scintilla_get_line_state(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2093, (gulong) line, 0);
}

gint gtk_scintilla_get_max_line_state(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2094, 0, 0);
}

gboolean gtk_scintilla_get_caret_line_visible(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2095, 0, 0);
}

void gtk_scintilla_set_caret_line_visible(GtkScintilla *sci, gboolean show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2096, (gulong) show, 0);
}

glong gtk_scintilla_get_caret_line_back(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2097, 0, 0);
}

void gtk_scintilla_set_caret_line_back(GtkScintilla *sci, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2098, (gulong) back, 0);
}

void gtk_scintilla_style_set_changeable(GtkScintilla *sci, gint style, gboolean changeable)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2099, (gulong) style, (gulong) changeable);
}

void gtk_scintilla_autoc_show(GtkScintilla *sci, gint len_entered, const gchar * item_list)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2100, (gulong) len_entered, (gulong) item_list);
}

void gtk_scintilla_autoc_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2101, 0, 0);
}

gboolean gtk_scintilla_autoc_active(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2102, 0, 0);
}

glong gtk_scintilla_autoc_pos_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2103, 0, 0);
}

void gtk_scintilla_autoc_complete(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2104, 0, 0);
}

void gtk_scintilla_autoc_stops(GtkScintilla *sci, const gchar * character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2105, 0, (gulong) character_set);
}

void gtk_scintilla_autoc_set_separator(GtkScintilla *sci, gint separator_character)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2106, (gulong) separator_character, 0);
}

gint gtk_scintilla_autoc_get_separator(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2107, 0, 0);
}

void gtk_scintilla_autoc_select(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2108, 0, (gulong) text);
}

void gtk_scintilla_autoc_set_cancel_at_start(GtkScintilla *sci, gboolean cancel)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2110, (gulong) cancel, 0);
}

gboolean gtk_scintilla_autoc_get_cancel_at_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2111, 0, 0);
}

void gtk_scintilla_autoc_set_fill_ups(GtkScintilla *sci, const gchar * character_set)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2112, 0, (gulong) character_set);
}

void gtk_scintilla_autoc_set_choose_single(GtkScintilla *sci, gboolean choose_single)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2113, (gulong) choose_single, 0);
}

gboolean gtk_scintilla_autoc_get_choose_single(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2114, 0, 0);
}

void gtk_scintilla_autoc_set_ignore_case(GtkScintilla *sci, gboolean ignore_case)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2115, (gulong) ignore_case, 0);
}

gboolean gtk_scintilla_autoc_get_ignore_case(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2116, 0, 0);
}

void gtk_scintilla_user_list_show(GtkScintilla *sci, gint list_type, const gchar * item_list)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2117, (gulong) list_type, (gulong) item_list);
}

void gtk_scintilla_autoc_set_auto_hide(GtkScintilla *sci, gboolean auto_hide)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2118, (gulong) auto_hide, 0);
}

gboolean gtk_scintilla_autoc_get_auto_hide(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2119, 0, 0);
}

void gtk_scintilla_autoc_set_drop_rest_of_word(GtkScintilla *sci, gboolean drop_rest_of_word)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2270, (gulong) drop_rest_of_word, 0);
}

gboolean gtk_scintilla_autoc_get_drop_rest_of_word(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2271, 0, 0);
}

void gtk_scintilla_register_image(GtkScintilla *sci, gint type, const gchar * xpm_data)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2405, (gulong) type, (gulong) xpm_data);
}

void gtk_scintilla_clear_registered_images(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2408, 0, 0);
}

gint gtk_scintilla_autoc_get_type_separator(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2285, 0, 0);
}

void gtk_scintilla_autoc_set_type_separator(GtkScintilla *sci, gint separator_character)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2286, (gulong) separator_character, 0);
}

void gtk_scintilla_autoc_set_max_width(GtkScintilla *sci, gint character_count)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2208, (gulong) character_count, 0);
}

gint gtk_scintilla_autoc_get_max_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2209, 0, 0);
}

void gtk_scintilla_autoc_set_max_height(GtkScintilla *sci, gint row_count)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2210, (gulong) row_count, 0);
}

gint gtk_scintilla_autoc_get_max_height(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2211, 0, 0);
}

void gtk_scintilla_set_indent(GtkScintilla *sci, gint indent_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2122, (gulong) indent_size, 0);
}

gint gtk_scintilla_get_indent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2123, 0, 0);
}

void gtk_scintilla_set_use_tabs(GtkScintilla *sci, gboolean use_tabs)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2124, (gulong) use_tabs, 0);
}

gboolean gtk_scintilla_get_use_tabs(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2125, 0, 0);
}

void gtk_scintilla_set_line_indentation(GtkScintilla *sci, gint line, gint indent_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2126, (gulong) line, (gulong) indent_size);
}

gint gtk_scintilla_get_line_indentation(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2127, (gulong) line, 0);
}

glong gtk_scintilla_get_line_indent_position(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2128, (gulong) line, 0);
}

gint gtk_scintilla_get_column(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2129, (gulong) pos, 0);
}

void gtk_scintilla_set_h_scroll_bar(GtkScintilla *sci, gboolean show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2130, (gulong) show, 0);
}

gboolean gtk_scintilla_get_h_scroll_bar(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2131, 0, 0);
}

void gtk_scintilla_set_indentation_guides(GtkScintilla *sci, gint indent_view)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2132, (gulong) indent_view, 0);
}

gint gtk_scintilla_get_indentation_guides(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2133, 0, 0);
}

void gtk_scintilla_set_highlight_guide(GtkScintilla *sci, gint column)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2134, (gulong) column, 0);
}

gint gtk_scintilla_get_highlight_guide(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2135, 0, 0);
}

gint gtk_scintilla_get_line_end_position(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2136, (gulong) line, 0);
}

gint gtk_scintilla_get_code_page(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2137, 0, 0);
}

glong gtk_scintilla_get_caret_fore(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2138, 0, 0);
}

gboolean gtk_scintilla_get_use_palette(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2139, 0, 0);
}

gboolean gtk_scintilla_get_read_only(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2140, 0, 0);
}

void gtk_scintilla_set_current_pos(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2141, (gulong) pos, 0);
}

void gtk_scintilla_set_selection_start(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2142, (gulong) pos, 0);
}

glong gtk_scintilla_get_selection_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2143, 0, 0);
}

void gtk_scintilla_set_selection_end(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2144, (gulong) pos, 0);
}

glong gtk_scintilla_get_selection_end(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2145, 0, 0);
}

void gtk_scintilla_set_print_magnification(GtkScintilla *sci, gint magnification)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2146, (gulong) magnification, 0);
}

gint gtk_scintilla_get_print_magnification(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2147, 0, 0);
}

void gtk_scintilla_set_print_colour_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2148, (gulong) mode, 0);
}

gint gtk_scintilla_get_print_colour_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2149, 0, 0);
}

gint gtk_scintilla_get_first_visible_line(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2152, 0, 0);
}

gint gtk_scintilla_get_line(GtkScintilla *sci, gint line, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2153, (gulong) line, (gulong) text);
}

gint gtk_scintilla_get_line_count(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2154, 0, 0);
}

void gtk_scintilla_set_margin_left(GtkScintilla *sci, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2155, 0, (gulong) pixel_width);
}

gint gtk_scintilla_get_margin_left(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2156, 0, 0);
}

void gtk_scintilla_set_margin_right(GtkScintilla *sci, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2157, 0, (gulong) pixel_width);
}

gint gtk_scintilla_get_margin_right(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2158, 0, 0);
}

gboolean gtk_scintilla_get_modify(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2159, 0, 0);
}

void gtk_scintilla_set_sel(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2160, (gulong) start, (gulong) end);
}

gint gtk_scintilla_get_sel_text(GtkScintilla *sci, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2161, 0, (gulong) text);
}

void gtk_scintilla_hide_selection(GtkScintilla *sci, gboolean normal)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2163, (gulong) normal, 0);
}

gint gtk_scintilla_point_x_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2164, 0, (gulong) pos);
}

gint gtk_scintilla_point_y_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2165, 0, (gulong) pos);
}

gint gtk_scintilla_line_from_position(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2166, (gulong) pos, 0);
}

glong gtk_scintilla_position_from_line(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2167, (gulong) line, 0);
}

void gtk_scintilla_line_scroll(GtkScintilla *sci, gint columns, gint lines)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2168, (gulong) columns, (gulong) lines);
}

void gtk_scintilla_scroll_caret(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2169, 0, 0);
}

void gtk_scintilla_replace_sel(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2170, 0, (gulong) text);
}

void gtk_scintilla_set_read_only(GtkScintilla *sci, gboolean read_only)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2171, (gulong) read_only, 0);
}

void gtk_scintilla_null(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2172, 0, 0);
}

gboolean gtk_scintilla_can_paste(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2173, 0, 0);
}

gboolean gtk_scintilla_can_undo(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2174, 0, 0);
}

void gtk_scintilla_empty_undo_buffer(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2175, 0, 0);
}

void gtk_scintilla_undo(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2176, 0, 0);
}

void gtk_scintilla_cut(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2177, 0, 0);
}

void gtk_scintilla_copy(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2178, 0, 0);
}

void gtk_scintilla_paste(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2179, 0, 0);
}

void gtk_scintilla_clear(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2180, 0, 0);
}

void gtk_scintilla_set_text(GtkScintilla *sci, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2181, 0, (gulong) text);
}

gint gtk_scintilla_get_text(GtkScintilla *sci, gint length, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2182, (gulong) length, (gulong) text);
}

gint gtk_scintilla_get_text_length(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2183, 0, 0);
}

gint gtk_scintilla_get_direct_function(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2184, 0, 0);
}

gint gtk_scintilla_get_direct_pointer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2185, 0, 0);
}

void gtk_scintilla_set_overtype(GtkScintilla *sci, gboolean overtype)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2186, (gulong) overtype, 0);
}

gboolean gtk_scintilla_get_overtype(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2187, 0, 0);
}

void gtk_scintilla_set_caret_width(GtkScintilla *sci, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2188, (gulong) pixel_width, 0);
}

gint gtk_scintilla_get_caret_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2189, 0, 0);
}

void gtk_scintilla_set_target_start(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2190, (gulong) pos, 0);
}

glong gtk_scintilla_get_target_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2191, 0, 0);
}

void gtk_scintilla_set_target_end(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2192, (gulong) pos, 0);
}

glong gtk_scintilla_get_target_end(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2193, 0, 0);
}

gint gtk_scintilla_replace_target(GtkScintilla *sci, gint length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2194, (gulong) length, (gulong) text);
}

gint gtk_scintilla_replace_target_re(GtkScintilla *sci, gint length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2195, (gulong) length, (gulong) text);
}

gint gtk_scintilla_search_in_target(GtkScintilla *sci, gint length, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2197, (gulong) length, (gulong) text);
}

void gtk_scintilla_set_search_flags(GtkScintilla *sci, gint flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2198, (gulong) flags, 0);
}

gint gtk_scintilla_get_search_flags(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2199, 0, 0);
}

void gtk_scintilla_call_tip_show(GtkScintilla *sci, glong pos, const gchar * definition)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2200, (gulong) pos, (gulong) definition);
}

void gtk_scintilla_call_tip_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2201, 0, 0);
}

gboolean gtk_scintilla_call_tip_active(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2202, 0, 0);
}

glong gtk_scintilla_call_tip_pos_start(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2203, 0, 0);
}

void gtk_scintilla_call_tip_set_hlt(GtkScintilla *sci, gint start, gint end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2204, (gulong) start, (gulong) end);
}

void gtk_scintilla_call_tip_set_back(GtkScintilla *sci, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2205, (gulong) back, 0);
}

void gtk_scintilla_call_tip_set_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2206, (gulong) fore, 0);
}

void gtk_scintilla_call_tip_set_fore_hlt(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2207, (gulong) fore, 0);
}

void gtk_scintilla_call_tip_use_style(GtkScintilla *sci, gint tab_size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2212, (gulong) tab_size, 0);
}

gint gtk_scintilla_visible_from_doc_line(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2220, (gulong) line, 0);
}

gint gtk_scintilla_doc_line_from_visible(GtkScintilla *sci, gint line_display)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2221, (gulong) line_display, 0);
}

gint gtk_scintilla_wrap_count(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2235, (gulong) line, 0);
}

void gtk_scintilla_set_fold_level(GtkScintilla *sci, gint line, gint level)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2222, (gulong) line, (gulong) level);
}

gint gtk_scintilla_get_fold_level(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2223, (gulong) line, 0);
}

gint gtk_scintilla_get_last_child(GtkScintilla *sci, gint line, gint level)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2224, (gulong) line, (gulong) level);
}

gint gtk_scintilla_get_fold_parent(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2225, (gulong) line, 0);
}

void gtk_scintilla_show_lines(GtkScintilla *sci, gint line_start, gint line_end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2226, (gulong) line_start, (gulong) line_end);
}

void gtk_scintilla_hide_lines(GtkScintilla *sci, gint line_start, gint line_end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2227, (gulong) line_start, (gulong) line_end);
}

gboolean gtk_scintilla_get_line_visible(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2228, (gulong) line, 0);
}

void gtk_scintilla_set_fold_expanded(GtkScintilla *sci, gint line, gboolean expanded)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2229, (gulong) line, (gulong) expanded);
}

gboolean gtk_scintilla_get_fold_expanded(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2230, (gulong) line, 0);
}

void gtk_scintilla_toggle_fold(GtkScintilla *sci, gint line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2231, (gulong) line, 0);
}

void gtk_scintilla_ensure_visible(GtkScintilla *sci, gint line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2232, (gulong) line, 0);
}

void gtk_scintilla_set_fold_flags(GtkScintilla *sci, gint flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2233, (gulong) flags, 0);
}

void gtk_scintilla_ensure_visible_enforce_policy(GtkScintilla *sci, gint line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2234, (gulong) line, 0);
}

void gtk_scintilla_set_tab_indents(GtkScintilla *sci, gboolean tab_indents)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2260, (gulong) tab_indents, 0);
}

gboolean gtk_scintilla_get_tab_indents(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2261, 0, 0);
}

void gtk_scintilla_set_backspace_unindents(GtkScintilla *sci, gboolean bs_unindents)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2262, (gulong) bs_unindents, 0);
}

gboolean gtk_scintilla_get_backspace_unindents(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2263, 0, 0);
}

void gtk_scintilla_set_mouse_dwell_time(GtkScintilla *sci, gint period_milliseconds)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2264, (gulong) period_milliseconds, 0);
}

gint gtk_scintilla_get_mouse_dwell_time(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2265, 0, 0);
}

gint gtk_scintilla_word_start_position(GtkScintilla *sci, glong pos, gboolean only_word_characters)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2266, (gulong) pos, (gulong) only_word_characters);
}

gint gtk_scintilla_word_end_position(GtkScintilla *sci, glong pos, gboolean only_word_characters)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2267, (gulong) pos, (gulong) only_word_characters);
}

void gtk_scintilla_set_wrap_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2268, (gulong) mode, 0);
}

gint gtk_scintilla_get_wrap_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2269, 0, 0);
}

void gtk_scintilla_set_wrap_visual_flags(GtkScintilla *sci, gint wrap_visual_flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2460, (gulong) wrap_visual_flags, 0);
}

gint gtk_scintilla_get_wrap_visual_flags(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2461, 0, 0);
}

void gtk_scintilla_set_wrap_visual_flags_location(GtkScintilla *sci, gint wrap_visual_flags_location)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2462, (gulong) wrap_visual_flags_location, 0);
}

gint gtk_scintilla_get_wrap_visual_flags_location(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2463, 0, 0);
}

void gtk_scintilla_set_wrap_start_indent(GtkScintilla *sci, gint indent)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2464, (gulong) indent, 0);
}

gint gtk_scintilla_get_wrap_start_indent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2465, 0, 0);
}

void gtk_scintilla_set_wrap_indent_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2472, (gulong) mode, 0);
}

gint gtk_scintilla_get_wrap_indent_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2473, 0, 0);
}

void gtk_scintilla_set_layout_cache(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2272, (gulong) mode, 0);
}

gint gtk_scintilla_get_layout_cache(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2273, 0, 0);
}

void gtk_scintilla_set_scroll_width(GtkScintilla *sci, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2274, (gulong) pixel_width, 0);
}

gint gtk_scintilla_get_scroll_width(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2275, 0, 0);
}

void gtk_scintilla_set_scroll_width_tracking(GtkScintilla *sci, gboolean tracking)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2516, (gulong) tracking, 0);
}

gboolean gtk_scintilla_get_scroll_width_tracking(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2517, 0, 0);
}

gint gtk_scintilla_text_width(GtkScintilla *sci, gint style, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2276, (gulong) style, (gulong) text);
}

void gtk_scintilla_set_end_at_last_line(GtkScintilla *sci, gboolean end_at_last_line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2277, (gulong) end_at_last_line, 0);
}

gboolean gtk_scintilla_get_end_at_last_line(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2278, 0, 0);
}

gint gtk_scintilla_text_height(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2279, (gulong) line, 0);
}

void gtk_scintilla_set_v_scroll_bar(GtkScintilla *sci, gboolean show)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2280, (gulong) show, 0);
}

gboolean gtk_scintilla_get_v_scroll_bar(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2281, 0, 0);
}

void gtk_scintilla_append_text(GtkScintilla *sci, gint length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2282, (gulong) length, (gulong) text);
}

gboolean gtk_scintilla_get_two_phase_draw(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2283, 0, 0);
}

void gtk_scintilla_set_two_phase_draw(GtkScintilla *sci, gboolean two_phase)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2284, (gulong) two_phase, 0);
}

void gtk_scintilla_set_font_quality(GtkScintilla *sci, gint font_quality)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2611, (gulong) font_quality, 0);
}

gint gtk_scintilla_get_font_quality(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2612, 0, 0);
}

void gtk_scintilla_set_first_visible_line(GtkScintilla *sci, gint line_display)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2613, (gulong) line_display, 0);
}

void gtk_scintilla_set_multi_paste(GtkScintilla *sci, gint multi_paste)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2614, (gulong) multi_paste, 0);
}

gint gtk_scintilla_get_multi_paste(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2615, 0, 0);
}

gint gtk_scintilla_get_tag(GtkScintilla *sci, gint tag_number, gchar * tag_value)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2616, (gulong) tag_number, (gulong) tag_value);
}

void gtk_scintilla_target_from_selection(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2287, 0, 0);
}

void gtk_scintilla_lines_join(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2288, 0, 0);
}

void gtk_scintilla_lines_split(GtkScintilla *sci, gint pixel_width)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2289, (gulong) pixel_width, 0);
}

void gtk_scintilla_set_fold_margin_colour(GtkScintilla *sci, gboolean use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2290, (gulong) use_setting, (gulong) back);
}

void gtk_scintilla_set_fold_margin_hi_colour(GtkScintilla *sci, gboolean use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2291, (gulong) use_setting, (gulong) fore);
}

void gtk_scintilla_line_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2300, 0, 0);
}

void gtk_scintilla_line_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2301, 0, 0);
}

void gtk_scintilla_line_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2302, 0, 0);
}

void gtk_scintilla_line_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2303, 0, 0);
}

void gtk_scintilla_char_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2304, 0, 0);
}

void gtk_scintilla_char_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2305, 0, 0);
}

void gtk_scintilla_char_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2306, 0, 0);
}

void gtk_scintilla_char_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2307, 0, 0);
}

void gtk_scintilla_word_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2308, 0, 0);
}

void gtk_scintilla_word_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2309, 0, 0);
}

void gtk_scintilla_word_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2310, 0, 0);
}

void gtk_scintilla_word_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2311, 0, 0);
}

void gtk_scintilla_home(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2312, 0, 0);
}

void gtk_scintilla_home_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2313, 0, 0);
}

void gtk_scintilla_line_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2314, 0, 0);
}

void gtk_scintilla_line_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2315, 0, 0);
}

void gtk_scintilla_document_start(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2316, 0, 0);
}

void gtk_scintilla_document_start_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2317, 0, 0);
}

void gtk_scintilla_document_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2318, 0, 0);
}

void gtk_scintilla_document_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2319, 0, 0);
}

void gtk_scintilla_page_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2320, 0, 0);
}

void gtk_scintilla_page_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2321, 0, 0);
}

void gtk_scintilla_page_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2322, 0, 0);
}

void gtk_scintilla_page_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2323, 0, 0);
}

void gtk_scintilla_edit_toggle_overtype(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2324, 0, 0);
}

void gtk_scintilla_cancel(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2325, 0, 0);
}

void gtk_scintilla_delete_back(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2326, 0, 0);
}

void gtk_scintilla_tab(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2327, 0, 0);
}

void gtk_scintilla_back_tab(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2328, 0, 0);
}

void gtk_scintilla_new_line(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2329, 0, 0);
}

void gtk_scintilla_form_feed(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2330, 0, 0);
}

void gtk_scintilla_v_c_home(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2331, 0, 0);
}

void gtk_scintilla_v_c_home_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2332, 0, 0);
}

void gtk_scintilla_zoom_in(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2333, 0, 0);
}

void gtk_scintilla_zoom_out(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2334, 0, 0);
}

void gtk_scintilla_del_word_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2335, 0, 0);
}

void gtk_scintilla_del_word_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2336, 0, 0);
}

void gtk_scintilla_del_word_right_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2518, 0, 0);
}

void gtk_scintilla_line_cut(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2337, 0, 0);
}

void gtk_scintilla_line_delete(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2338, 0, 0);
}

void gtk_scintilla_line_transpose(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2339, 0, 0);
}

void gtk_scintilla_line_duplicate(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2404, 0, 0);
}

void gtk_scintilla_lower_case(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2340, 0, 0);
}

void gtk_scintilla_upper_case(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2341, 0, 0);
}

void gtk_scintilla_line_scroll_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2342, 0, 0);
}

void gtk_scintilla_line_scroll_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2343, 0, 0);
}

void gtk_scintilla_delete_back_not_line(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2344, 0, 0);
}

void gtk_scintilla_home_display(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2345, 0, 0);
}

void gtk_scintilla_home_display_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2346, 0, 0);
}

void gtk_scintilla_line_end_display(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2347, 0, 0);
}

void gtk_scintilla_line_end_display_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2348, 0, 0);
}

void gtk_scintilla_home_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2349, 0, 0);
}

void gtk_scintilla_home_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2450, 0, 0);
}

void gtk_scintilla_line_end_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2451, 0, 0);
}

void gtk_scintilla_line_end_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2452, 0, 0);
}

void gtk_scintilla_v_c_home_wrap(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2453, 0, 0);
}

void gtk_scintilla_v_c_home_wrap_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2454, 0, 0);
}

void gtk_scintilla_line_copy(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2455, 0, 0);
}

void gtk_scintilla_move_caret_inside_view(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2401, 0, 0);
}

gint gtk_scintilla_line_length(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2350, (gulong) line, 0);
}

void gtk_scintilla_brace_highlight(GtkScintilla *sci, glong pos1, glong pos2)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2351, (gulong) pos1, (gulong) pos2);
}

void gtk_scintilla_brace_highlight_indicator(GtkScintilla *sci, gboolean use_brace_highlight_indicator, gint indicator)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2498, (gulong) use_brace_highlight_indicator, (gulong) indicator);
}

void gtk_scintilla_brace_bad_light(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2352, (gulong) pos, 0);
}

void gtk_scintilla_brace_bad_light_indicator(GtkScintilla *sci, gboolean use_brace_bad_light_indicator, gint indicator)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2499, (gulong) use_brace_bad_light_indicator, (gulong) indicator);
}

glong gtk_scintilla_brace_match(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2353, (gulong) pos, 0);
}

gboolean gtk_scintilla_get_view_eol(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2355, 0, 0);
}

void gtk_scintilla_set_view_eol(GtkScintilla *sci, gboolean visible)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2356, (gulong) visible, 0);
}

gint gtk_scintilla_get_doc_pointer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2357, 0, 0);
}

void gtk_scintilla_set_doc_pointer(GtkScintilla *sci, gint pointer)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2358, 0, (gulong) pointer);
}

void gtk_scintilla_set_mod_event_mask(GtkScintilla *sci, gint mask)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2359, (gulong) mask, 0);
}

gint gtk_scintilla_get_edge_column(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2360, 0, 0);
}

void gtk_scintilla_set_edge_column(GtkScintilla *sci, gint column)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2361, (gulong) column, 0);
}

gint gtk_scintilla_get_edge_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2362, 0, 0);
}

void gtk_scintilla_set_edge_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2363, (gulong) mode, 0);
}

glong gtk_scintilla_get_edge_colour(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2364, 0, 0);
}

void gtk_scintilla_set_edge_colour(GtkScintilla *sci, glong edge_colour)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2365, (gulong) edge_colour, 0);
}

void gtk_scintilla_search_anchor(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2366, 0, 0);
}

gint gtk_scintilla_search_next(GtkScintilla *sci, gint flags, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2367, (gulong) flags, (gulong) text);
}

gint gtk_scintilla_search_prev(GtkScintilla *sci, gint flags, const gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2368, (gulong) flags, (gulong) text);
}

gint gtk_scintilla_lines_on_screen(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2370, 0, 0);
}

void gtk_scintilla_use_pop_up(GtkScintilla *sci, gboolean allow_pop_up)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2371, (gulong) allow_pop_up, 0);
}

gboolean gtk_scintilla_selection_is_rectangle(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2372, 0, 0);
}

void gtk_scintilla_set_zoom(GtkScintilla *sci, gint zoom)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2373, (gulong) zoom, 0);
}

gint gtk_scintilla_get_zoom(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2374, 0, 0);
}

gint gtk_scintilla_create_document(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2375, 0, 0);
}

void gtk_scintilla_add_ref_document(GtkScintilla *sci, gint doc)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2376, 0, (gulong) doc);
}

void gtk_scintilla_release_document(GtkScintilla *sci, gint doc)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2377, 0, (gulong) doc);
}

gint gtk_scintilla_get_mod_event_mask(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2378, 0, 0);
}

void gtk_scintilla_set_focus(GtkScintilla *sci, gboolean focus)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2380, (gulong) focus, 0);
}

gboolean gtk_scintilla_get_focus(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2381, 0, 0);
}

void gtk_scintilla_set_status(GtkScintilla *sci, gint status_code)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2382, (gulong) status_code, 0);
}

gint gtk_scintilla_get_status(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2383, 0, 0);
}

void gtk_scintilla_set_mouse_down_captures(GtkScintilla *sci, gboolean captures)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2384, (gulong) captures, 0);
}

gboolean gtk_scintilla_get_mouse_down_captures(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2385, 0, 0);
}

void gtk_scintilla_set_cursor(GtkScintilla *sci, gint cursor_type)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2386, (gulong) cursor_type, 0);
}

gint gtk_scintilla_get_cursor(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2387, 0, 0);
}

void gtk_scintilla_set_control_char_symbol(GtkScintilla *sci, gint symbol)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2388, (gulong) symbol, 0);
}

gint gtk_scintilla_get_control_char_symbol(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2389, 0, 0);
}

void gtk_scintilla_word_part_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2390, 0, 0);
}

void gtk_scintilla_word_part_left_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2391, 0, 0);
}

void gtk_scintilla_word_part_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2392, 0, 0);
}

void gtk_scintilla_word_part_right_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2393, 0, 0);
}

void gtk_scintilla_set_visible_policy(GtkScintilla *sci, gint visible_policy, gint visible_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2394, (gulong) visible_policy, (gulong) visible_slop);
}

void gtk_scintilla_del_line_left(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2395, 0, 0);
}

void gtk_scintilla_del_line_right(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2396, 0, 0);
}

void gtk_scintilla_set_x_offset(GtkScintilla *sci, gint new_offset)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2397, (gulong) new_offset, 0);
}

gint gtk_scintilla_get_x_offset(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2398, 0, 0);
}

void gtk_scintilla_choose_caret_x(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2399, 0, 0);
}

void gtk_scintilla_grab_focus(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2400, 0, 0);
}

void gtk_scintilla_set_x_caret_policy(GtkScintilla *sci, gint caret_policy, gint caret_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2402, (gulong) caret_policy, (gulong) caret_slop);
}

void gtk_scintilla_set_y_caret_policy(GtkScintilla *sci, gint caret_policy, gint caret_slop)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2403, (gulong) caret_policy, (gulong) caret_slop);
}

void gtk_scintilla_set_print_wrap_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2406, (gulong) mode, 0);
}

gint gtk_scintilla_get_print_wrap_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2407, 0, 0);
}

void gtk_scintilla_set_hotspot_active_fore(GtkScintilla *sci, gboolean use_setting, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2410, (gulong) use_setting, (gulong) fore);
}

glong gtk_scintilla_get_hotspot_active_fore(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2494, 0, 0);
}

void gtk_scintilla_set_hotspot_active_back(GtkScintilla *sci, gboolean use_setting, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2411, (gulong) use_setting, (gulong) back);
}

glong gtk_scintilla_get_hotspot_active_back(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2495, 0, 0);
}

void gtk_scintilla_set_hotspot_active_underline(GtkScintilla *sci, gboolean underline)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2412, (gulong) underline, 0);
}

gboolean gtk_scintilla_get_hotspot_active_underline(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2496, 0, 0);
}

void gtk_scintilla_set_hotspot_single_line(GtkScintilla *sci, gboolean single_line)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2421, (gulong) single_line, 0);
}

gboolean gtk_scintilla_get_hotspot_single_line(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2497, 0, 0);
}

void gtk_scintilla_para_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2413, 0, 0);
}

void gtk_scintilla_para_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2414, 0, 0);
}

void gtk_scintilla_para_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2415, 0, 0);
}

void gtk_scintilla_para_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2416, 0, 0);
}

glong gtk_scintilla_position_before(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2417, (gulong) pos, 0);
}

glong gtk_scintilla_position_after(GtkScintilla *sci, glong pos)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2418, (gulong) pos, 0);
}

void gtk_scintilla_copy_range(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2419, (gulong) start, (gulong) end);
}

void gtk_scintilla_copy_text(GtkScintilla *sci, gint length, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2420, (gulong) length, (gulong) text);
}

void gtk_scintilla_set_selection_mode(GtkScintilla *sci, gint mode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2422, (gulong) mode, 0);
}

gint gtk_scintilla_get_selection_mode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2423, 0, 0);
}

glong gtk_scintilla_get_line_sel_start_position(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2424, (gulong) line, 0);
}

glong gtk_scintilla_get_line_sel_end_position(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2425, (gulong) line, 0);
}

void gtk_scintilla_line_down_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2426, 0, 0);
}

void gtk_scintilla_line_up_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2427, 0, 0);
}

void gtk_scintilla_char_left_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2428, 0, 0);
}

void gtk_scintilla_char_right_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2429, 0, 0);
}

void gtk_scintilla_home_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2430, 0, 0);
}

void gtk_scintilla_v_c_home_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2431, 0, 0);
}

void gtk_scintilla_line_end_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2432, 0, 0);
}

void gtk_scintilla_page_up_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2433, 0, 0);
}

void gtk_scintilla_page_down_rect_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2434, 0, 0);
}

void gtk_scintilla_stuttered_page_up(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2435, 0, 0);
}

void gtk_scintilla_stuttered_page_up_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2436, 0, 0);
}

void gtk_scintilla_stuttered_page_down(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2437, 0, 0);
}

void gtk_scintilla_stuttered_page_down_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2438, 0, 0);
}

void gtk_scintilla_word_left_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2439, 0, 0);
}

void gtk_scintilla_word_left_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2440, 0, 0);
}

void gtk_scintilla_word_right_end(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2441, 0, 0);
}

void gtk_scintilla_word_right_end_extend(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2442, 0, 0);
}

void gtk_scintilla_set_whitespace_chars(GtkScintilla *sci, const gchar * characters)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2443, 0, (gulong) characters);
}

void gtk_scintilla_set_chars_default(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2444, 0, 0);
}

gint gtk_scintilla_autoc_get_current(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2445, 0, 0);
}

gint gtk_scintilla_autoc_get_current_text(GtkScintilla *sci, gchar * s)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2610, 0, (gulong) s);
}

void gtk_scintilla_allocate(GtkScintilla *sci, gint bytes)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2446, (gulong) bytes, 0);
}

gint gtk_scintilla_target_as_u_t_f8(GtkScintilla *sci, gchar * s)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2447, 0, (gulong) s);
}

void gtk_scintilla_set_length_for_encode(GtkScintilla *sci, gint bytes)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2448, (gulong) bytes, 0);
}

gint gtk_scintilla_encoded_from_u_t_f8(GtkScintilla *sci, const gchar * utf8, gchar * encoded)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2449, (gulong) utf8, (gulong) encoded);
}

gint gtk_scintilla_find_column(GtkScintilla *sci, gint line, gint column)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2456, (gulong) line, (gulong) column);
}

gint gtk_scintilla_get_caret_sticky(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2457, 0, 0);
}

void gtk_scintilla_set_caret_sticky(GtkScintilla *sci, gint use_caret_sticky_behaviour)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2458, (gulong) use_caret_sticky_behaviour, 0);
}

void gtk_scintilla_toggle_caret_sticky(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2459, 0, 0);
}

void gtk_scintilla_set_paste_convert_endings(GtkScintilla *sci, gboolean convert)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2467, (gulong) convert, 0);
}

gboolean gtk_scintilla_get_paste_convert_endings(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2468, 0, 0);
}

void gtk_scintilla_selection_duplicate(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2469, 0, 0);
}

void gtk_scintilla_set_caret_line_back_alpha(GtkScintilla *sci, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2470, (gulong) alpha, 0);
}

gint gtk_scintilla_get_caret_line_back_alpha(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2471, 0, 0);
}

void gtk_scintilla_set_caret_style(GtkScintilla *sci, gint caret_style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2512, (gulong) caret_style, 0);
}

gint gtk_scintilla_get_caret_style(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2513, 0, 0);
}

void gtk_scintilla_set_indicator_current(GtkScintilla *sci, gint indicator)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2500, (gulong) indicator, 0);
}

gint gtk_scintilla_get_indicator_current(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2501, 0, 0);
}

void gtk_scintilla_set_indicator_value(GtkScintilla *sci, gint value)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2502, (gulong) value, 0);
}

gint gtk_scintilla_get_indicator_value(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2503, 0, 0);
}

void gtk_scintilla_indicator_fill_range(GtkScintilla *sci, gint position, gint fill_length)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2504, (gulong) position, (gulong) fill_length);
}

void gtk_scintilla_indicator_clear_range(GtkScintilla *sci, gint position, gint clear_length)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2505, (gulong) position, (gulong) clear_length);
}

gint gtk_scintilla_indicator_all_on_for(GtkScintilla *sci, gint position)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2506, (gulong) position, 0);
}

gint gtk_scintilla_indicator_value_at(GtkScintilla *sci, gint indicator, gint position)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2507, (gulong) indicator, (gulong) position);
}

gint gtk_scintilla_indicator_start(GtkScintilla *sci, gint indicator, gint position)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2508, (gulong) indicator, (gulong) position);
}

gint gtk_scintilla_indicator_end(GtkScintilla *sci, gint indicator, gint position)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2509, (gulong) indicator, (gulong) position);
}

void gtk_scintilla_set_position_cache(GtkScintilla *sci, gint size)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2514, (gulong) size, 0);
}

gint gtk_scintilla_get_position_cache(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2515, 0, 0);
}

void gtk_scintilla_copy_allow_line(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2519, 0, 0);
}

gint gtk_scintilla_get_character_pointer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2520, 0, 0);
}

void gtk_scintilla_set_keys_unicode(GtkScintilla *sci, gboolean keys_unicode)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2521, (gulong) keys_unicode, 0);
}

gboolean gtk_scintilla_get_keys_unicode(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2522, 0, 0);
}

void gtk_scintilla_indic_set_alpha(GtkScintilla *sci, gint indicator, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2523, (gulong) indicator, (gulong) alpha);
}

gint gtk_scintilla_indic_get_alpha(GtkScintilla *sci, gint indicator)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2524, (gulong) indicator, 0);
}

void gtk_scintilla_indic_set_outline_alpha(GtkScintilla *sci, gint indicator, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2558, (gulong) indicator, (gulong) alpha);
}

gint gtk_scintilla_indic_get_outline_alpha(GtkScintilla *sci, gint indicator)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2559, (gulong) indicator, 0);
}

void gtk_scintilla_set_extra_ascent(GtkScintilla *sci, gint extra_ascent)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2525, (gulong) extra_ascent, 0);
}

gint gtk_scintilla_get_extra_ascent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2526, 0, 0);
}

void gtk_scintilla_set_extra_descent(GtkScintilla *sci, gint extra_descent)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2527, (gulong) extra_descent, 0);
}

gint gtk_scintilla_get_extra_descent(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2528, 0, 0);
}

gint gtk_scintilla_marker_symbol_defined(GtkScintilla *sci, gint marker_number)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2529, (gulong) marker_number, 0);
}

void gtk_scintilla_margin_set_text(GtkScintilla *sci, gint line, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2530, (gulong) line, (gulong) text);
}

gint gtk_scintilla_margin_get_text(GtkScintilla *sci, gint line, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2531, (gulong) line, (gulong) text);
}

void gtk_scintilla_margin_set_style(GtkScintilla *sci, gint line, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2532, (gulong) line, (gulong) style);
}

gint gtk_scintilla_margin_get_style(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2533, (gulong) line, 0);
}

void gtk_scintilla_margin_set_styles(GtkScintilla *sci, gint line, const gchar * styles)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2534, (gulong) line, (gulong) styles);
}

gint gtk_scintilla_margin_get_styles(GtkScintilla *sci, gint line, gchar * styles)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2535, (gulong) line, (gulong) styles);
}

void gtk_scintilla_margin_text_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2536, 0, 0);
}

void gtk_scintilla_margin_set_style_offset(GtkScintilla *sci, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2537, (gulong) style, 0);
}

gint gtk_scintilla_margin_get_style_offset(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2538, 0, 0);
}

void gtk_scintilla_annotation_set_text(GtkScintilla *sci, gint line, const gchar * text)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2540, (gulong) line, (gulong) text);
}

gint gtk_scintilla_annotation_get_text(GtkScintilla *sci, gint line, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2541, (gulong) line, (gulong) text);
}

void gtk_scintilla_annotation_set_style(GtkScintilla *sci, gint line, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2542, (gulong) line, (gulong) style);
}

gint gtk_scintilla_annotation_get_style(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2543, (gulong) line, 0);
}

void gtk_scintilla_annotation_set_styles(GtkScintilla *sci, gint line, const gchar * styles)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2544, (gulong) line, (gulong) styles);
}

gint gtk_scintilla_annotation_get_styles(GtkScintilla *sci, gint line, gchar * styles)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2545, (gulong) line, (gulong) styles);
}

gint gtk_scintilla_annotation_get_lines(GtkScintilla *sci, gint line)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2546, (gulong) line, 0);
}

void gtk_scintilla_annotation_clear_all(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2547, 0, 0);
}

void gtk_scintilla_annotation_set_visible(GtkScintilla *sci, gint visible)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2548, (gulong) visible, 0);
}

gint gtk_scintilla_annotation_get_visible(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2549, 0, 0);
}

void gtk_scintilla_annotation_set_style_offset(GtkScintilla *sci, gint style)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2550, (gulong) style, 0);
}

gint gtk_scintilla_annotation_get_style_offset(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2551, 0, 0);
}

void gtk_scintilla_add_undo_action(GtkScintilla *sci, gint token, gint flags)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2560, (gulong) token, (gulong) flags);
}

glong gtk_scintilla_char_position_from_point(GtkScintilla *sci, gint x, gint y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2561, (gulong) x, (gulong) y);
}

glong gtk_scintilla_char_position_from_point_close(GtkScintilla *sci, gint x, gint y)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2562, (gulong) x, (gulong) y);
}

void gtk_scintilla_set_multiple_selection(GtkScintilla *sci, gboolean multiple_selection)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2563, (gulong) multiple_selection, 0);
}

gboolean gtk_scintilla_get_multiple_selection(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2564, 0, 0);
}

void gtk_scintilla_set_additional_selection_typing(GtkScintilla *sci, gboolean additional_selection_typing)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2565, (gulong) additional_selection_typing, 0);
}

gboolean gtk_scintilla_get_additional_selection_typing(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2566, 0, 0);
}

void gtk_scintilla_set_additional_carets_blink(GtkScintilla *sci, gboolean additional_carets_blink)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2567, (gulong) additional_carets_blink, 0);
}

gboolean gtk_scintilla_get_additional_carets_blink(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2568, 0, 0);
}

void gtk_scintilla_set_additional_carets_visible(GtkScintilla *sci, gboolean additional_carets_blink)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2608, (gulong) additional_carets_blink, 0);
}

gboolean gtk_scintilla_get_additional_carets_visible(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2609, 0, 0);
}

gint gtk_scintilla_get_selections(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2570, 0, 0);
}

void gtk_scintilla_clear_selections(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2571, 0, 0);
}

gint gtk_scintilla_set_selection(GtkScintilla *sci, gint caret, gint anchor)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2572, (gulong) caret, (gulong) anchor);
}

gint gtk_scintilla_add_selection(GtkScintilla *sci, gint caret, gint anchor)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2573, (gulong) caret, (gulong) anchor);
}

void gtk_scintilla_set_main_selection(GtkScintilla *sci, gint selection)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2574, (gulong) selection, 0);
}

gint gtk_scintilla_get_main_selection(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2575, 0, 0);
}

void gtk_scintilla_set_selection_n_caret(GtkScintilla *sci, gint selection, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2576, (gulong) selection, (gulong) pos);
}

glong gtk_scintilla_get_selection_n_caret(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2577, (gulong) selection, 0);
}

void gtk_scintilla_set_selection_n_anchor(GtkScintilla *sci, gint selection, glong pos_anchor)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2578, (gulong) selection, (gulong) pos_anchor);
}

glong gtk_scintilla_get_selection_n_anchor(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2579, (gulong) selection, 0);
}

void gtk_scintilla_set_selection_n_caret_virtual_space(GtkScintilla *sci, gint selection, gint space)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2580, (gulong) selection, (gulong) space);
}

gint gtk_scintilla_get_selection_n_caret_virtual_space(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2581, (gulong) selection, 0);
}

void gtk_scintilla_set_selection_n_anchor_virtual_space(GtkScintilla *sci, gint selection, gint space)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2582, (gulong) selection, (gulong) space);
}

gint gtk_scintilla_get_selection_n_anchor_virtual_space(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2583, (gulong) selection, 0);
}

void gtk_scintilla_set_selection_n_start(GtkScintilla *sci, gint selection, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2584, (gulong) selection, (gulong) pos);
}

glong gtk_scintilla_get_selection_n_start(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2585, (gulong) selection, 0);
}

glong gtk_scintilla_get_selection_n_end(GtkScintilla *sci, gint selection)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2587, (gulong) selection, 0);
}

void gtk_scintilla_set_rectangular_selection_caret(GtkScintilla *sci, glong pos)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2588, (gulong) pos, 0);
}

glong gtk_scintilla_get_rectangular_selection_caret(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2589, 0, 0);
}

void gtk_scintilla_set_rectangular_selection_anchor(GtkScintilla *sci, glong pos_anchor)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2590, (gulong) pos_anchor, 0);
}

glong gtk_scintilla_get_rectangular_selection_anchor(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2591, 0, 0);
}

void gtk_scintilla_set_rectangular_selection_caret_virtual_space(GtkScintilla *sci, gint space)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2592, (gulong) space, 0);
}

gint gtk_scintilla_get_rectangular_selection_caret_virtual_space(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2593, 0, 0);
}

void gtk_scintilla_set_rectangular_selection_anchor_virtual_space(GtkScintilla *sci, gint space)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2594, (gulong) space, 0);
}

gint gtk_scintilla_get_rectangular_selection_anchor_virtual_space(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2595, 0, 0);
}

void gtk_scintilla_set_virtual_space_options(GtkScintilla *sci, gint virtual_space_options)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2596, (gulong) virtual_space_options, 0);
}

gint gtk_scintilla_get_virtual_space_options(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2597, 0, 0);
}

void gtk_scintilla_set_rectangular_selection_modifier(GtkScintilla *sci, gint modifier)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2598, (gulong) modifier, 0);
}

gint gtk_scintilla_get_rectangular_selection_modifier(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2599, 0, 0);
}

void gtk_scintilla_set_additional_sel_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2600, (gulong) fore, 0);
}

void gtk_scintilla_set_additional_sel_back(GtkScintilla *sci, glong back)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2601, (gulong) back, 0);
}

void gtk_scintilla_set_additional_sel_alpha(GtkScintilla *sci, gint alpha)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2602, (gulong) alpha, 0);
}

gint gtk_scintilla_get_additional_sel_alpha(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2603, 0, 0);
}

void gtk_scintilla_set_additional_caret_fore(GtkScintilla *sci, glong fore)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2604, (gulong) fore, 0);
}

glong gtk_scintilla_get_additional_caret_fore(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2605, 0, 0);
}

void gtk_scintilla_rotate_selection(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2606, 0, 0);
}

void gtk_scintilla_swap_main_anchor_caret(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2607, 0, 0);
}

gint gtk_scintilla_change_lexer_state(GtkScintilla *sci, glong start, glong end)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2617, (gulong) start, (gulong) end);
}

gint gtk_scintilla_contracted_fold_next(GtkScintilla *sci, gint line_start)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        2618, (gulong) line_start, 0);
}

void gtk_scintilla_vertical_centre_caret(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        2619, 0, 0);
}

void gtk_scintilla_start_record(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        3001, 0, 0);
}

void gtk_scintilla_stop_record(GtkScintilla *sci)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        3002, 0, 0);
}

void gtk_scintilla_set_lexer(GtkScintilla *sci, gint lexer)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4001, (gulong) lexer, 0);
}

gint gtk_scintilla_get_lexer(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4002, 0, 0);
}

void gtk_scintilla_colourise(GtkScintilla *sci, glong start, glong end)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4003, (gulong) start, (gulong) end);
}

void gtk_scintilla_set_property(GtkScintilla *sci, const gchar * key, const gchar * value)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4004, (gulong) key, (gulong) value);
}

void gtk_scintilla_set_keywords(GtkScintilla *sci, gint keyword_set, const gchar * key_words)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4005, (gulong) keyword_set, (gulong) key_words);
}

void gtk_scintilla_set_lexer_language(GtkScintilla *sci, const gchar * language)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4006, 0, (gulong) language);
}

void gtk_scintilla_load_lexer_library(GtkScintilla *sci, const gchar * path)
{
    scintilla_send_message(SCINTILLA(sci->scintilla),
        4007, 0, (gulong) path);
}

gint gtk_scintilla_get_property(GtkScintilla *sci, const gchar * key, gchar * buf)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4008, (gulong) key, (gulong) buf);
}

gint gtk_scintilla_get_property_expanded(GtkScintilla *sci, const gchar * key, gchar * buf)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4009, (gulong) key, (gulong) buf);
}

gint gtk_scintilla_get_property_int(GtkScintilla *sci, const gchar * key)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4010, (gulong) key, 0);
}

gint gtk_scintilla_get_style_bits_needed(GtkScintilla *sci)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4011, 0, 0);
}

gint gtk_scintilla_get_lexer_language(GtkScintilla *sci, gchar * text)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4012, 0, (gulong) text);
}

gint gtk_scintilla_private_lexer_call(GtkScintilla *sci, gint operation, gint pointer)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4013, (gulong) operation, (gulong) pointer);
}

gint gtk_scintilla_property_names(GtkScintilla *sci, gchar * names)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4014, 0, (gulong) names);
}

gint gtk_scintilla_property_type(GtkScintilla *sci, const gchar * name)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4015, (gulong) name, 0);
}

gint gtk_scintilla_describe_property(GtkScintilla *sci, const gchar * name, gchar * description)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4016, (gulong) name, (gulong) description);
}

gint gtk_scintilla_describe_key_word_sets(GtkScintilla *sci, gchar * descriptions)
{
    return scintilla_send_message(SCINTILLA(sci->scintilla),
        4017, 0, (gulong) descriptions);
}


/* --- End of autogenerated code --- */

/* === Support functions =================================================== */
static void
notify_cb (GtkWidget *w, gint param, gpointer notif, gpointer data)
{
    struct SCNotification *notification = (struct SCNotification *) notif;
    
    switch (notification->nmhdr.code) {
        case SCN_STYLENEEDED:
            g_signal_emit (G_OBJECT (data),
                           signals[STYLE_NEEDED], 0,
                           (gint) notification->position);
            break;
        case SCN_UPDATEUI:
            g_signal_emit (G_OBJECT (data),
                           signals[UPDATE_UI], 0);
            break;
        case SCN_CHARADDED:
            g_signal_emit (G_OBJECT (data),
                           signals[CHAR_ADDED], 0,
                           (gint) notification->ch);
            break;
        case SCN_SAVEPOINTREACHED:
            g_signal_emit (G_OBJECT (data),
                           signals[SAVE_POINT_REACHED], 0);
            break;
        case SCN_SAVEPOINTLEFT:
            g_signal_emit (G_OBJECT (data),
                           signals[SAVE_POINT_LEFT], 0);
            break;
        case SCN_MODIFYATTEMPTRO:
            g_signal_emit (G_OBJECT (data),
                           signals[MODIFY_ATTEMPT_RO], 0);
            break;
        case SCN_KEY:
            pass_throug_key (GTK_SCINTILLA (data),
                             (gint) notification->ch,
                             (gint) notification->modifiers);
            g_signal_emit (G_OBJECT (data),
                           signals[KEY], 0,
                           (gint) notification->ch,
                           (gint) notification->modifiers);
            break;
        case SCN_DOUBLECLICK:
            g_signal_emit (G_OBJECT (data),
                           signals[DOUBLE_CLICK], 0);
            break;
        case SCN_MODIFIED:
            g_signal_emit (G_OBJECT (data),
                           signals[MODIFIED], 0,
                           (gint) notification->position,
                           (gint) notification->modificationType,
                           (gchar *)notification->text,
                           (gint) notification->length,
                           (gint) notification->linesAdded,
                           (gint) notification->line,
                           (gint) notification->foldLevelNow,
                           (gint) notification->foldLevelPrev);
            break;
        case SCN_MACRORECORD:
            g_signal_emit (G_OBJECT (data),
                           signals[MACRO_RECORD], 0,
                           (gint) notification->message,
                           (gulong) notification->wParam,
                           (glong) notification->lParam);
            break;
        case SCN_MARGINCLICK:
            g_signal_emit (G_OBJECT (data),
                           signals[MARGIN_CLICK], 0,
                           (gint) notification->modifiers,
                           (gint) notification->position,
                           (gint) notification->margin);
            break;
        case SCN_NEEDSHOWN:
            g_signal_emit (G_OBJECT (data),
                           signals[NEED_SHOWN], 0,
                           (gint) notification->position,
                           (gint) notification->length);
            break;
        case SCN_PAINTED:
            g_signal_emit (G_OBJECT (data),
                           signals[PAINTED], 0);
            break;
        case SCN_USERLISTSELECTION:
            g_signal_emit (G_OBJECT (data),
                           signals[USER_LIST_SELECTION], 0,
                           (gint) notification->listType,
                           (gchar *) notification->text);
            break;
        case SCN_URIDROPPED:
            g_signal_emit (G_OBJECT (data),
                           signals[URI_DROPPED], 0,
                           (gchar *) notification->text);
            break;
        case SCN_DWELLSTART:
            g_signal_emit (G_OBJECT (data),
                           signals[DWELL_START], 0,
                           (gint) notification->position);
            break;
        case SCN_DWELLEND:
            g_signal_emit (G_OBJECT (data),
                           signals[DWELL_END], 0,
                           (gint) notification->position);
            break;
        case SCN_ZOOM:
            g_signal_emit (G_OBJECT (data),
                           signals[ZOOM], 0);
            break;
        default:
            //g_warning ("GtkScintilla2: Notification code %d not handled!\n",
            //           (gint) notification->nmhdr.code);
            break;
    }
}

void
pass_throug_key (GtkScintilla *sci, gint ch, gint modifiers)
{
    gint mods = 0;
    
    if (modifiers & SCMOD_SHIFT)
        mods |= GDK_SHIFT_MASK;
    if (modifiers & SCMOD_CTRL)
        mods |= GDK_CONTROL_MASK;
    if (modifiers & SCMOD_ALT)
        mods |= GDK_MOD1_MASK;
    
    if (sci->accel_group) {
        gtk_accel_groups_activate (G_OBJECT (sci->accel_group),
                                   ch, (GdkModifierType) mods);
    }
}
