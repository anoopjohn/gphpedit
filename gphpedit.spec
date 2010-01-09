Summary: A PHP source editor for GNOME 2.
Name: gphpedit
Version: 0.9.96
Release: 1
License: GPL
Group: X11/Utilities
Source: gphpedit-%{version}.tar.gz
URL: http://www.gphpedit.org/
BuildRequires: webkit-1.0
BuildRoot: /var/tmp/%{name}-root               

%description
gPHPEdit is a GNOME2 editor dedicated to editing PHP files and other 
supporting files like HTML/CSS/JS.

%prep
rm -rf $RPM_BUILD_ROOT
%setup
./configure

%build
make

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README

%{_bindir}/gphpedit
%{_datadir}/pixmaps/gphpedit.png
%{_datadir}/applications/gphpedit.desktop
%{_datadir}/gphpedit/php-gphpedit.api

%changelog
* Wed Nov 03 2004 Andy Jeffries <andy@gphpedit.org>
- Removed dependancy on GtkScintilla2

* Wed Jun 04 2003 Tim Jackson <tim@timj.co.uk>
- Updated to work with new build system

* Fri Jan 17 2003 Tim Jackson <tim@timj.co.uk>
- Initial RPM packaging
