
#ifndef __scintilla_marshal_MARSHAL_H__
#define __scintilla_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:INT,INT,INT (marshal.list:1) */
extern void scintilla_marshal_VOID__INT_INT_INT (GClosure     *closure,
                                                 GValue       *return_value,
                                                 guint         n_param_values,
                                                 const GValue *param_values,
                                                 gpointer      invocation_hint,
                                                 gpointer      marshal_data);

/* VOID:INT,ULONG,LONG (marshal.list:2) */
extern void scintilla_marshal_VOID__INT_ULONG_LONG (GClosure     *closure,
                                                    GValue       *return_value,
                                                    guint         n_param_values,
                                                    const GValue *param_values,
                                                    gpointer      invocation_hint,
                                                    gpointer      marshal_data);

/* VOID:INT,INT (marshal.list:3) */
extern void scintilla_marshal_VOID__INT_INT (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* VOID:INT,INT,STRING,INT,INT,INT,INT,INT (marshal.list:4) */
extern void scintilla_marshal_VOID__INT_INT_STRING_INT_INT_INT_INT_INT (GClosure     *closure,
                                                                        GValue       *return_value,
                                                                        guint         n_param_values,
                                                                        const GValue *param_values,
                                                                        gpointer      invocation_hint,
                                                                        gpointer      marshal_data);

G_END_DECLS

#endif /* __scintilla_marshal_MARSHAL_H__ */

