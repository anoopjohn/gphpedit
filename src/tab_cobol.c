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

#include "tab_php.h"
#include "tab_util.h"
#include "preferences.h"

void scintilla_cobol_set_lexer(GtkScintilla *scintilla, Preferences prefs)
{
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_COBOL);
  gtk_scintilla_set_style_bits(scintilla, 7);

  /* Example style setting up code! */
  gtk_scintilla_autoc_set_choose_single (scintilla, TRUE);
  gtk_scintilla_set_use_tabs (scintilla, 1);
  gtk_scintilla_set_tab_indents (scintilla, 1);
  gtk_scintilla_set_backspace_unindents (scintilla, 1);
  gtk_scintilla_set_tab_width (scintilla, prefs.indentation_size);
  gtk_scintilla_set_indent (scintilla, prefs.tab_size);

  gtk_scintilla_style_set_font (scintilla, STYLE_DEFAULT, prefs.default_font);
  gtk_scintilla_style_set_size (scintilla, STYLE_DEFAULT, prefs.default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_DEFAULT, prefs.default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_DEFAULT, prefs.default_bold);
  gtk_scintilla_style_set_fore (scintilla, STYLE_DEFAULT, prefs.default_fore);
  gtk_scintilla_style_set_back (scintilla, STYLE_DEFAULT, prefs.default_back);

  gtk_scintilla_style_set_font (scintilla, SCE_C_DEFAULT, prefs.c_default_font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_DEFAULT, prefs.c_string_fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_DEFAULT, prefs.c_default_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_DEFAULT, prefs.c_default_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_DEFAULT, prefs.c_default_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_DEFAULT, prefs.c_default_bold);

  //gtk_scintilla_set_sel_back (GTK_SCINTILLA(editor->scintilla), TRUE, prefs.set_sel_back);
  //gtk_scintilla_marker_set_back (GTK_SCINTILLA(editor->scintilla), TRUE, prefs.marker_back);

  gtk_scintilla_style_set_font (scintilla, STYLE_BRACELIGHT, prefs.php_default_font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACELIGHT, 16711680);// Matching bracket
  gtk_scintilla_style_set_back (scintilla, STYLE_BRACELIGHT, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACELIGHT, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACELIGHT, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (scintilla, STYLE_BRACEBAD, prefs.php_default_font);
  gtk_scintilla_style_set_fore (scintilla, STYLE_BRACEBAD, 255);
  gtk_scintilla_style_set_back (scintilla, STYLE_BRACEBAD, prefs.php_default_back);
  gtk_scintilla_style_set_size (scintilla, STYLE_BRACEBAD, prefs.php_default_size);
  gtk_scintilla_style_set_italic (scintilla, STYLE_BRACEBAD, prefs.php_default_italic);
  gtk_scintilla_style_set_bold (scintilla, STYLE_BRACEBAD, TRUE);
  
  gtk_scintilla_set_indentation_guides (scintilla, prefs.show_indentation_guides);
  gtk_scintilla_set_edge_mode (scintilla, prefs.edge_mode);
  gtk_scintilla_set_edge_column (scintilla, prefs.edge_column);
  gtk_scintilla_set_edge_colour (scintilla, prefs.edge_colour);

  gtk_scintilla_set_keywords(scintilla, 0, "configuration data declaratives division environment environment-division file file-control function i-o i-o-control identification input input-output linkage local-storage output procedure program program-id receive-control section special-names working-storage");
  gtk_scintilla_set_keywords(scintilla, 1, "accept add alter apply assign call chain close compute continue control convert copy count delete display divide draw drop eject else enable end-accept end-add end-call end-chain end-compute end-delete end-display end-divide end-evaluate end-if end-invoke end-multiply end-perform end-read end-receive end-return end-rewrite end-search end-start end-string end-subtract end-unstring end-write erase evaluate examine exec execute exit go goback generate if ignore initialize initiate insert inspect invoke leave merge move multiply open otherwise perform print read receive release reload replace report reread rerun reserve reset return rewind rewrite rollback run search seek select send set sort start stop store string subtract sum suppress terminate then transform unlock unstring update use wait when wrap write");
  gtk_scintilla_set_keywords(scintilla, 2, "a access acquire actual address advancing after all allowing alphabet alphabetic alphabetic-lower alphabetic-upper alphanumeric alphanumeric-edited also alternate and any are area areas as ascending at attribute author auto auto-hyphen-skip auto-skip automatic autoterminate background-color background-colour backward basis beep before beginning bell binary blank blink blinking block bold bottom box boxed by c01 c02 c03 c04 c05 c06 c07 c08 c09 c10 c11 c12 cancel cbl cd centered cf ch chaining changed character characters chart class clock-units cobol code code-set col collating color colour column com-reg comma command-line commit commitment common communication comp comp-0 comp-1 comp-2 comp-3 comp-4 comp-5 comp-6 comp-x compression computational computational-1 computational-2 computational-3 computational-4 computational-5 computational-6 computational-x computational console contains content control-area controls conversion converting core-index corr corresponding crt crt-under csp currency current-date cursor cycle cyl-index cyl-overflow date date-compiled date-written day day-of-week dbcs de debug debug-contents debug-item debug-line debug-name debug-sub-1 debug-sub-2 debug-sub-3 debugging decimal-point default delimited delimiter depending descending destination detail disable disk disp display-1 display-st down duplicates dynamic echo egcs egi emi empty-check encryption end end-of-page ending enter entry eol eop eos equal equals error escape esi every exceeds exception excess-3 exclusive exhibit extend extended-search external externally-described-key factory false fd fh--fcd fh--keydef file-id file-limit file-limits file-prefix filler final first fixed footing for foreground-color foreground-colour footing format from full giving global greater grid group heading high high-value high-values highlight id in index indexed indic indicate indicator indicators inheriting initial installation into invalid invoked is japanese just justified kanji kept key keyboard label last leading left left-justify leftline length length-check less limit limits lin linage linage-counter line line-counter lines lock lock-holding locking low low-value low-values lower lowlight manual mass-update master-index memory message method mode modified modules more-labels multiple name named national national-edited native nchar negative next no no-echo nominal not note nstd-reels null nulls number numeric numeric-edited numeric-fill o-fill object object-computer object-storage occurs of off omitted on oostackptr optional or order organization other others overflow overline packed-decimal padding page page-counter packed-decimal paragraph password pf ph pic picture plus pointer pop-up pos position positioning positive previous print-control print-switch printer printer-1 printing prior private procedure-pointer procedures proceed process processing prompt protected public purge queue quote quotes random range rd readers ready record record-overflow recording records redefines reel reference references relative remainder remarks removal renames reorg-criteria repeated replacing reporting reports required resident return-code returning reverse reverse-video reversed rf rh right right-justify rolling rounded s01 s02 s03 s04 s05 same screen scroll sd secure security segment segment-limit selective self selfclass sentence separate sequence sequential service setshadow shift-in shift-out sign size skip1 skip2 skip3 sort-control sort-core-size sort-file-size sort-merge sort-message sort-mode-size sort-option sort-return source source-computer space spaces space-fill spaces standard standard-1 standard-2 starting status sub-queue-1 sub-queue-2 sub-queue-3 subfile super symbolic sync synchronized sysin sysipt syslst sysout syspch syspunch system-info tab tallying tape terminal terminal-info test text than through thru time time-of-day time-out timeout times title to top totaled totaling trace track-area track-limit tracks trailing trailing-sign transaction true type typedef underline underlined unequal unit until up updaters upon upper upsi-0 upsi-1 upsi-2 upsi-3 upsi-4 upsi-5 upsi-6 upsi-7 usage user using value values variable varying when-compiled window with words write-only write-verify writerszero x zero zero-fill zeros zeroes");

  gtk_scintilla_style_set_fore (scintilla, SCE_C_STRING, prefs.c_string_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_STRING, prefs.c_string_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_STRING, prefs.c_string_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_STRING, prefs.c_string_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_STRING, prefs.c_string_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_STRING, prefs.c_string_bold);
  
  gtk_scintilla_style_set_fore (scintilla, SCE_C_CHARACTER, prefs.c_character_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_CHARACTER, prefs.c_character_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_CHARACTER, prefs.c_character_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_CHARACTER, prefs.c_character_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_CHARACTER, prefs.c_character_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_CHARACTER, prefs.c_character_bold);

  gtk_scintilla_style_set_font (scintilla, SCE_C_WORD, prefs.c_word_font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD, prefs.c_word_fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_WORD, prefs.c_word_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_WORD, prefs.c_word_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD, prefs.c_word_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD, prefs.c_word_bold);


  gtk_scintilla_style_set_font (scintilla, SCE_C_WORD2, prefs.c_word_font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD2, prefs.c_word_fore);
  gtk_scintilla_style_set_back (scintilla, SCE_C_WORD2, prefs.c_word_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_WORD2, prefs.c_word_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD2, prefs.c_word_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD2, prefs.c_word_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_NUMBER, prefs.c_number_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_NUMBER, prefs.c_number_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_NUMBER, prefs.c_number_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_NUMBER, prefs.c_number_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_NUMBER, prefs.c_number_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_NUMBER, prefs.c_number_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_IDENTIFIER, prefs.c_identifier_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENT, prefs.c_comment_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENT, prefs.c_comment_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENT, prefs.c_comment_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENT, prefs.c_comment_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENT, prefs.c_comment_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENT, prefs.c_comment_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENTLINE, prefs.c_commentline_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_PREPROCESSOR, prefs.c_preprocesor_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_PREPROCESSOR, prefs.c_preprocesor_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_PREPROCESSOR, prefs.c_preprocesor_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_PREPROCESSOR, prefs.c_preprocesor_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_PREPROCESSOR, prefs.c_preprocesor_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_PREPROCESSOR, TRUE);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_OPERATOR, prefs.c_operator_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_OPERATOR, prefs.c_operator_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_OPERATOR, prefs.c_operator_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_OPERATOR, prefs.c_operator_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_OPERATOR, prefs.c_operator_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_OPERATOR, prefs.c_operator_bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_UUID, prefs.c_uuid_fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_UUID, prefs.c_uuid_font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_UUID, prefs.c_uuid_back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_UUID, prefs.c_uuid_size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_UUID, prefs.c_uuid_italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_UUID, prefs.c_uuid_bold);

  gtk_scintilla_indic_set_style(scintilla, 0, INDIC_SQUIGGLE);
  gtk_scintilla_indic_set_fore(scintilla, 0, scintilla_color(255,0,0));

  gtk_scintilla_set_property(scintilla, "fold.compact", "1");
  gtk_scintilla_set_property(scintilla, "fold", "1");
  gtk_scintilla_colourise(scintilla, 0, -1);
}

void tab_cobol_set_lexer(Editor *editor)
{
  scintilla_cobol_set_lexer(GTK_SCINTILLA(editor->scintilla), preferences);
}
