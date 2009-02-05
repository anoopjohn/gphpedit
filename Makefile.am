SUBDIRS = \
	data	\
	doc	\
	pixmaps	\
	src \
	po

applicationsdir = $(datadir)/applications
applications_in_files = gphpedit.desktop.in
applications_DATA = $(applications_in_files:.desktop.in=.desktop)

spec_in_files = gphpedit.spec.in

EXTRA_DIST = \
	gphpedit.spec			\
	gphpedit.applications		\
	$(applications_in_files)	\
	$(spec_in_files)		\
	src/gtkscintilla2
