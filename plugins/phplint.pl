#!/usr/bin/perl

## 
##  This file is part of gPHPEdit, a GNOME2 PHP Editor.
##
##  Copyright (C) 2010 Jose Rostagno <joserostagno@vijona.com.ar>
##
##  This program is free software; you can redistribute it and/or modify
##  it under the terms of the GNU General Public License as published by
##  the Free Software Foundation; either version 2 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU General Public License for more details.
##
##  You should have received a copy of the GNU General Public License
##  along with this program; if not, write to the Free Software
##  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
##

  if ($ARGV[0] eq "-type") {
  print "SYNTAX";
  exit(0);
  } elsif ($ARGV[0] eq "-name") {
  print "PHPLINT";
  exit(0);
  } elsif ($ARGV[0] eq "-desc"){
  print "Check php syntax";
  exit(0);
  } elsif ($ARGV[0] eq "-version"){
  print "1.00";
  exit(0);
  } elsif ($ARGV[0] eq "-copyright"){
  print "Copyright \xc2\xa9 2010 José Rostagno";
  exit(0);
  } elsif ($ARGV[0] eq "-ftype"){
  print "PHP";
  exit(0);
  }

  $rec = join('', 'php -q -l -d html_errors=Off -f \'',$ARGV[0], '\'');

	use IPC::Open3;
	use Symbol qw(gensym);
	use IO::File;
	local *CATCHERR = IO::File->new_tmpfile;
	my $pid = open3(gensym, \*CATCHOUT, ">&CATCHERR", $rec);
	while( <CATCHOUT> ) {
    $view = $_;
  }
	waitpid($pid, 0);
	seek CATCHERR, 0, 0;
	while( <CATCHERR> ) {
   $_=~ s/PHP Parse error:  syntax error,/"E"/eg;
   $_=~ s/PHP Warning:  /"W"/eg;
   $_=~ s/(.*?)on line\s+(\d+)/"$2 $1\."/eg;
   $_=~ s/\s+\./"\."/eg;
   $_=~ s/$ARGV[0]//eg;
   $_=~ s/in .*?\.//eg;
   $view = join("", $_, $view);
  }
  print "SYNTAX\n".$view;
