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

#include "tab_sql.h"
#include "tab_util.h"
#include "preferences_manager.h"

void tab_sql_set_lexer(Document *document)
{
  Preferences_Manager *prefmg = preferences_manager_new ();
  GtkScintilla *scintilla = document_get_scintilla(document);
  gtk_scintilla_clear_document_style (scintilla);
  gtk_scintilla_set_lexer(scintilla, SCLEX_SQL);
  gtk_scintilla_set_style_bits(scintilla, 5);

  gchar *font =NULL;
  gint size, fore, back;
  gboolean italic, bold;

  get_preferences_manager_style_settings(prefmg, "c_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla,  SCE_C_DEFAULT, font);
  gtk_scintilla_style_set_fore (scintilla,  SCE_C_DEFAULT, fore);
  gtk_scintilla_style_set_back (scintilla,  SCE_C_DEFAULT, back);
  gtk_scintilla_style_set_size (scintilla,  SCE_C_DEFAULT, size);
  gtk_scintilla_style_set_italic (scintilla,  SCE_C_DEFAULT, italic);
  gtk_scintilla_style_set_bold (scintilla,  SCE_C_DEFAULT, bold);

  get_preferences_manager_style_settings(prefmg, "php_default", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_font (scintilla,  STYLE_BRACELIGHT, font);
  gtk_scintilla_style_set_fore (scintilla,  STYLE_BRACELIGHT, 16711680);// Matching bracket
  if(get_preferences_manager_higthlight_caret_line (prefmg))
    gtk_scintilla_style_set_back (scintilla,  STYLE_BRACELIGHT, get_preferences_manager_higthlight_caret_line_color(prefmg));
  else
    gtk_scintilla_style_set_back (scintilla,  STYLE_BRACELIGHT, back);
  gtk_scintilla_style_set_size (scintilla,  STYLE_BRACELIGHT, size);
  gtk_scintilla_style_set_italic (scintilla,  STYLE_BRACELIGHT, italic);
  gtk_scintilla_style_set_bold (scintilla,  STYLE_BRACELIGHT, TRUE);
  gtk_scintilla_style_set_font (scintilla,  STYLE_BRACEBAD, font);
  gtk_scintilla_style_set_fore (scintilla,  STYLE_BRACEBAD, 255);
  if(get_preferences_manager_higthlight_caret_line (prefmg))
    gtk_scintilla_style_set_back (scintilla,  STYLE_BRACEBAD, get_preferences_manager_higthlight_caret_line_color(prefmg));
  else
    gtk_scintilla_style_set_back (scintilla,  STYLE_BRACEBAD, back);
  gtk_scintilla_style_set_size (scintilla,  STYLE_BRACEBAD, size);
  gtk_scintilla_style_set_italic (scintilla,  STYLE_BRACEBAD, italic);
  gtk_scintilla_style_set_bold (scintilla,  STYLE_BRACEBAD, TRUE);

  gtk_scintilla_set_keywords(scintilla, 0, "ADD ALL ALTER ANALYZE AND AS ASC ASENSITIVE AUTO_INCREMENT BDB BEFORE BERKELEYDB BETWEEN BIGINT BINARY BLOB BOTH BTREE BY CALL CASCADE CASE CHANGE CHAR CHARACTER CHECK COLLATE COLUMN COLUMNS CONNECTION CONSTRAINT CREATE CROSS CURRENT_DATE CURRENT_TIME CURRENT_TIMESTAMP CURSOR DATABASE DATABASES DAY_HOUR DAY_MINUTE DAY_SECOND DEC DECIMAL DECLARE DEFAULT DELAYED DELETE DESC DESCRIBE DISTINCT DISTINCTROW DIV DOUBLE DROP ELSE ELSEIF ENCLOSED ERRORS ESCAPED EXISTS EXPLAIN FALSE FIELDS FLOAT FOR FORCE FOREIGN FROM FULLTEXT GRANT GROUP HASH HAVING HIGH_PRIORITY HOUR_MINUTE HOUR_SECOND IF IGNORE IN INDEX INFILE INNER INNODB INOUT INSENSITIVE INSERT INT INTEGER INTERVAL INTO IS ITERATE JOIN KEY KEYS KILL LEADING LEAVE LEFT LIKE LIMIT LINES LOAD LOCALTIME LOCALTIMESTAMP LOCK LONG LONGBLOB LONGTEXT LOOP LOW_PRIORITY MASTER_SERVER_ID MATCH MEDIUMBLOB MEDIUMINT MEDIUMTEXT MIDDLEINT MINUTE_SECOND MOD MRG_MYISAM NATURAL NOT NULL NUMERIC ON OPTIMIZE OPTION OPTIONALLY OR ORDER OUT OUTER OUTFILE PRECISION PRIMARY PRIVILEGES PROCEDURE PURGE READ REAL REFERENCES REGEXP RENAME REPEAT REPLACE REQUIRE RESTRICT RETURN RETURNS REVOKE RIGHT RLIKE RTREE SELECT SENSITIVE SEPARATOR SET SHOW SMALLINT SOME SONAME SPATIAL SPECIFIC SQL_BIG_RESULT SQL_CALC_FOUND_ROWS SQL_SMALL_RESULT SSL STARTING STRAIGHT_JOIN STRIPED TABLE TABLES TERMINATED THEN TINYBLOB TINYINT TINYTEXT TO TRAILING TRUE TYPES UNION UNIQUE UNLOCK UNSIGNED UNTIL UPDATE USAGE USE USER_RESOURCES USING VALUES VARBINARY VARCHAR VARCHARACTER VARYING WARNINGS WHEN WHERE WHILE WITH WRITE XOR YEAR_MONTH ZEROFILL");

  get_preferences_manager_style_settings(prefmg, "sql_word", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_back (scintilla, SCE_C_WORD, back);
  gtk_scintilla_style_set_font (scintilla, SCE_C_WORD, font);
  gtk_scintilla_style_set_fore (scintilla, SCE_C_WORD, fore);
  gtk_scintilla_style_set_size (scintilla, SCE_C_WORD, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_WORD, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_WORD, bold);

  get_preferences_manager_style_settings(prefmg, "sql_string", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_STRING, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_STRING, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_STRING, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_STRING, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_STRING, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_STRING, bold);

  get_preferences_manager_style_settings(prefmg, "sql_operator", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_OPERATOR, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_OPERATOR, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_OPERATOR, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_OPERATOR, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_OPERATOR, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_OPERATOR, bold);

  get_preferences_manager_style_settings(prefmg, "sql_comment", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENT, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENT, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENT, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENT, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENT, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENT, bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_COMMENTLINE, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_COMMENTLINE, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_COMMENTLINE, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_COMMENTLINE, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_COMMENTLINE, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_COMMENTLINE, bold);

  get_preferences_manager_style_settings(prefmg, "sql_number", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_NUMBER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_NUMBER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_NUMBER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_NUMBER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_NUMBER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_NUMBER, bold);

  get_preferences_manager_style_settings(prefmg, "sql_identifier", &font , &size, &fore, &back, &italic, &bold);

  gtk_scintilla_style_set_fore (scintilla, SCE_C_IDENTIFIER, fore);
  gtk_scintilla_style_set_font (scintilla, SCE_C_IDENTIFIER, font);
  gtk_scintilla_style_set_back (scintilla, SCE_C_IDENTIFIER, back);
  gtk_scintilla_style_set_size (scintilla, SCE_C_IDENTIFIER, size);
  gtk_scintilla_style_set_italic (scintilla, SCE_C_IDENTIFIER, italic);
  gtk_scintilla_style_set_bold (scintilla, SCE_C_IDENTIFIER, bold);

  gtk_scintilla_colourise(scintilla, 0, -1);
  g_object_unref(prefmg);
}
