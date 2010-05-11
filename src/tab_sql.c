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

void tab_sql_set_lexer(Editor *editor)
{
  gtk_scintilla_clear_document_style (GTK_SCINTILLA(editor->scintilla));
  gtk_scintilla_set_lexer(GTK_SCINTILLA (editor->scintilla), SCLEX_SQL);
  gtk_scintilla_set_style_bits(GTK_SCINTILLA(editor->scintilla), 5);

  /* Example style setting up code! */
  gtk_scintilla_autoc_set_choose_single (GTK_SCINTILLA(editor->scintilla), TRUE);
  gtk_scintilla_set_use_tabs (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_tab_indents (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_backspace_unindents (GTK_SCINTILLA(editor->scintilla), 1);
  gtk_scintilla_set_tab_width (GTK_SCINTILLA(editor->scintilla), preferences.indentation_size);
  gtk_scintilla_set_indent (GTK_SCINTILLA(editor->scintilla), preferences.tab_size);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_DEFAULT, preferences.default_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_fore);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_DEFAULT, preferences.default_bold);

  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, 16711680);// Matching bracket
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, preferences.php_default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, 255);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, preferences.php_default_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), STYLE_BRACEBAD, TRUE);
  
  gtk_scintilla_set_indentation_guides (GTK_SCINTILLA(editor->scintilla), preferences.show_indentation_guides);
  gtk_scintilla_set_edge_mode (GTK_SCINTILLA(editor->scintilla), preferences.edge_mode);
  gtk_scintilla_set_edge_column (GTK_SCINTILLA(editor->scintilla), preferences.edge_column);
  gtk_scintilla_set_edge_colour (GTK_SCINTILLA(editor->scintilla), preferences.edge_colour);

  gtk_scintilla_set_keywords(GTK_SCINTILLA(editor->scintilla), 0, "ADD ALL ALTER ANALYZE AND AS ASC ASENSITIVE AUTO_INCREMENT BDB BEFORE BERKELEYDB BETWEEN BIGINT BINARY BLOB BOTH BTREE BY CALL CASCADE CASE CHANGE CHAR CHARACTER CHECK COLLATE COLUMN COLUMNS CONNECTION CONSTRAINT CREATE CROSS CURRENT_DATE CURRENT_TIME CURRENT_TIMESTAMP CURSOR DATABASE DATABASES DAY_HOUR DAY_MINUTE DAY_SECOND DEC DECIMAL DECLARE DEFAULT DELAYED DELETE DESC DESCRIBE DISTINCT DISTINCTROW DIV DOUBLE DROP ELSE ELSEIF ENCLOSED ERRORS ESCAPED EXISTS EXPLAIN FALSE FIELDS FLOAT FOR FORCE FOREIGN FROM FULLTEXT GRANT GROUP HASH HAVING HIGH_PRIORITY HOUR_MINUTE HOUR_SECOND IF IGNORE IN INDEX INFILE INNER INNODB INOUT INSENSITIVE INSERT INT INTEGER INTERVAL INTO IS ITERATE JOIN KEY KEYS KILL LEADING LEAVE LEFT LIKE LIMIT LINES LOAD LOCALTIME LOCALTIMESTAMP LOCK LONG LONGBLOB LONGTEXT LOOP LOW_PRIORITY MASTER_SERVER_ID MATCH MEDIUMBLOB MEDIUMINT MEDIUMTEXT MIDDLEINT MINUTE_SECOND MOD MRG_MYISAM NATURAL NOT NULL NUMERIC ON OPTIMIZE OPTION OPTIONALLY OR ORDER OUT OUTER OUTFILE PRECISION PRIMARY PRIVILEGES PROCEDURE PURGE READ REAL REFERENCES REGEXP RENAME REPEAT REPLACE REQUIRE RESTRICT RETURN RETURNS REVOKE RIGHT RLIKE RTREE SELECT SENSITIVE SEPARATOR SET SHOW SMALLINT SOME SONAME SPATIAL SPECIFIC SQL_BIG_RESULT SQL_CALC_FOUND_ROWS SQL_SMALL_RESULT SSL STARTING STRAIGHT_JOIN STRIPED TABLE TABLES TERMINATED THEN TINYBLOB TINYINT TINYTEXT TO TRAILING TRUE TYPES UNION UNIQUE UNLOCK UNSIGNED UNTIL UPDATE USAGE USE USER_RESOURCES USING VALUES VARBINARY VARCHAR VARCHARACTER VARYING WARNINGS WHEN WHERE WHILE WITH WRITE XOR YEAR_MONTH ZEROFILL");

  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_back);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_font);
  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_fore);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_WORD, preferences.sql_word_bold);


  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_STRING, preferences.sql_string_bold);


  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_OPERATOR, preferences.sql_operator_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENT, preferences.sql_comment_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_COMMENTLINE, preferences.sql_comment_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_NUMBER, preferences.sql_number_bold);

  gtk_scintilla_style_set_fore (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_identifier_fore);
  gtk_scintilla_style_set_font (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_identifier_font);
  gtk_scintilla_style_set_back (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_identifier_back);
  gtk_scintilla_style_set_size (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_number_size);
  gtk_scintilla_style_set_italic (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_number_italic);
  gtk_scintilla_style_set_bold (GTK_SCINTILLA(editor->scintilla), SCE_C_IDENTIFIER, preferences.sql_number_bold);

  gtk_scintilla_colourise(GTK_SCINTILLA (editor->scintilla), 0, -1);
}
