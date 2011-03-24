/* This file is part of gPHPEdit, a GNOME PHP Editor.
 
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009, 2011 José Rostagno (for vijona.com.ar)
	  
   For more information or to find the latest release, visit our 
   website at http://www.gperledit.org/
 
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
#include <string.h>
#include <glib/gi18n.h>

#include "debug.h"
#include "language_perl.h"
#include "preferences_manager.h"
#include "language_provider.h"
#include "symbol_manager.h"
#include "main_window.h"
#include "gvfs_utils.h"

/*
* language_perl private struct
*/
struct Language_PERLDetails
{
  Documentable *doc;
  GtkScintilla *sci;
  PreferencesManager *prefmg;
  SymbolManager *symbolmg;

  /* calltip stuff*/
  guint calltip_timer_id;
  gboolean calltip_timer_set;
  /*completion stuff*/
  guint completion_timer_id;
  gboolean completion_timer_set;
};

#define LANGUAGE_PERL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object),\
					    LANGUAGE_PERL_TYPE,\
					    Language_PERLDetails))
enum
{
  PROP_0,
  PROP_DOCUMENT_SCINTILLA
};

static void language_perl_language_provider_init(Language_ProviderInterface *iface, gpointer user_data);
static void language_perl_trigger_completion (Language_Provider *lgperl, guint ch);
static void show_calltip (Language_Provider *lgperl);
static void language_perl_setup_lexer(Language_Provider *lgperl);
static gchar *language_perl_do_syntax_check(Language_Provider *lgperl);

G_DEFINE_TYPE_WITH_CODE(Language_PERL, language_perl, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE (IFACE_TYPE_LANGUAGE_PROVIDER,
                                                 language_perl_language_provider_init));

static void language_perl_language_provider_init(Language_ProviderInterface *iface, gpointer user_data)
{
  iface->trigger_completion = language_perl_trigger_completion;
  iface->show_calltip = show_calltip;
  iface->setup_lexer = language_perl_setup_lexer;
  iface->do_syntax_check = language_perl_do_syntax_check;
}

static void
language_perl_set_property (GObject      *object,
			      guint         prop_id,
			      const GValue *value,
			      GParamSpec   *pspec)
{
  Language_PERLDetails *lgperldet;
  lgperldet = LANGUAGE_PERL_GET_PRIVATE(object);

  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      if (lgperldet->doc) g_object_unref(lgperldet->doc);
      lgperldet->doc = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
language_perl_get_property (GObject    *object,
			      guint       prop_id,
			      GValue     *value,
			      GParamSpec *pspec)
{
  Language_PERLDetails *lgperldet;
  lgperldet = LANGUAGE_PERL_GET_PRIVATE(object);

  
  switch (prop_id)
  {
    case PROP_DOCUMENT_SCINTILLA:
      g_value_set_object (value, lgperldet->doc);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}


/*
* disposes the Gobject
*/
static void language_perl_dispose (GObject *object)
{
  Language_PERLDetails *lgperldet;
  lgperldet = LANGUAGE_PERL_GET_PRIVATE(object);
  g_object_unref(lgperldet->prefmg);
  g_object_unref(lgperldet->doc);
  g_object_unref(lgperldet->symbolmg);
  /* Chain up to the parent class */
  G_OBJECT_CLASS (language_perl_parent_class)->dispose (object);
}

static void language_perl_constructed (GObject *object)
{
  Language_PERLDetails *lgperldet;
  lgperldet = LANGUAGE_PERL_GET_PRIVATE(object);
  GtkScintilla *sci;
  g_object_get(lgperldet->doc, "scintilla", &sci, NULL);
  lgperldet->sci = sci;
}

static void
language_perl_class_init (Language_PERLClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);
  object_class->dispose = language_perl_dispose;
  object_class->set_property = language_perl_set_property;
  object_class->get_property = language_perl_get_property;
  object_class->constructed = language_perl_constructed;
  g_object_class_install_property (object_class,
                              PROP_DOCUMENT_SCINTILLA,
                              g_param_spec_object ("document_scintilla",
                              NULL, NULL,
                              DOCUMENT_SCINTILLA_TYPE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));

  g_type_class_add_private (klass, sizeof (Language_PERLDetails));
}

static void
language_perl_init (Language_PERL * object)
{
  Language_PERLDetails *lgperldet;
  lgperldet = LANGUAGE_PERL_GET_PRIVATE(object);
  lgperldet->prefmg = preferences_manager_new ();
  lgperldet->symbolmg = symbol_manager_new ();
}

Language_PERL *language_perl_new (Document_Scintilla *doc)
{
  Language_PERL *lgperl;
  lgperl = g_object_new (LANGUAGE_PERL_TYPE, "document_scintilla", doc, NULL);
  return lgperl; /* return new object */
}

typedef struct {
 Language_PERL *lgperl;
 gint old_pos;
} Calltip_Data;

static gboolean auto_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(dat->lgperl);
  gint current_pos;
  current_pos = documentable_get_current_position(lgperldet->doc);
  GtkScintilla *scintilla = lgperldet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgperldet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgperldet->symbolmg, prefix, SYMBOL_FUNCTION | SYMBOL_CLASS | SYMBOL_VAR, TAB_PERL);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
    g_free(prefix);
    g_free(calltip);
  }
  g_slice_free(Calltip_Data, dat);
  lgperldet->completion_timer_set=FALSE;
  return FALSE;
}

static void show_autocompletion (Language_PERL *lgperl, gint pos)
{
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);
  if (!lgperldet->completion_timer_set) {
    gint delay;
    g_object_get(lgperldet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgperl = lgperl;
    dat->old_pos = pos;
    lgperldet->completion_timer_id = g_timeout_add(delay, auto_complete_callback, dat);
    lgperldet->completion_timer_set=TRUE;
  }
}

static gboolean auto_memberfunc_complete_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(dat->lgperl);
  gint current_pos;
  current_pos = documentable_get_current_position(lgperldet->doc);
  GtkScintilla *scintilla = lgperldet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgperldet->doc);
    gchar *calltip = symbol_manager_get_symbols_matches (lgperldet->symbolmg, prefix, SYMBOL_MEMBER, TAB_PERL);
    if (calltip && strlen(calltip)!=0) gtk_scintilla_user_list_show(GTK_SCINTILLA(scintilla), 1, calltip);
//    gtk_scintilla_autoc_show(GTK_SCINTILLA(docdet->scintilla), current_pos, calltip);
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgperldet->completion_timer_set=FALSE;
  return FALSE;
}

static void autocomplete_member(Language_PERL *lgperl, gint pos)
{
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);
  if (!lgperldet->completion_timer_set) {
    gint delay;
    g_object_get(lgperldet->prefmg, "autocomplete_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgperl = lgperl;
    dat->old_pos = pos;
    lgperldet->completion_timer_id = g_timeout_add(delay, auto_memberfunc_complete_callback, dat);
    lgperldet->completion_timer_set=TRUE;
  }
}

static gboolean calltip_callback(gpointer data)
{
  Calltip_Data *dat = (Calltip_Data *) data;
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(dat->lgperl);
  gint current_pos;
  current_pos = documentable_get_current_position(lgperldet->doc);
  GtkScintilla *scintilla = lgperldet->sci;
  if (current_pos == dat->old_pos) {
    gchar *prefix = documentable_get_current_word(lgperldet->doc);
    gchar *calltip = symbol_manager_get_calltip (lgperldet->symbolmg, prefix, TAB_PERL);
    if (calltip){
      gtk_scintilla_call_tip_show(GTK_SCINTILLA(scintilla), current_pos, calltip);
      g_free(calltip);
    }
    g_free(prefix);
  }
  g_slice_free(Calltip_Data, dat);
  lgperldet->calltip_timer_set = FALSE;
  return FALSE;
}

static void show_calltip (Language_Provider *lgperl)
{
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);
  if (!lgperldet->calltip_timer_set) {
    gint delay;
    g_object_get(lgperldet->prefmg, "calltip_delay", &delay, NULL);
    Calltip_Data *dat = g_slice_new(Calltip_Data);
    dat->lgperl = LANGUAGE_PERL(lgperl);
    dat->old_pos = documentable_get_current_position(lgperldet->doc);
    lgperldet->calltip_timer_id = g_timeout_add(delay, calltip_callback, dat);
    lgperldet->calltip_timer_set = TRUE;
  }
}

static gboolean IsOpenBrace(gchar ch)
{
	return ch == '[' || ch == '(' || ch == '{';
}

static void InsertCloseBrace (GtkScintilla *scintilla, gint current_pos, gchar ch)
{
  switch (ch) {
    case '[':
      gtk_scintilla_insert_text(scintilla, current_pos,"]");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
    case '{':
      gtk_scintilla_insert_text(scintilla, current_pos,"}");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
    case '(':
      gtk_scintilla_insert_text(scintilla, current_pos,")");
      gtk_scintilla_goto_pos(scintilla, current_pos);
      break;
  }
}

static void indent_line(GtkScintilla *sci, gint line, gint indent)
{
  gint selStart;
  gint selEnd;
  gint posBefore;
  gint posAfter;
  gint posDifference;

  selStart = gtk_scintilla_get_selection_start(sci);
  selEnd = gtk_scintilla_get_selection_end(sci);
  posBefore = gtk_scintilla_get_line_indentation(sci, line);
  gtk_scintilla_set_line_indentation(sci, line, indent);
  posAfter = gtk_scintilla_get_line_indentation(sci, line);
  posDifference =  posAfter - posBefore;

  if (posAfter > posBefore) {
    // Move selection on
    if (selStart >= posBefore) {
      selStart += posDifference;
    }
    if (selEnd >= posBefore) {
      selEnd += posDifference;
    }
  }
  else if (posAfter < posBefore) {
    // Move selection back
    if (selStart >= posAfter) {
      if (selStart >= posBefore)
        selStart += posDifference;
      else
        selStart = posAfter;
    }
    if (selEnd >= posAfter) {
      if (selEnd >= posBefore)
        selEnd += posDifference;
      else
        selEnd = posAfter;
    }
  }
  gtk_scintilla_set_selection_start(sci, selStart);
  gtk_scintilla_set_selection_end(sci, selEnd);
}

static gboolean is_perl_char_autoindent(char ch)
{
  return (ch == '{');
}

static gboolean is_perl_char_autounindent(char ch)
{
  return (ch == '}');
}

static void autoindent_brace_code (GtkScintilla *sci, PreferencesManager *pref)
{
  gint current_pos;
  gint current_line;
  gint previous_line;
  gint previous_line_indentation;
  gint previous_line_start;
  gint previous_line_end;
  gchar *previous_char_buffer;
  gint previous_char_buffer_length;
  gchar *previous_line_buffer;
  gint previous_line_buffer_length;

  current_pos = gtk_scintilla_get_current_pos(sci);
  current_line = gtk_scintilla_line_from_position(sci, current_pos);

  gphpedit_debug (DEBUG_DOCUMENT);

  if (current_line>0) {
    gtk_scintilla_begin_undo_action(sci);
    previous_line = current_line-1;
    previous_line_indentation = gtk_scintilla_get_line_indentation(sci, previous_line);

    previous_line_end = gtk_scintilla_get_line_end_position(sci, previous_line);
    previous_char_buffer = gtk_scintilla_get_text_range (sci, previous_line_end-1, previous_line_end, &previous_char_buffer_length);
    if (is_perl_char_autoindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation+=indentation_size;
    } else if (is_perl_char_autounindent(*previous_char_buffer)) {
      gint indentation_size;
      g_object_get(pref, "indentation_size", &indentation_size, NULL);
      previous_line_indentation-=indentation_size;
      if (previous_line_indentation < 0) previous_line_indentation = 0;
      previous_line_start = gtk_scintilla_position_from_line(sci, previous_line);
      previous_line_buffer = gtk_scintilla_get_text_range (sci, previous_line_start, previous_line_end, &previous_line_buffer_length);
      gboolean unindent = TRUE;
      gint char_act = 0;
      while (char_act <= previous_line_buffer_length)
      {
        char c = previous_line_buffer[char_act];
        if (!(g_ascii_iscntrl(c) || g_ascii_isspace(c) || is_perl_char_autounindent(c))) {
          unindent = FALSE;
          break;
        }
        char_act++;
      }
      if (unindent) gtk_scintilla_set_line_indentation(sci, previous_line, previous_line_indentation);
      g_free(previous_line_buffer);
    }
    g_free(previous_char_buffer);
    indent_line(sci, current_line, previous_line_indentation);
    gphpedit_debug_message (DEBUG_DOCUMENT, "previous_line=%d, previous_indent=%d\n", previous_line, previous_line_indentation);
    gint pos;
    gboolean tabs_instead_spaces;
    g_object_get(pref,"tabs_instead_spaces", &tabs_instead_spaces, NULL);
    if(tabs_instead_spaces){
      pos = gtk_scintilla_position_from_line(sci, current_line)+(previous_line_indentation/gtk_scintilla_get_tab_width(sci));
    } else {
      pos = gtk_scintilla_position_from_line(sci, current_line)+(previous_line_indentation);
    }
    gtk_scintilla_goto_pos(sci, pos);
    gtk_scintilla_end_undo_action(sci);
  }
}

static void cancel_calltip (GtkScintilla *sci)
{
  if (gtk_scintilla_call_tip_active(sci)) {
    gtk_scintilla_call_tip_cancel(sci);
  }
}

/*
 * gboolean check_variable_before(const gchar *line_text)
 * check if there is a valid perl variable has suffix in the gchar input
 * something like this "p$sk->" return FALSE
 * $this-> return TRUE
 * $var($this-> return TRUE
 * $var[$this-> return TRUE
 */
static gboolean check_perl_variable_before(const gchar *line_text)
{
  gboolean r=FALSE;
  int i;
  if (!strchr(line_text,'$')) return r;
  for (i=strlen(line_text)-1;i>=0;i--){
    if (*(line_text+i)==';') break;
    if (*(line_text+i)==' ') break;
    if (*(line_text+i)=='$' && (*(line_text+i-1)==' ' || *(line_text+i-1)=='(' || *(line_text+i-1)=='[' || i==0)){  
    r=TRUE; 
    break;
    }
  }
  return r;
}

static void language_perl_trigger_completion (Language_Provider *lgperl, guint ch)
{
  g_return_if_fail(lgperl);
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);
  gint current_pos;
  gchar *member_function_buffer = NULL;
  guint prev_char;
  gint current_line;
  gint wordStart;
  current_pos = gtk_scintilla_get_current_pos(lgperldet->sci);
  current_line = gtk_scintilla_line_from_position(lgperldet->sci, current_pos);
  wordStart = gtk_scintilla_word_start_position(lgperldet->sci, current_pos-1, TRUE);
  gboolean auto_brace;
  g_object_get(lgperldet->prefmg, "auto_complete_braces", &auto_brace, NULL);

  if (IsOpenBrace(ch) && auto_brace) {
    InsertCloseBrace (lgperldet->sci, current_pos, ch);
    return;
  }

  switch(ch) {
    case ('\r'):
    case ('\n'):
        autoindent_brace_code (lgperldet->sci, lgperldet->prefmg);
        break;
    case (')'):
        cancel_calltip (lgperldet->sci);
        break;
    case ('('):
        show_calltip (lgperl);
        break;
     case ('>'):
     case (':'):
       prev_char = gtk_scintilla_get_char_at(lgperldet->sci, current_pos-2);
       if ((prev_char=='-' && ch =='>') || (prev_char==':' && ch ==':')) {
         /*search back for a '$' in that line */
         gint initial_pos= gtk_scintilla_position_from_line(lgperldet->sci, current_line);
         gint line_size;
         gchar *line_text = gtk_scintilla_get_text_range (lgperldet->sci, initial_pos, wordStart-1, &line_size);
         if (!check_perl_variable_before(line_text)) break;
         autocomplete_member(LANGUAGE_PERL(lgperl), current_pos);
       }
       break;
    default:
        member_function_buffer = documentable_get_current_word(lgperldet->doc);
        if (member_function_buffer && strlen(member_function_buffer)>=3) show_autocompletion (LANGUAGE_PERL(lgperl), current_pos);
        g_free(member_function_buffer);
  }
}

/*
* (internal)
*/

static gchar *process_perl_lines(gchar *output)
{
  gchar *copy;
  gchar *token;
  gchar *line_number;
  copy = output;
  GString *result;
  result = g_string_new (NULL);
  gphpedit_debug_message(DEBUG_SYNTAX, "syntax:\n%s\n", output);
  gint quote=0;
  gint a=0;
  gchar *cop=copy;
  while (*cop!='\0'){
      if(*cop=='"' && quote==0) quote++;
      else if(*cop=='"' && quote!=0) quote--;
      if (*cop=='\n' && quote==1) *(copy +a)=' ';
      cop++;
      a++;
//      g_print("char:%c, quote:%d,pos:%d\n",*cop,quote,a);
      }      
      while ((token = strtok(copy, "\n"))) {
        gchar number[15];  
        int i=15;
        line_number = strstr(token, "line ");
        if (line_number){
        line_number+=5;
        while (*line_number!=',' && *line_number!='.' && i!=0){
        number[15-i]=*line_number;
        line_number++;
        i--;
        }
        number[i]='\0';
        }
        gint num=atoi(number);
        if (num>0) {
          if (g_str_has_prefix(token, "syntax error") || g_str_has_prefix(token, "Unrecognized character")){
            g_string_append_printf (result, "%d E %s\n", num, token);
          } else {
            g_string_append_printf (result, "%d %s\n", num, token);
          }
        }
        else {
           if (g_str_has_suffix(token, "syntax OK")) g_string_append_printf (result, "%s", "syntax OK\n");
           else g_string_append_printf (result, "%s\n", token);
        }
      number[0]='a'; /*force new number */
      copy = NULL;
    }
  return g_string_free (result,FALSE);
}

/*
* save_as_temp_file (internal)
* save the content of an editor and return the filename of the temp file or NULL on error.
*/
static GString *save_as_temp_file(Documentable *document)
{
  gphpedit_debug(DEBUG_SYNTAX);
  gchar *write_buffer = documentable_get_text(document);
  GString *filename = text_save_as_temp_file(write_buffer);
  g_free(write_buffer);
  return filename;
}

static GString *get_syntax_filename(Documentable *document, gboolean *using_temp)
{
  GString *filename = NULL;
  gchar *docfilename = documentable_get_filename(document);
  gboolean untitled, saved;
  g_object_get(document, "untitled", &untitled, "saved", &saved, NULL);
  if (saved && filename_is_native(docfilename) && !untitled) {
    gchar *local_path = filename_get_scaped_path(docfilename);
    filename = g_string_new(local_path);
    g_free(local_path);
    *using_temp = FALSE;
  } else {
    filename = save_as_temp_file(document);
    *using_temp = TRUE;
  }
  g_free(docfilename);
  return filename;
}

static gchar *language_perl_do_syntax_check(Language_Provider *lgperl)
{
  g_return_val_if_fail(lgperl, NULL);
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);

  GString *command_line=NULL;
  gchar *output;
  gboolean using_temp;
  GString *filename = NULL;

  command_line = g_string_new(NULL);
  filename = get_syntax_filename(lgperldet->doc, &using_temp);
  g_string_append_printf(command_line, "perl -c '%s'", filename->str);

  gphpedit_debug_message(DEBUG_SYNTAX, "eject:%s\n", command_line->str);

  output = command_spawn_with_error (command_line->str);
  if (using_temp) release_temp_file (filename->str);
  g_string_free(filename, TRUE);
  g_string_free(command_line, TRUE);
  gchar *result=NULL;
  if (output) {
    result = process_perl_lines(output);
    g_free(output);
  } else {
    result = g_strdup(_("Error calling PHP CLI (is PHP command line binary installed? If so, check if it's in your path or set php_binary in Preferences)\n"));
  }
  return result;
}

static void language_perl_setup_lexer(Language_Provider *lgperl)
{
  g_return_if_fail(lgperl);
  Language_PERLDetails *lgperldet = LANGUAGE_PERL_GET_PRIVATE(lgperl);

  gtk_scintilla_clear_document_style (lgperldet->sci);
  gtk_scintilla_set_lexer(lgperldet->sci, SCLEX_PERL);
  gtk_scintilla_set_style_bits(lgperldet->sci, 5);

  gtk_scintilla_set_keywords(lgperldet->sci, 0, "NULL __FILE__ __LINE__ __PACKAGE__ __DATA__ __END__ AUTOLOAD BEGIN CORE DESTROY END EQ GE GT INIT LE LT NE CHECK abs accept alarm and atan2 bind binmode bless caller chdir chmod chomp chop chown chr chroot close closedir cmp connect continue cos crypt dbmclose dbmopen defined delete die do dump each else elsif endgrent endhostent endnetent endprotoent endpwent endservent eof eq eval exec exists exit exp fcntl fileno flock for foreach fork format formline ge getc getgrent getgrgid getgrnam gethostbyaddr gethostbyname gethostent getlogin getnetbyaddr getnetbyname getnetent getpeername getpgrp getppid getpriority getprotobyname getprotobynumber getprotoent getpwent getpwnam getpwuid getservbyname getservbyport getservent getsockname getsockopt glob gmtime goto grep gt hex if index int ioctl join keys kill last lc lcfirst le length link listen local localtime lock log lstat lt m map mkdir msgctl msgget msgrcv msgsnd my ne next no not oct open opendir or ord our pack package pipe pop pos print printf prototype push q qq qr quotemeta qu qw qx rand read readdir readline readlink readpipe recv redo ref rename require reset return reverse rewinddir rindex rmdir s scalar seek seekdir select semctl semget semop send setgrent sethostent setnetent setpgrp setpriority setprotoent setpwent setservent setsockopt shift shmctl shmget shmread shmwrite shutdown sin sleep socket socketpair sort splice split sprintf sqrt srand stat study sub substr symlink syscall sysopen sysread sysseek system syswrite tell telldir tie tied time times tr truncate uc ucfirst umask undef unless unlink unpack unshift untie until use utime values vec wait waitpid wantarray warn while write x xor y");
  
  const gchar *font;
  guint size;
  g_object_get(lgperldet->prefmg, "style_font_name", &font,"font_size", &size, NULL);

  const gchar *style_name;
  g_object_get(lgperldet->prefmg, "style_name", &style_name, NULL);

  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, style_name);
  /* PHP LEXER STYLE */
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_FORMAT_IDENT, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_FORMAT, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_WORD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_SUB_PROTOTYPE, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_POD_VERB, font, size);
  set_scintilla_lexer_comment_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_POD, font, size);
  set_scintilla_lexer_type_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_IDENTIFIER, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_ARRAY, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_SYMBOLTABLE, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_DATASECTION, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_VARIABLE_INDEXER, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING_Q, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING_QQ, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING_QX, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING_QR, font, size);
  set_scintilla_lexer_xml_atribute_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING_QW, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_LONGQUOTE, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_BACKTICKS, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_CHARACTER, font, size);
  set_scintilla_lexer_preprocessor_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_PREPROCESSOR, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_SCALAR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_COMMENTLINE, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_HASH, font, size);
  set_scintilla_lexer_xml_element_style (GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_REGSUBST, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_REGEX, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_PUNCTUATION, font, size);
  set_scintilla_lexer_error_style (GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_ERROR, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_HERE_DELIM, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_HERE_Q, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_HERE_QQ, font, size);
  set_scintilla_lexer_xml_instruction_style(GTK_WIDGET(lgperldet->sci), scheme, SCE_PL_HERE_QX, font, size);

  gtk_scintilla_set_property(lgperldet->sci, "fold", "1");
  gtk_scintilla_set_property(lgperldet->sci, "fold.comment", "1");
  gtk_scintilla_set_property(lgperldet->sci, "fold.compact", "1");
  gtk_scintilla_set_property(lgperldet->sci, "fold.perl.pod", "1");
  gtk_scintilla_set_property(lgperldet->sci, "fold.perl.package", "1");

  gtk_scintilla_colourise(lgperldet->sci, 0, -1);
}
