/* This file is part of gPHPEdit, a GNOME2 PHP Editor.
 
   Copyright (C) 2003, 2004, 2005 Andy Jeffries <andy at gphpedit.org>
   Copyright (C) 2009 Anoop John <anoop dot john at zyxware.com>
   Copyright (C) 2009 José Rostagno(for vijona.com.ar)
    
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

#include "tab_sql.h"
#include "tab_util.h"
#include "preferences_manager.h"
#include "main_window.h"
#include "gtksourcestyleschememanager.h"


void tab_sql_set_lexer(Document *document)
{
  Preferences_Manager *prefmg = preferences_manager_new ();
  GtkScintilla *scintilla = document_get_scintilla(document);
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_SQL);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gtk_scintilla_set_keywords(scintilla, 0, "absolute action add admin after aggregate alias all allocate alter and any are array as asc assertion at authorization before begin binary bit blob boolean both breadth by call cascade cascaded case cast catalog char character check class clob close collate collation column commit completion connect connection constraint constraints constructor continue corresponding create cross cube current current_date current_path current_role current_time current_timestamp current_user cursor cycle data date day deallocate dec decimal declare default deferrable deferred delete depth deref desc describe descriptor destroy destructor deterministic dictionary diagnostics disconnect distinct domain double drop dynamic each else end end-exec equals escape every except exception exec execute external false fetch first float for foreign found from free full function general get global go goto grant group grouping having host hour identity if ignore immediate in indicator initialize initially inner inout input insert int integer intersect interval into is isolation iterate join key language large last lateral leading left less level like limit local localtime localtimestamp locator map match minute modifies modify module month names national natural nchar nclob new next no none not null numeric object of off old on only open operation option or order ordinality out outer output pad parameter parameters partial path postfix precision prefix preorder prepare preserve primary prior privileges procedure public read reads real recursive ref references referencing relative restrict result return returns revoke right role rollback rollup routine row rows savepoint schema scroll scope search second section select sequence session session_user set sets size smallint some| space specific specifictype sql sqlexception sqlstate sqlwarning start state statement static structure system_user table temporary terminate than then time timestamp timezone_hour timezone_minute to trailing transaction translation treat trigger true under union unique unknown unnest update usage user using value values varchar variable varying view when whenever where with without work write year zone");

gtk_scintilla_set_keywords(scintilla, 1, "all alter and any array as asc at authid avg begin between binary_integer body boolean bulk by char char_base check close cluster collect comment commit compress connect constant create current currval cursor date day declare decimal default delete desc distinct do drop else elsif end exception exclusive execute exists exit extends false fetch float for forall from function goto group having heap hour if immediate in index indicator insert integer interface intersect interval into is isolation java level like limited lock long loop max min minus minute mlslabel mod mode month natural naturaln new nextval nocopy not nowait null number number_base ocirowid of on opaque open operator option or order organization others out package partition pctfree pls_integer positive positiven pragma prior private procedure public raise range raw real record ref release return reverse rollback row rowid rownum rowtype savepoint second select separate set share smallint space sql sqlcode sqlerrm start stddev subtype successful sum synonym sysdate table then time timestamp to trigger true type uid union unique update use user validate values varchar varchar2 variance view when whenever where while with work write year zone");

  const gchar *font = get_preferences_manager_style_font(prefmg);
  guint size = get_preferences_manager_style_size(prefmg);
  GtkSourceStyleScheme	*scheme = gtk_source_style_scheme_manager_get_scheme (main_window.stylemg, get_preferences_manager_style_name(prefmg));
  /* SQL LEXER STYLE */
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_DEFAULT, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_WORD, font, size);
  set_scintilla_lexer_xml_element_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_WORD2, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_SQLPLUS, font, size);
  set_scintilla_lexer_keyword_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_SQLPLUS_PROMPT, font, size);
  set_scintilla_lexer_string_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_STRING, font, size);
  set_scintilla_lexer_simple_string_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_CHARACTER, font, size);
  set_scintilla_lexer_operator_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_OPERATOR, font, size);
  set_scintilla_lexer_number_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_NUMBER, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_SQL_COMMENT, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_SQL_COMMENTLINE, font, size);
  set_scintilla_lexer_comment_style (GTK_WIDGET(scintilla), scheme, SCE_SQL_SQLPLUS_COMMENT, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_COMMENTDOC, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_COMMENTDOCKEYWORD, font, size);
  set_scintilla_lexer_doc_comment_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_COMMENTLINEDOC, font, size);
  set_scintilla_lexer_variable_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_IDENTIFIER, font, size);
  set_scintilla_lexer_special_constant_style (GTK_WIDGET(scintilla), scheme, SCE_SQL_QUOTEDIDENTIFIER, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_USER1, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_USER2, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_USER3, font, size);
  set_scintilla_lexer_default_style(GTK_WIDGET(scintilla), scheme, SCE_SQL_USER4, font, size);

  gtk_scintilla_colourise(scintilla, 0, -1);
  g_object_unref(prefmg);
}
